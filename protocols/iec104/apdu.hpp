#ifndef IEC104_APDU_HPP_
#define IEC104_APDU_HPP_

#include <cstdint>
#include <optional>
#include "protocols/iec104/sequence.hpp"
#include "protocols/iec104/servicetype.hpp"

class ByteStream;

namespace IEC104
{
    class Apdu
    {
    public:
        static constexpr size_t HEADER_SIZE = 6;
        static const Apdu STARTDT_ACT;
        static const Apdu STOPDT_ACT;
        static const Apdu TESTFR_ACT;
        static const Apdu STARTDT_CON;
        static const Apdu STOPDT_CON;
        static const Apdu TESTFR_CON;
        
        static bool IsFullyAvailable(const ByteStream& buf);

        // create from network stream
        explicit Apdu(ByteStream& buf);
        // create Recv Ack APDU (S-Frame)
        explicit Apdu(Sequence recv);

        ~Apdu();

        size_t Length() const noexcept;
        size_t PayloadLength() const noexcept;
        size_t HeaderLength() const noexcept { return HEADER_SIZE; }

        void WriteTo(std::vector<uint8_t>& dest) const;

        bool IsValid() const noexcept;

        bool HasPayload() const noexcept;
        bool IsRecvAck() const noexcept;

        ServiceType ServiceActivation() const noexcept;
        ServiceType ServiceConfirmation() const noexcept;

        std::optional<Sequence> ReceiveSequence() const noexcept;
        std::optional<Sequence> SendSequence() const noexcept;

    private:
        static constexpr uint8_t MAX_PAYLOAD_SIZE = 255;

        static constexpr uint8_t STARTDT_ACT_BYTE = 0x07;
        static constexpr uint8_t STARTDT_CON_BYTE = 0x0B;
        static constexpr uint8_t STOPDT_ACT_BYTE  = 0x13;
        static constexpr uint8_t STOPDT_CON_BYTE  = 0x23;
        static constexpr uint8_t TESTFR_ACT_BYTE  = 0x43;
        static constexpr uint8_t TESTFR_CON_BYTE  = 0x83;

        // construct a service Apdu
        explicit Apdu(uint8_t service) noexcept;

        uint8_t mHeader[HEADER_SIZE] = { 0x68, 0x04, 0x00, 0x00, 0x00, 0x00 };
        uint8_t mPayload[MAX_PAYLOAD_SIZE] = {};
    };
}

#endif