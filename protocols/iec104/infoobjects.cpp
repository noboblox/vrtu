#include "protocols/iec104/infoobjects.hpp"

#include <algorithm>
#include <stdexcept>

#include "protocols/iec104/quality.hpp"
#include "errordecode.hpp"

namespace IEC104
{
    // Static initialization
    std::map<InfoObjectFactory::LookupKey, int>        InfoObjectFactory::msRegistered;
    std::vector<std::function<SharedInfoObject(void)>> InfoObjectFactory::msFunctions;
    std::vector<int>                                   InfoObjectFactory::msSizes;

    SharedInfoObject
    InfoObjectFactory::Create(uint8_t aType)
    {
        auto it_found = msRegistered.find(LookupKey(aType, LookupKey::ANY_PRIORITY));

        if (it_found == msRegistered.end())
            throw std::out_of_range("Creation function not found");

        auto& CreateFunction = msFunctions[it_found->second];
        return CreateFunction();
    }

    void
    InfoObjectFactory::RegisterInfoObject(int aType, int aPriority, int aInfoElementSize,
                                          const std::function<SharedInfoObject(void)>& arCreateFunction)
    {
        auto it_found = msRegistered.find(LookupKey(aType, LookupKey::ANY_PRIORITY));

        if (it_found == msRegistered.end() || 
            it_found->first.GetPriority() < aPriority)
        {
            msFunctions.push_back(arCreateFunction);
            msSizes.push_back(aInfoElementSize);
            msRegistered[LookupKey(aType, aPriority)] = msFunctions.size() - 1;
        }
    }

    int InfoObjectFactory::GetSize(uint8_t aType) noexcept
    {
        auto it_found = msRegistered.find(LookupKey(aType, LookupKey::ANY_PRIORITY));

        if (it_found == msRegistered.cend())
            return 0;
        else
            return msSizes[it_found->second];
    }

    bool InfoObjectFactory::HasType(uint8_t aType) noexcept
    {
        return msRegistered.find(LookupKey(aType, LookupKey::ANY_PRIORITY)) != msRegistered.cend();
    }

    BaseInfoObject::BaseInfoObject(const std::string& arName, int aTypeId, uint8_t aFlags)
        : DataStruct(arName),
          mType(*this, "type"),
          mAddress(*this, "ioa"),
          mFlags(aFlags)
    {
        mType = TypeEnum(static_cast<IEC104::Type>(aTypeId));
    }

    int BaseInfoObject::GetTypeId() const
    {
        return static_cast<int> ((*mType).GetValue());
    }

    IEC104::InfoAddress BaseInfoObject::GetAddress() const
    {
        return mAddress.IsValid() ? *mAddress : IEC104::InfoAddress();
    }

