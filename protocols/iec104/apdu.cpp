#include "protocols/iec104/apdu.hpp"

#include <cstring>
#include "core/bytestream.hpp"
#include "protocols/iec104/sequence.hpp"

namespace IEC104
{
    const Apdu Apdu::STARTDT_ACT = Apdu(Apdu::STARTDT_ACT_BYTE);
    const Apdu Apdu::STOPDT_ACT  = Apdu(Apdu::STOPDT_ACT_BYTE);
    const Apdu Apdu::TESTFR_ACT  = Apdu(Apdu::TESTFR_ACT_BYTE);
    const Apdu Apdu::STARTDT_CON = Apdu(Apdu::STARTDT_CON_BYTE);
    const Apdu Apdu::STOPDT_CON  = Apdu(Apdu::STOPDT_CON_BYTE);
    const Apdu Apdu::TESTFR_CON  = Apdu(Apdu::TESTFR_CON_BYTE);

    Apdu::Apdu(uint8_t service) noexcept
    {
        mHeader[2] = service;
    }

    Apdu::Apdu(ByteStream& buf)
    {
        if (!IsFullyAvailable(buf))
            throw std::invalid_argument("data is not ready to construct an apdu");

        buf.ReadInto(mHeader, HEADER_SIZE);

        if (PayloadLength() > 0)
            buf.ReadInto(mPayload, PayloadLength());

        if (!IsValid())
            throw std::invalid_argument("apdu structure is unplausible");
    }

    Apdu::Apdu(Sequence recv)
        : mHeader{ 0x68, 0x04, 0x01, recv.EncodedLowByte(), recv.EncodedHighByte() }
    {
    }

    Apdu::~Apdu()
    {
    }

    size_t Apdu::PayloadLength() const noexcept
    {
        if (Length() > HEADER_SIZE)
            return Length() - HEADER_SIZE;
        else
            return 0;
    }

    size_t Apdu::Length() const noexcept
    {
        return static_cast<size_t>(mHeader[1]) + 2;
    }

    bool Apdu::IsFullyAvailable(const ByteStream& buf)
    {
        if (buf.RemainingBytes() < Apdu::HEADER_SIZE)
            return false;

        auto reported_len = static_cast<size_t>(buf.PeekAt(1)) + 2;

        if (reported_len > 255 || reported_len < 6)
            throw std::runtime_error("reported apdu length is not in valid range");

        return buf.RemainingBytes() >= reported_len;
    }

    bool Apdu::IsValid() const noexcept 
    {
        if (Length() > 255)
            return false;

        if (HasPayload())
            return Length() > HEADER_SIZE;
        else
            return Length() == HEADER_SIZE;
    }

    bool Apdu::HasPayload() const noexcept
    {
        return (mHeader[2] & 0x01) == 0;
    }
    bool Apdu::IsRecvAck() const noexcept
    {
        return mHeader[2] == 0x01;
    }

    void Apdu::WriteTo(std::vector<uint8_t>& dest) const
    {
        dest.insert(dest.end(), mHeader, mHeader + HEADER_SIZE);

        if (HasPayload())
            dest.insert(dest.end(), mPayload, mPayload + PayloadLength());
    }

    ServiceType Apdu::ServiceActivation() const noexcept
    {
        int service = mHeader[2];

        switch (service) {
            case STARTDT_ACT_BYTE: return ServiceType::START;
            case STOPDT_ACT_BYTE:  return ServiceType::STOP;
            case TESTFR_ACT_BYTE:  return ServiceType::TEST;
            default:               return ServiceType::NONE;
        }
    }

    ServiceType Apdu::ServiceConfirmation() const noexcept
    {
        int service = mHeader[2];

        switch (service) {
            case STARTDT_CON_BYTE: return ServiceType::START;
            case STOPDT_CON_BYTE:  return ServiceType::STOP;
            case TESTFR_CON_BYTE:  return ServiceType::TEST;
            default:               return ServiceType::NONE;
        }
    }

    std::optional<Sequence> Apdu::ReceiveSequence() const noexcept
    {
        if (HasPayload() || IsRecvAck())
            return Sequence(mHeader[4], mHeader[5]);
        else
            return {};
    }

    std::optional<Sequence> Apdu::SendSequence() const noexcept
    {
        if (HasPayload())
            return Sequence(mHeader[2], mHeader[3]);
        else
            return {};
    }
}