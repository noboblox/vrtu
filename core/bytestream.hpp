#ifndef BYTESTREAM_HPP_
#define BYTESTREAM_HPP_

#include <cstdint>
#include <stdexcept>
#include <vector>

class ByteStream
{
public:
    // initialize a bytestream with default capacity but no data
    explicit ByteStream() noexcept
        : mBuffer(1024, 0), mBegin(0), mEnd(0) {}

    // initialize a bytestream with capacity but no data
    explicit ByteStream(size_t capacity) noexcept
        : mBuffer(capacity, 0), mBegin(0), mEnd(0) {}


    // initialize the bytestream with available data
    explicit ByteStream(const uint8_t* apBegin, const uint8_t* apEnd)
        : mBuffer(apBegin, apEnd), mBegin(0), mEnd(mBuffer.size()) {
    }

    // initialize the bytestream with available data
    explicit ByteStream(std::initializer_list<uint8_t> l) 
        : mBuffer(l), mBegin(0), mEnd(mBuffer.size())
    {
    }

    /// Overall size of the internal buffer
    inline size_t Capacity() const noexcept
    {
        return mBuffer.size();
    }

    inline void Reserve(size_t capacity)
    {
        if (capacity > mBuffer.size())
            return mBuffer.resize(capacity);
    }

    inline uint8_t* WriteBegin() noexcept
    {
        return mBuffer.data() + mEnd;
    }

    inline uint8_t* WriteEnd() noexcept
    {
        return mBuffer.data() + mBuffer.size();
    }

    inline const uint8_t* WriteBegin() const noexcept
    {
        return mBuffer.data() + mEnd;
    }

    inline const uint8_t* WriteEnd() const noexcept
    {
        return mBuffer.data() + mBuffer.size();
    }


    inline void BytesWritten(size_t count)
    {
        if (count > WritableBytes())
            throw std::invalid_argument("cannot have written amount of bytes without out of bounds write");
        mEnd += count;
    }

    inline const uint8_t* MemoryBegin() const noexcept
    {
        return mBuffer.data();
    }

    inline const uint8_t* MemoryEnd() const noexcept
    {
        return mBuffer.data() + mBuffer.size();
    }

    inline const uint8_t* DataBegin() const noexcept
    {
        return mBuffer.data() + mBegin;
    }

    inline const uint8_t* DataEnd() const noexcept
    {
        return mBuffer.data() + mEnd;
    }

    void Flush() noexcept
    {
        if (mBegin < mEnd)
            std::memmove(mBuffer.data(), mBuffer.data() + mBegin, mEnd - mBegin);

        mEnd -= mBegin;
        mBegin = 0;
    }

    inline size_t RemainingBytes() const noexcept
    {
        return mEnd - mBegin;
    }

    inline size_t WritableBytes() const noexcept
    {
        return std::distance(WriteBegin(), WriteEnd());
    }


    // Read a single byte from stream
    uint8_t ReadByte()
    {
        return *ReadData(1);
    }

    void ReadInto(uint8_t* dest, size_t count)
    {
        auto data = ReadData(count);
        std::memcpy(dest, data, count);
    }

    /*
     * @brief Read one or multiple bytes and interpret them as the provided type 
     * 
     * @note In most cases its easier to use ReadByte(). 
     * When integers / floats are decoded directly from single bytes, instead of being read completely at once, 
     * we never need to care about endianess at all (At least for IEC104, which already defines the byte order).
     * 
     * This makes handling multibyte values much more straightforward, because the code does always the same (no conditionals).
     */
    template <typename POD>
    const POD& ReadAs()
    {
        static_assert(std::is_trivially_copyable<POD>::value, "ReadAs is not allowed for non-trivially-copyable types");
        return *reinterpret_cast<const POD*>(ReadData(sizeof(POD)));
    }
    
    void WriteByte(uint8_t byte)
    {
        WriteData(&byte, 1);
    }

    const uint8_t* ReadData(size_t aBytes)
    {
        if (RemainingBytes() < aBytes)
            throw std::out_of_range("tried to read more data than available");

        const uint8_t* p_result = DataBegin();
        mBegin += aBytes;
        return p_result;
    }

    void WriteData(const void* apData, size_t aBytes)
    {
        if (apData && aBytes > 0)
        {
            Reserve(mEnd + aBytes);
            std::memcpy(WriteBegin(), apData, aBytes);
            BytesWritten(aBytes);
        }
    }

    uint8_t PeekAt(size_t index) const {
        if (RemainingBytes() <= index)
            throw std::out_of_range("no data at requested index");

        return DataBegin()[index];
    }

private:
    std::vector<uint8_t> mBuffer;
    size_t mBegin = 0;
    size_t mEnd = 0;
};

#endif

