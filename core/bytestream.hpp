#ifndef BYTESTREAM_HPP_
#define BYTESTREAM_HPP_

#include <cstdint>
#include <stdexcept>
#include <vector>

class ByteStream
{
public:
    explicit ByteStream() noexcept
        : mIterator(0), mBuffer() {}

    explicit ByteStream(const uint8_t* apBegin, const uint8_t* apEnd)
        : mIterator(0), mBuffer(apBegin, apEnd) {}

    explicit ByteStream(std::initializer_list<uint8_t> l) 
        : mIterator(0), mBuffer(l)
    {
    }

    /* Buffer status API (for error handling) */

    /// Overall size of the internal buffer
    inline size_t BufferSize() const noexcept
    {
        return mBuffer.size();
    }

    inline void Reserve(size_t aBytes)
    {
        return mBuffer.reserve(aBytes);
    }

    /// Begin iterator of the internal buffer
    inline const uint8_t* Begin() const noexcept
    {
        return mBuffer.empty() ? nullptr : mBuffer.data();
    }

    /// End iterator of the internal buffer
    inline const uint8_t* End() const noexcept
    {
        return mBuffer.empty() ? nullptr : (mBuffer.data() + mBuffer.size());
    }

    /// Current iterator position inside the internal buffer
    /// Do not use for data access! The iterator is not incremented
    inline const uint8_t* Iterator() const noexcept
    {
        return mBuffer.empty() ? nullptr : (mBuffer.data() + mIterator);
    }

    /* Data access API */

    /// Reset the stream iterator 
    void ResetIterator() noexcept
    {
        mIterator = 0;
    }

    inline size_t RemainingBytes() const noexcept
    {
        return mBuffer.size() - mIterator;
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
    
    void WriteByte(uint8_t arByte)
    {
        mBuffer.push_back(arByte);
    }

    const uint8_t* ReadData(size_t aBytes)
    {
        if (RemainingBytes() < aBytes)
            throw std::out_of_range("tried to read more data than available");

        const uint8_t* p_result = Iterator();
        mIterator += aBytes;
        return p_result;
    }

    void WriteData(const uint8_t* apData, size_t aBytes)
    {
        if (apData)
        {
            mBuffer.reserve(mBuffer.size() + aBytes);
            mBuffer.insert(mBuffer.end(), apData, apData + aBytes);
        }
    }

    uint8_t PeekAt(size_t index) const {
        if (RemainingBytes() <= index)
            throw std::out_of_range("no data at requested index");

        return Iterator()[index];
    }

private:
    size_t mIterator;
    std::vector<uint8_t> mBuffer;
};

#endif