    void BaseInfoObject::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        const uint8_t* p_encoded = nullptr;
        RETHROW_FAIL_AS_DECODE_ERROR(p_encoded = arInput.ReadData(aAddressSize), mAddress);
        mAddress = InfoAddress(p_encoded, aAddressSize);
    }

    void BaseInfoObject::WriteTo(ByteStream& arOutput) const
    {
        (*mAddress).WriteTo(arOutput);
    }

    void BaseInfoObject::WriteJson(JSON::ValueHandle& arHandle) const
    {
        for (const auto& rp_child : mChilds)
        {
            if (!IsRoot() && (rp_child == &mType))
                continue; // Print type for standalone IOs only

            auto child_member = arHandle.AddMember(rp_child->GetName());
            rp_child->WriteJson(child_member);
        }
    }

    bool BaseInfoObject::HasQuality() const noexcept
    {
        return mFlags & FLAG_WITH_QUALITY;
    }

    const Quality& BaseInfoObject::GetQuality() const
    {
        if (!HasQuality())
            throw std::invalid_argument("InfoObject has no quality");
        return static_cast<const WithQuality&>(*this).GetQuality();
    }

    void BaseInfoObject::RequireNull(int aChecked) const
    {
        if (aChecked)
            throw std::runtime_error("Found data where nothing was expected");
    }

    void BaseInfoObject::RequireValid(const BaseData& arChecked) const
    {
        if (!arChecked.IsValid())
            throw std::runtime_error("Data is not valid");
    }

    const Quality& WithQuality::GetQuality() const
    {
        if (!mQuality.IsValid())
            throw std::invalid_argument("quality is not valid");
        return *mQuality;
    }

    WithQuality::WithQuality(const std::string& arName, int aTypeId)
        : BaseInfoObject(arName, aTypeId, FLAG_WITH_QUALITY),
          mQuality(*this, "quality")
    {
    }

    // Type 1: M_SP_NA_1 ////////////////////////////////////////////////////////////
    DataSinglePoint::DataSinglePoint()
        : WithQuality("singlePoint", TYPE_ID),
          mValue(*this, "value")
    {
    }

    void DataSinglePoint::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        BaseInfoObject::ReadFrom(arInput, aAddressSize);

        uint8_t encoded;
        RETHROW_FAIL_AS_DECODE_ERROR(encoded = arInput.ReadByte(), mValue);
        mValue = (         encoded & 0x01);
        RequireNull(       encoded & 0x0E);
        mQuality = Quality(encoded & 0xF0);
    }

    void DataSinglePoint::WriteTo(ByteStream& arOutput) const
    {
        BaseInfoObject::WriteTo(arOutput);

        uint8_t encoded = *mValue;
        encoded |= ((*mQuality).GetEncoded() & 0xF0);
        arOutput.WriteByte(encoded);
    }

    std::string DataSinglePoint::GetValueAsString() const
    {
        RequireValid(mValue);
       
        static constexpr const char* OUTPUT[] = {"false", "true"};
        return OUTPUT[(*mValue) & 0x01];
    }

    // Type 3: M_DP_NA_1 ////////////////////////////////////////////////////////////
    DataDoublePoint::DataDoublePoint()
      : WithQuality("doublePoint", TYPE_ID),
        mValue(*this, "value")
    {
    }

    void DataDoublePoint::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        BaseInfoObject::ReadFrom(arInput, aAddressSize);

        uint8_t encoded;
        RETHROW_FAIL_AS_DECODE_ERROR(encoded = arInput.ReadByte(), mValue);
        mValue = DoublePoint(encoded & 0x03);
        RequireNull(encoded & 0x0C);
        mQuality = Quality(encoded & 0xF0);
    }

    void DataDoublePoint::WriteTo(ByteStream& arOutput) const
    {
        BaseInfoObject::WriteTo(arOutput);

        const DoublePoint value = *mValue;
        uint8_t encoded = static_cast<uint8_t>(value);
        encoded |= ((*mQuality).GetEncoded() & 0xF0);
        arOutput.WriteByte(encoded);
    }

    std::string DataDoublePoint::GetValueAsString() const
    {
        RequireValid(mValue);
        return (*mValue).GetLabel();
    }

    // Type 11: M_ME_NB_1 ////////////////////////////////////////////////////////////
    DataMeasuredScaled::DataMeasuredScaled()
        : WithQuality("measuredScaled", TYPE_ID),
        mValue(*this, "value")
    {
    }

    void DataMeasuredScaled::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        BaseInfoObject::ReadFrom(arInput, aAddressSize);

        {
            // Data to read: 16 bit signed int as little-endian
            // The read is performed as uint16, to make sure that the compiler doesn't mess with the sign bit during the bit-shift
            uint16_t encoded;
            RETHROW_FAIL_AS_DECODE_ERROR(encoded = arInput.ReadByte(), mValue);
            RETHROW_FAIL_AS_DECODE_ERROR(encoded |= (arInput.ReadByte() << 8), mValue);

            const int16_t result = static_cast<int16_t>(encoded);
            mValue = result;
        }

        uint8_t encoded;
        RETHROW_FAIL_AS_DECODE_ERROR(encoded = arInput.ReadByte(), mQuality);
        RequireNull(encoded & 0x0E);
        mQuality = Quality(encoded);
    }

    void DataMeasuredScaled::WriteTo(ByteStream& arOutput) const
    {
        BaseInfoObject::WriteTo(arOutput);

        const int16_t value = static_cast<int16_t>(*mValue);
        const uint16_t bytes = *reinterpret_cast<const uint16_t*>(&value);

        arOutput.WriteByte(bytes & 0xFF); // Start with LSB
        arOutput.WriteByte((bytes >> 8) & 0xFF);

        arOutput.WriteByte((*mQuality).GetEncoded());
    }

    std::string DataMeasuredScaled::GetValueAsString() const
    {
        RequireValid(mValue);
        return std::to_string(*mValue);
    }

    // Type 13: M_ME_NC_1 ////////////////////////////////////////////////////////////
    DataMeasuredFloat::DataMeasuredFloat()
        : WithQuality("measuredFloat", TYPE_ID),
        mValue(*this, "value")
    {
    }

    void DataMeasuredFloat::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        BaseInfoObject::ReadFrom(arInput, aAddressSize);

        // Assumption: Host endianess is the same for "float" and "int"
        {
            // For IEC104 the float is always stored as little-endian (first byte is LSB):
            RETHROW_FAIL_AS_DECODE_ERROR
            (
                uint32_t encoded;
                encoded = arInput.ReadByte();
                encoded |= (arInput.ReadByte() << 8);
                encoded |= (arInput.ReadByte() << 16);
                encoded |= (arInput.ReadByte() << 24);

                mValue = *reinterpret_cast<float*>(&encoded),
            mValue);

        }

        RETHROW_FAIL_AS_DECODE_ERROR
        (
            const uint8_t encoded = arInput.ReadByte();
            RequireNull(encoded & 0x0E);
            mQuality = Quality(encoded),
        mQuality);
    }

    void DataMeasuredFloat::WriteTo(ByteStream& arOutput) const
    {
        BaseInfoObject::WriteTo(arOutput);

        // Interpret 32bit float value as integer for encoding operations
        const uint32_t bytes = *reinterpret_cast<const uint32_t*>(&(*mValue));
        arOutput.WriteByte(bytes & 0xFF); // Start with LSB
        arOutput.WriteByte((bytes >> 8)  & 0xFF);
        arOutput.WriteByte((bytes >> 16) & 0xFF);
        arOutput.WriteByte((bytes >> 24) & 0xFF);

        arOutput.WriteByte((*mQuality).GetEncoded());
    }

    std::string DataMeasuredFloat::GetValueAsString() const
    {
        RequireValid(mValue);
        return std::to_string(*mValue);
    }
}
