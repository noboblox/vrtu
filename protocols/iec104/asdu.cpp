#include "protocols/iec104/asdu.hpp"

#include "core/util.hpp"
#include "core/bytestream.hpp"

namespace IEC104
{
    AsduConfig AsduConfig::Defaults(2, 2, 3);

    AsduConfig::AsduConfig(int aReasonSize, int aCASize, int aIOASize)
        // No initializer list
    {
        // Set with range checks
        SetCASize(aCASize);
        SetIOASize(aIOASize);
        SetReasonSize(aReasonSize);
    }

    void AsduConfig::SetCASize(int aValue)
    {
        UTIL::AssertRange(1, 2, aValue);
        mCASize = aValue;
    }

    void AsduConfig::SetIOASize(int aValue)
    {
        UTIL::AssertRange(1, 3, aValue);
        mIOASize = aValue;
    }

    void AsduConfig::SetReasonSize(int aValue)
    {
        UTIL::AssertRange(1, 2, aValue);
        mReasonSize = aValue;
    }

    Asdu::Asdu(const AsduConfig& arConfig)
        : mConfig(arConfig)
    {
    }

    int Asdu::GetType() const
    {
        return mType;
    }

    int Asdu::GetNumberOfInfoObjects() const noexcept
    {
        return mObjects.size();
    }

    void Asdu::ReadFrom(ByteStream& arBuffer)
    {
        ReadHeader(arBuffer);
        mObjects.clear();

        /*
            * the standard defines 2 cases:
            * 1. ASDU sequence-optimization is DISABLED -> each info object has its own address
            * 2. ASDU sequence-optimization is ENABLED  -> only the 1st info object has an address
            *    subsequent objects have an implicit address, which is incremented
            */
        int ioa_size = mConfig.GetIOASize(); // 1st address is always present

        while (arBuffer.RemainingBytes() != 0)
        {
            if (mObjects.size() == mSize)
                throw std::runtime_error("More data available than expected");

            auto p_data = InfoObjectFactory::Create(mType);
            mObjects.push_back(p_data);
            p_data->ReadFrom(arBuffer, ioa_size);

            if (mIsSequence)
                ioa_size = 0;
        }
    }
    
    void Asdu::WriteTo(ByteStream& arBuffer) const 
    {
        WriteHeader(arBuffer);

        for (const auto& obj : mObjects) {
            obj->WriteTo(arBuffer);
        }
    }

    void Asdu::ReadHeader(ByteStream& arBuffer)
    {
        mType = static_cast<IEC104::Type> (arBuffer.ReadByte());

        if (!InfoObjectFactory::HasType(mType))
            throw std::runtime_error("data type is not registered");

        uint8_t size_seq = arBuffer.ReadByte();
        mSize = (size_seq & 0x7F);
        mIsSequence = (size_seq & 0x80);

        if (GetExpectedSize() != (arBuffer.RemainingBytes() + 2))
            throw std::runtime_error("data size does not match the expected asdu size");

        mReason = static_cast<ReasonCode>(arBuffer.ReadByte() & 0x3F);

        if (mConfig.GetReasonSize() == 2)
            mOrigin = arBuffer.ReadByte();

        mCommonAddress = arBuffer.ReadByte();

        if (mConfig.GetCASize() == 2)
            mCommonAddress += (arBuffer.ReadByte() << 8);
    }

    void Asdu::WriteHeader(ByteStream& arBuffer) const
    {
        arBuffer.WriteByte(mType);

        uint8_t size_seq = (mSize & 0x7F);

        if (mIsSequence)
            size_seq |= 0x80;

        arBuffer.WriteByte(size_seq);

        uint8_t cause_neg_test = static_cast<uint8_t> (mReason.GetValue());
        // TODO negative, test...
        arBuffer.WriteByte(cause_neg_test);

        if (mConfig.GetReasonSize() == 2)
            arBuffer.WriteByte((mOrigin & 0xFF));

        arBuffer.WriteByte(mCommonAddress & 0xFF);

        if (mConfig.GetCASize() == 2)
            arBuffer.WriteByte(((mCommonAddress >> 8) & 0xFF));
    }

    unsigned Asdu::GetExpectedSize() const
    {
        int result = 2 + mConfig.GetReasonSize() + mConfig.GetCASize();

        if (InfoObjectFactory::HasType(mType))
        {
            result += InfoObjectFactory::GetSize(mType) * mSize;

            if (mIsSequence)
                result += mConfig.GetIOASize();
            else
                result += mConfig.GetIOASize() * mSize;
        }

        return result;
    }
}
