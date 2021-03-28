#ifndef IEC104_APDU_HPP_
#define IEC104_APDU_HPP_

#include <cstdint>
#include <memory>
#include <string>

class ByteStream;

namespace IEC104
{
    class Asdu;

    class Apdu
    {
    public:
        static constexpr int GetNeededSize() noexcept { return sizeof(mData); }
        static constexpr int MAX_SEQUENCE = 0x7FFF;

        static inline void IncrementSequence(int& arSequence)
        {
            if (arSequence >= MAX_SEQUENCE)
                arSequence = 0;
            else
                ++arSequence;
        }

        explicit Apdu() noexcept;
        Apdu(const uint8_t* apData, size_t aSize) noexcept;

        void Assign(const uint8_t* apData, size_t aSize);

        bool IsValid() const noexcept;

        bool HasReceiveCounter() const noexcept;
        int GetReceiveCounter() const noexcept;
        void SetReceiveCounter(int aValue) noexcept;

        bool HasSendCounter() const noexcept;
        int GetSendCounter() const noexcept;
        void SetSendCounter(int aValue) noexcept;

        const void* GetData() const noexcept;

        bool ReadAsdu(ByteStream& arInput) noexcept;

        size_t GetAsduSize() const noexcept;
        bool HasAsdu() const noexcept;
        const Asdu* GetAsdu() const noexcept;

        bool NeedsConfirmation() const noexcept;
        void ConvertToConfirmation() noexcept;

        std::string TypeToString() const;

    protected:
        enum MessageType
        {
            INVALID = -1,

            DATA = 0,

            RECEIVE_CONFIRM = 0x01,

            STARTDT_REQUEST = 0x07,
            STARTDT_CONFIRM = 0x0B,
            STOPDT_REQUEST = 0x13,
            STOPDT_CONFIRM = 0x23,
            TESTFR_REQUEST = 0x43,
            TESTFR_CONFIRM = 0x83,
        };

        void SetType(MessageType aType) noexcept;

    private:
        int ReadSequenceInternal(const uint8_t* apBegin) const noexcept;
        void WriteSequenceInternal(uint8_t* apBegin, int aValue) noexcept;
        MessageType VerifyMessage() noexcept;

        MessageType mType;
        uint8_t mData[6];
        std::unique_ptr<Asdu> mpAsdu;
    };

    class ReceiveConfirmation : public Apdu
    {
    public:
        explicit ReceiveConfirmation(int aReceiveCounter);
    };
}

#endif