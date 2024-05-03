#include "protocols/iec104/apdu.hpp"

#include <cstring>
#include <iostream>
#include <string>

#include "core/bytestream.hpp"
#include "protocols/iec104/asdu.hpp"

namespace IEC104
{
    Apdu::Apdu() noexcept
        : mType(INVALID), mHeader()
    {
    }

    Apdu::Apdu(const uint8_t* apData, size_t aSize) noexcept
        : mType(INVALID)
    {
        ByteStream source;
        source.WriteData(apData, aSize);
        ReadFrom(source);
    }

    Apdu::Apdu(const Apdu& arOther)
        : mType(arOther.mType),
          mHeader(arOther.mHeader),
          mpAsdu(nullptr)
    {
        if (mType == DATA) // TODO FIXME
            throw std::runtime_error("Copy construction of ASDUs not yet implemented");
    }

    Apdu::Apdu(Apdu&& arOther) noexcept
        : mType(arOther.mType),
          mHeader(arOther.mHeader),
          mpAsdu(std::move(arOther.mpAsdu))
    {
    }

    Apdu::~Apdu()
    {
    }

    // Count hops from left to right.
    // If aLeft < aRight -> positive
    // If aRight < aLeft -> negative
    int Apdu::SequenceDistance(int aLeft, int aRight) noexcept
    {
        const int distance_normal = aRight - aLeft;

        int distance_overflow;

        if (aLeft < aRight)
            distance_overflow = aRight - (aLeft + MAX_SEQUENCE);
        else
            distance_overflow = (aRight + MAX_SEQUENCE) - aLeft;

        if (std::abs(distance_overflow) < std::abs(distance_normal))
            return distance_overflow;
        else
            return distance_normal;
    }


    void Apdu::ReadFrom(ByteStream& arSource)
    {
        if (arSource.RemainingBytes() >= GetHeaderSize())
        {
            std::memcpy(&mHeader, arSource.ReadData(GetHeaderSize()), GetHeaderSize());
            mType = VerifyMessage();

            if (mType == DATA)
            {
                ReadAsdu(arSource) ? mType = DATA : mType = INVALID;
            }
        }
        else
        {
            mHeader = {};
        }
    }

    bool Apdu::IsValid() const noexcept 
    {
        return mType != INVALID; 
    }

    bool Apdu::IsAsdu() const noexcept 
    {
        return mType == DATA; 
    }

    size_t Apdu::GetAsduSize() const noexcept 
    {
        return IsAsdu() ? (mHeader.mSize - 4) : 0; 
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
        return ReadSequenceInternal(mHeader.mControl + 2);
    }

    void Apdu::SetReceiveCounter(int aValue) noexcept
    {
        if (HasReceiveCounter() && aValue <= MAX_SEQUENCE)
        {
            WriteSequenceInternal(mHeader.mControl + 2, aValue);
        }
    }

    int Apdu::GetSendCounter() const noexcept
    {
        return ReadSequenceInternal(mHeader.mControl);
    }

    void Apdu::SetSendCounter(int aValue) noexcept
    {
        if (HasSendCounter() && aValue <= MAX_SEQUENCE)
        {
            WriteSequenceInternal(mHeader.mControl, aValue);
        }
    }

    const void* Apdu::GetData() const noexcept 
    {
        return &mHeader; 
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
            catch (std::exception& e)  { std::cout << e.what() << std::endl; }
            catch (...) {}
        }

