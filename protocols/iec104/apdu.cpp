#include "protocols/iec104/apdu.hpp"

#include <cstring>
#include <string>

#include "core/bytestream.hpp"
#include "protocols/iec104/asdu.hpp"

namespace IEC104
{
    Apdu::Apdu() noexcept
        : mType(INVALID), mData()
    {
    }

    Apdu::Apdu(const uint8_t* apData, size_t aSize) noexcept
        : mType(INVALID)
    {
        Assign(apData, aSize);
    }

    void Apdu::Assign(const uint8_t* apData, size_t aSize)
    {
        if (apData && aSize >= GetNeededSize())
        {
            std::memcpy(mData, apData, GetNeededSize());
            mType = VerifyMessage();
        }
        else
        {
            std::memset(mData, 0, GetNeededSize());
        }
    }

    bool Apdu::IsValid() const noexcept 
    {
        return mType != INVALID; 
    }

    bool Apdu::HasAsdu() const noexcept 
    {
        return mType == DATA; 
    }

    size_t Apdu::GetAsduSize() const noexcept 
    {
        return HasAsdu() ? (mData[1] - 4) : 0; 
    }

    bool Apdu::HasReceiveCounter() const noexcept 
    {
        return (mType == DATA) || (mType == RECEIVE_CONFIRM); 
    }
    
    bool Apdu::HasSendCounter() const noexcept 
    {
        return mType == DATA; 
    }

    int Apdu::GetReceiveCounter() const noexcept
    {
        return ReadSequenceInternal(&mData[4]);
    }

    void Apdu::SetReceiveCounter(int aValue) noexcept
    {
        if (HasReceiveCounter() && aValue <= MAX_SEQUENCE)
        {
            WriteSequenceInternal(&mData[4], aValue);
        }
    }

    int Apdu::GetSendCounter() const noexcept
    {
        return ReadSequenceInternal(&mData[2]);
    }

    void Apdu::SetSendCounter(int aValue) noexcept
    {
        if (HasSendCounter() && aValue <= MAX_SEQUENCE)
        {
            WriteSequenceInternal(&mData[2], aValue);
        }
    }

    const void* Apdu::GetData() const noexcept 
    {
        return mData; 
    }

    bool Apdu::ReadAsdu(ByteStream& arInput) noexcept
    {
        if ((mType == DATA) && (arInput.RemainingBytes() >= GetAsduSize()))
        {
            try
            {
                mpAsdu.reset(new Asdu(AsduConfig::Defaults));
                mpAsdu->ReadFrom(arInput);
                return true;
            }
            catch (...) {}
        }

        return false;
    }

    const Asdu* Apdu::GetAsdu() const noexcept 
    {
        return mpAsdu.get(); 
    }


    bool Apdu::NeedsConfirmation() const noexcept
    {
        return (mType == STARTDT_REQUEST) ||
               (mType == STOPDT_REQUEST) ||
               (mType == TESTFR_REQUEST);
    }

    void Apdu::ConvertToConfirmation() noexcept
    {
        switch (mType)
        {
        case STARTDT_REQUEST:
            mType = STARTDT_CONFIRM;
            break;
        case STOPDT_REQUEST:
            mType = STOPDT_CONFIRM;
            break;
        case TESTFR_REQUEST:
            mType = TESTFR_CONFIRM;
            break;
        default:
            break;
        }
        mData[2] = mType;
    }

    std::string Apdu::TypeToString() const
    {
        switch (mType)
        {
        case DATA:
            return "ASDU";
        case RECEIVE_CONFIRM:
            return "RECEIVE ack";
        case STARTDT_REQUEST:
            return "STARTDT req";
        case STARTDT_CONFIRM:
            return "STARTDT con";
        case STOPDT_REQUEST:
            return "STOPDT req";
        case STOPDT_CONFIRM:
            return "STOPDT con";
        case TESTFR_REQUEST:
            return "TESTFR req";
        case TESTFR_CONFIRM:
            return "TESTFR con";

        default:
            return "INVALID";
        }
    }

    void Apdu::SetType(MessageType aType) noexcept
    {
        mType = aType;

        mData[0] = 0x68;
        mData[1] = 0x04;
        std::memset(&mData[2], 0, 4);

        switch (aType)
        {
            break;

        case RECEIVE_CONFIRM:
            mData[2] = 0x01;
            break;

        case STARTDT_REQUEST:
        case STARTDT_CONFIRM:
        case STOPDT_REQUEST:
        case STOPDT_CONFIRM:
        case TESTFR_REQUEST:
        case TESTFR_CONFIRM:
            mData[2] = static_cast<uint8_t>(aType);
            break;

        default:
            break;
        };
    }

    int Apdu::ReadSequenceInternal(const uint8_t* apBegin) const noexcept
    {
        return (apBegin[0] + (apBegin[1] << 8)) >> 1;
    }

    void Apdu::WriteSequenceInternal(uint8_t* apBegin, int aValue) noexcept
    {
        apBegin[0] = ((aValue & 0x7F) << 1);
        apBegin[1] = ((aValue >> 7) & 0xFF);
    }

    Apdu::MessageType Apdu::VerifyMessage() noexcept
    {
        // APCI 
        // [0] Always 0x68
        // [1] Message size excluding first 2 bytes (>= 4)
        // [2] Protocol control field 1
        // [3] Protocol control field 2
        // [4] Protocol control field 3
        // [5] Protocol control field 4
        // From here: Message end or ASDU begin

        if ((mData[0] != 0x68) || (mData[1] < 4) || ((mData[4] & 0x01) != 0))
            return INVALID;

        // Type I
        if ((mData[2] & 0x01) == 0)
            return DATA;

        // Type S
        else if (mData[2] == RECEIVE_CONFIRM && mData[3] == 0)
            return RECEIVE_CONFIRM;

        // Now only type "U" is left, which means that only control field 1 is allowed to be != 0
        else if ((mData[3] != 0) || (mData[4] != 0) || (mData[5]) != 0)
            return INVALID;

        switch (mData[2])
        {
        case STARTDT_REQUEST:
        case STARTDT_CONFIRM:
        case STOPDT_REQUEST:
        case STOPDT_CONFIRM:
        case TESTFR_REQUEST:
        case TESTFR_CONFIRM:
            return static_cast<MessageType>(mData[2]);

        default:
            return INVALID;
        }
    }

    ReceiveConfirmation::ReceiveConfirmation(int aReceiveCounter)
        : Apdu()
    {
        if (aReceiveCounter < MAX_SEQUENCE)
        {
            SetType(RECEIVE_CONFIRM);
            SetReceiveCounter(aReceiveCounter);
        }
    }
}