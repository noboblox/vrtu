#include "protocols/iec104/asdu.hpp"

#include "errordecode.hpp"
#include "errorunknowntype.hpp"

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
        RequireRange(1, 2, aValue);
        mCASize = aValue;
    }

    void AsduConfig::SetIOASize(int aValue)
    {
        RequireRange(1, 3, aValue);
        mIOASize = aValue;
    }

    void AsduConfig::SetReasonSize(int aValue)
    {
        RequireRange(1, 2, aValue);
        mReasonSize = aValue;
    }

    void AsduConfig::RequireRange(int aMin, int aMax, int aChecked) const
    {
        if (aChecked < aMin || aChecked > aMax)
            throw std::invalid_argument("Value out of range");
    }

    Asdu::Asdu(const AsduConfig& arConfig)
      : DataStruct("asdu"),
        mConfig(arConfig),
        mType(*this, "type"),
        mSize(*this, "size"),
        mReason(*this, "reason"),
        mIsSequence(*this, "sequence"),
        mOrigin(*this, "origin"),
        mCommonAddress(*this, "address"),
        mObjects(*this, "objects") {}

    void Asdu::ReadFrom(ByteStream& arBuffer)
    {
        try
        {
            ReadHeader(arBuffer);
            mObjects.Clear();

            while (arBuffer.RemainingBytes() != 0)
            {
                if (mObjects.GetSize() == (*mSize))
                    throw std::runtime_error("More data available than expected");

                auto p_data = InfoObjectFactory::Create(static_cast<int>((*mType).GetValue()));
                mObjects.Append(p_data);
                p_data->ReadFrom(arBuffer, mConfig.GetIOASize());
            }
        }
        catch (std::exception& e)
        {
            // TODO report something
            Clear();
            throw;
        }
    }
    
    void Asdu::WriteTo(ByteStream& arBuffer) const
    {
        WriteHeader(arBuffer);
        for (size_t i = 0; i < mObjects.GetSize(); ++i)
            mObjects.Get(i).WriteTo(arBuffer);
    }

    void Asdu::ReadHeader(ByteStream& arBuffer)
    {
        RETHROW_FAIL_AS_DECODE_ERROR(mType = static_cast<IEC104::Type> (arBuffer.ReadByte()), mType);

        if (!InfoObjectFactory::HasType(static_cast<int>((*mType).GetValue())))
        {
            arBuffer.Rollback(1);
            throw ErrorUnknownType(arBuffer, mType);
        }

        {
            uint8_t encoded;
            RETHROW_FAIL_AS_DECODE_ERROR(encoded = arBuffer.ReadByte(), mSize);
            mSize = (encoded & 0x7F);
            mIsSequence = (encoded & 0x80);

            if (GetExpectedSize() != (arBuffer.RemainingBytes() + 2))
                //throw ErrorSizeUnexpected(mSize, mType);
                throw std::invalid_argument("");
        }

        {
            uint8_t encoded;
            RETHROW_FAIL_AS_DECODE_ERROR(encoded = arBuffer.ReadByte(), mReason);
            mReason = Reason(static_cast<ReasonCode>(encoded & 0x3F), encoded & 0x40, encoded & 0x80);
        }

        if (mConfig.GetReasonSize() == 2)
        {
            RETHROW_FAIL_AS_DECODE_ERROR(mOrigin = arBuffer.ReadByte(), mOrigin);
        }
        else
        {
            mOrigin.Clear();
        }

        int ca;
        RETHROW_FAIL_AS_DECODE_ERROR(ca = arBuffer.ReadByte(), mCommonAddress);

        if (mConfig.GetCASize() == 2)
        {
            RETHROW_FAIL_AS_DECODE_ERROR(ca += (arBuffer.ReadByte() << 8), mCommonAddress);
        }

        mCommonAddress = ca;
    }

    void Asdu::WriteHeader(ByteStream& arBuffer) const
    {
        /*
         * Buffer to store encoded data, 
         * in order to avoid all the endianess problems with actual integers
         */
        uint8_t encoded;
        
        encoded = static_cast<uint8_t> ((*mType).GetValue());
        arBuffer.WriteByte(encoded);

        {
            encoded = (*mSize & 0x7F);

            if (*mIsSequence)
                encoded |= 0x80;
            arBuffer.WriteByte(encoded);
        }

        {
            encoded = static_cast<uint8_t> ((*mReason).GetReasonCode().GetValue());
            encoded |= ((*mReason).IsNegative() << 6);
            encoded |= ((*mReason).IsTest()     << 7);
            arBuffer.WriteByte(encoded);
        }

        if (mConfig.GetReasonSize() == 2)
        {
            encoded = (*mOrigin & 0xFF);
            arBuffer.WriteByte(encoded);
        }

        encoded = (*mCommonAddress & 0xFF);
        arBuffer.WriteByte(encoded);

        if (mConfig.GetCASize() == 2)
        {
            encoded = ((*mCommonAddress >> 8) & 0xFF);
            arBuffer.WriteByte(encoded);
        }
    }

    int Asdu::GetExpectedSize() const
    {
        int result = 2 + mConfig.GetReasonSize() + mConfig.GetCASize();
        const int type = mType.IsValid() ? static_cast<int> ((*mType).GetValue()) : 0;

        if (mSize.IsValid() && type && 
            InfoObjectFactory::HasType(type))
        {
            result += InfoObjectFactory::GetSize(type) * (*mSize);

            if (mIsSequence.IsValid() && *mIsSequence)
                result += mConfig.GetIOASize();
            else
                result += mConfig.GetIOASize() * (*mSize);
        }

        return result;
    }
}