        return false;
    }

    const Asdu& Apdu::GetAsdu() const
    {
        if (!mpAsdu)
            throw std::invalid_argument("ASDU is not available");

        return *mpAsdu.get(); 
    }

    bool Apdu::IsService() const noexcept
    {
        return IsServiceRequest() || IsServiceConfirmation();
    }

    bool Apdu::IsServiceRequest() const noexcept
    {
        return (mType == STARTDT_REQUEST) ||
               (mType == STOPDT_REQUEST)  ||
               (mType == TESTFR_REQUEST);
    }

    bool Apdu::IsServiceConfirmation() const noexcept
    {
        return (mType == STARTDT_CONFIRM) ||
               (mType == STOPDT_CONFIRM)  ||
               (mType == TESTFR_CONFIRM);
    }

    Apdu Apdu::CreateServiceConfirmation() const noexcept
    {
        Apdu result;

        switch (mType)
        {
        case STARTDT_REQUEST:
            result.InitHeader(STARTDT_CONFIRM);
            break;
        case STOPDT_REQUEST:
            result.InitHeader(STOPDT_CONFIRM);
            break;
        case TESTFR_REQUEST:
            result.InitHeader(TESTFR_CONFIRM);
            break;

        default:
            break;
        }

        return result;
    }

    std::string Apdu::GetTypeString() const
    {
        switch (mType)
        {
        case DATA:
            return "I";
        case RECEIVE_CONFIRM:
            return "S";
        case STARTDT_REQUEST:
        case STARTDT_CONFIRM:
        case STOPDT_REQUEST:
        case STOPDT_CONFIRM:
        case TESTFR_REQUEST:
        case TESTFR_CONFIRM:
            return "U";

        default:
            return "";
        }
    }

    std::string Apdu::GetServiceString() const
    {
        switch (mType)
        {
        case STARTDT_REQUEST:
            return "STARTDT act";
        case STARTDT_CONFIRM:
            return "STARTDT con";
        case STOPDT_REQUEST:
            return "STOPDT act";
        case STOPDT_CONFIRM:
            return "STOPDT con";
        case TESTFR_REQUEST:
            return "TESTFR act";
        case TESTFR_CONFIRM:
            return "TESTFR con";

        default:
            return "";
        }
    }

    void Apdu::InitHeader(MessageType aType) noexcept
    {
        mType = aType;

        mHeader.mStartByte = 0x68;
        mHeader.mSize = 0x04;
        std::memset(mHeader.mControl, 0, 4);

        mHeader.mControl[0] = static_cast<uint8_t>(aType);
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

    Apdu::MessageType Apdu::VerifyMessage() const noexcept
    {
        // APCI 
        // [0] Always 0x68
        // [1] Message size excluding first 2 bytes (>= 4)
        // [2] Protocol control field 1
        // [3] Protocol control field 2
        // [4] Protocol control field 3
        // [5] Protocol control field 4
        // From here: Message end or ASDU begin

        if ((mHeader.mStartByte != 0x68) || (mHeader.mSize < 4) || ((mHeader.mControl[2] & 0x01) != 0))
            return INVALID;

        // Type I
        if ((mHeader.mControl[0] & 0x01) == 0)
            return DATA;

        // Type S
        else if (mHeader.mControl[0] == RECEIVE_CONFIRM && mHeader.mControl[1] == 0)
            return RECEIVE_CONFIRM;

        // Now only type "U" is left, which means that only control field 1 is allowed to be != 0
        else if ((mHeader.mControl[1] != 0) || (mHeader.mControl[2] != 0) || (mHeader.mControl[3]) != 0)
            return INVALID;

        switch (mHeader.mControl[0])
        {
        case STARTDT_REQUEST:
        case STARTDT_CONFIRM:
        case STOPDT_REQUEST:
        case STOPDT_CONFIRM:
        case TESTFR_REQUEST:
        case TESTFR_CONFIRM:
            return static_cast<MessageType>(mHeader.mControl[0]);

        default:
            return INVALID;
        }
    }

    AsduAcknowledgeApdu::AsduAcknowledgeApdu(int aReceiveCounter)
        : Apdu()
    {
        if (aReceiveCounter <= MAX_SEQUENCE)
        {
            InitHeader(RECEIVE_CONFIRM);
            SetReceiveCounter(aReceiveCounter);
        }
    }
}