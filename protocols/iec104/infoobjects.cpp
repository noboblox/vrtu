#include "protocols/iec104/infoobjects.hpp"

#include <algorithm>
#include <stdexcept>

#include "core/bytestream.hpp"
#include "protocols/iec104/quality.hpp"

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

    BaseInfoObject::BaseInfoObject(int aTypeId)
        : mType(aTypeId)
    {
    }

    int BaseInfoObject::GetTypeId() const
    {
        return mType;
    }

    InfoAddress BaseInfoObject::GetAddress() const
    {
        return mAddress;
    }

    void BaseInfoObject::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        if (aAddressSize == 0)
        {
            mAddress = InfoAddress();
        }
        else
        {
            const uint8_t* p_encoded = nullptr;
            p_encoded = arInput.ReadData(aAddressSize);
            mAddress = InfoAddress(p_encoded, aAddressSize);
        }
    }

    void BaseInfoObject::WriteTo(ByteStream& arOutput) const
    {
        mAddress.WriteTo(arOutput);
    }

    // Type 1: M_SP_NA_1 ////////////////////////////////////////////////////////////
    void DataSinglePoint::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        BaseInfoObject::ReadFrom(arInput, aAddressSize);

        uint8_t encoded = arInput.ReadByte();
        val = (encoded & 0x01);
        q = Quality(encoded & 0xF0);
    }

    void DataSinglePoint::WriteTo(ByteStream& arOutput) const
    {
        BaseInfoObject::WriteTo(arOutput);

        uint8_t encoded = val;
        encoded |= (q.GetEncoded() & 0xF0);
        arOutput.WriteByte(encoded);
    }

    // Type 3: M_DP_NA_1 ////////////////////////////////////////////////////////////
    void DataDoublePoint::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        BaseInfoObject::ReadFrom(arInput, aAddressSize);

        uint8_t encoded = arInput.ReadByte();
        val = DoublePoint(encoded & 0x03);
        q = Quality(encoded & 0xF0);
    }

    void DataDoublePoint::WriteTo(ByteStream& arOutput) const
    {
        BaseInfoObject::WriteTo(arOutput);

        uint8_t encoded = static_cast<uint8_t>(val.GetValue());
        encoded |= (q.GetEncoded() & 0xF0);
        arOutput.WriteByte(encoded);
    }

    // Type 11: M_ME_NB_1 ////////////////////////////////////////////////////////////
    void DataMeasuredScaled::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        BaseInfoObject::ReadFrom(arInput, aAddressSize);

        {
            // TODO warning implementation defined if > INT_MAX
            uint16_t encoded;
            encoded = arInput.ReadByte();
            encoded |= (arInput.ReadByte() << 8);

            const int16_t result = static_cast<int16_t>(encoded);
            val = result;
        }
        // TODO ValueQuality
    }

    void DataMeasuredScaled::WriteTo(ByteStream& arOutput) const
    {
        BaseInfoObject::WriteTo(arOutput);

        const int16_t value = static_cast<int16_t>(val);
        const uint16_t bytes = *reinterpret_cast<const uint16_t*>(&value);

        arOutput.WriteByte(bytes & 0xFF); // Start with LSB
        arOutput.WriteByte((bytes >> 8) & 0xFF);

        // TODO ValueQuality
    }

    // Type 13: M_ME_NC_1 ////////////////////////////////////////////////////////////
    void DataMeasuredFloat::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        BaseInfoObject::ReadFrom(arInput, aAddressSize);

        uint32_t encoded;
        encoded = arInput.ReadByte();
        encoded |= (arInput.ReadByte() << 8);
        encoded |= (arInput.ReadByte() << 16);
        encoded |= (arInput.ReadByte() << 24);

        val = *reinterpret_cast<float*>(&encoded);


        // TODO ValueQuality
    }

    void DataMeasuredFloat::WriteTo(ByteStream& arOutput) const
    {
        BaseInfoObject::WriteTo(arOutput);

        // Interpret 32bit float value as integer for encoding operations
        const uint32_t bytes = *reinterpret_cast<const uint32_t*>(&val);
        arOutput.WriteByte(bytes & 0xFF); // Start with LSB
        arOutput.WriteByte((bytes >> 8)  & 0xFF);
        arOutput.WriteByte((bytes >> 16) & 0xFF);
        arOutput.WriteByte((bytes >> 24) & 0xFF);

        // TODO ValueQuality
    }

    // Type 100: C_IC_NA_1 ////////////////////////////////////////////////////////////
    void DataInterrogationCommand::ReadFrom(ByteStream& arInput, int aAddressSize)
    {
        BaseInfoObject::ReadFrom(arInput, aAddressSize);
        val = InterrogationQualifier(arInput.ReadByte());
    }

    void DataInterrogationCommand::WriteTo(ByteStream& arOutput) const
    {
        BaseInfoObject::WriteTo(arOutput);
        arOutput.WriteByte(static_cast<uint8_t>(val.GetValue()));
    }
}
