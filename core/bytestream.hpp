#ifndef BYTESTREAM_HPP_
#define BYTESTREAM_HPP_

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "errornomoredata.hpp"

class ByteStream
{
public:
    explicit ByteStream()
        : mIterator(0), mBuffer() {}

    void ResetIterator()
    {
        mIterator = 0;
    }
    inline size_t RemainingBytes() const
    {
        return mBuffer.size() - mIterator;
    }

    // Read a single byte from stream
    uint8_t ReadByte()
    {
        return *ReadData(1);
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
            throw ErrorNoMoreData(mBuffer.size(), mBuffer.data(), mBuffer.data() + mIterator);

        const uint8_t* p_result = (mBuffer.data() + mIterator);
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

private:
    size_t mIterator;
    std::vector<uint8_t> mBuffer;
};

#endif

