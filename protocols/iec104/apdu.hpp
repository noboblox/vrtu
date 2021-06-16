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
        static constexpr size_t GetHeaderSize() noexcept { return sizeof(mHeader); }
        static constexpr size_t GetMaximumMsgSize() noexcept { return 0xFF; }
        
        // Get the size of the complete message including header
        // Needs at least a fully received header @see GetHeaderSize()
        static size_t GetMessageSize(const void* apHeader) noexcept
        {
            // 1. IEC104 stores the message size at the 2nd inside the message (offset 1)
            // 2. IEC104 does not count the first 2 bytes into the size (+ 2 needed).

            return reinterpret_cast<const uint8_t*>(apHeader)[1] + 2;
        }

        static constexpr int MAX_SEQUENCE = 0x7FFF;

        static inline void IncrementSequence(int& arSequence) noexcept
        {
            if (arSequence >= MAX_SEQUENCE)
                arSequence = 0;
            else
                ++arSequence;
        }

        static int SequenceDistance(int aLeft, int aRight) noexcept;

        explicit Apdu() noexcept;
        Apdu(const uint8_t* apData, size_t aSize) noexcept;
        Apdu(const Apdu& arOther);
        Apdu(Apdu&& arOther) noexcept;

        ~Apdu();

        void ReadFrom(ByteStream& arSource);

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
        bool IsAsdu() const noexcept;
        const Asdu& GetAsdu() const;

        bool IsServiceRequest() const noexcept;
        Apdu CreateServiceConfirmation() const noexcept;

        std::string TypeToString() const;

    protected:

        /**
         * @brief MessageType with native encoding for IEC 60870-5-104
         */
        enum MessageType
        {
            INVALID = -1,

            DATA            = 0x00, //!< Type I (Type mask 0x00)

            RECEIVE_CONFIRM = 0x01, //!< Type S (Type mask 0x01)

            /**
             * Type U (Type mask 0x03)
             * Bit   |  7     6  |  5     4  |  3     2  | 1     0 |
             *       |  TESTFR   |  STOPDT   |  STARTDT  | Type U  |
             *       | con | act | con | act | con | act | 1  |  1 |
             */
            STARTDT_REQUEST = 0x07,
            STARTDT_CONFIRM = 0x0B,
            STOPDT_REQUEST  = 0x13,
            STOPDT_CONFIRM  = 0x23,
            TESTFR_REQUEST  = 0x43,
            TESTFR_CONFIRM  = 0x83
        };

        void InitHeader(MessageType aType) noexcept;

    private:
        struct Header
        {
            uint8_t mStartByte;  //!< Startbyte 0x68
            uint8_t mSize;       //!< Message size excluding mStartByte and mSize itself. At least sizeof(mControl).
            uint8_t mControl[4]; //!< Encoding defined by IEC 60870-5-104
        };

        /**
         * @brief Read the sequence number from an APDU
         * 
         * The IEC104 sequence encoding stores 15 bit values
         * The bit 0 of the lower byte is not used for sequencing, because it's needed for message identification
         *             7  6  5  4  3  2  1  0
         * apSource[0] X  X  X  X  X  X  X  0  <-- least significant bit @ 1
         * apSource[1] X  X  X  X  X  X  X  X  <-- most  significant bit @ 7

         * 
         * @param[in] apSource Pointer to first byte of the evaluated sequence number
         * @return Sequence number
        */
        int ReadSequenceInternal(const uint8_t* apSource) const noexcept;

        /**
         * @brief Write the sequence number into an APDU
         * 
         * @param [in/out] apDestination Pointer to first byte of destination
         * @param [in] aValue Value to set
        */
        void WriteSequenceInternal(uint8_t* apDestination, int aValue) noexcept;
        MessageType VerifyMessage() noexcept;

        MessageType mType;
        Header mHeader;
        std::unique_ptr<Asdu> mpAsdu;
    };

    class AsduAcknowledgeApdu : public Apdu
    {
    public:
        explicit AsduAcknowledgeApdu(int aReceiveCounter);
    };
}

#endif