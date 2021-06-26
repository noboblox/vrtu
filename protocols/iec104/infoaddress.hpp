#ifndef IEC104_INFOADDRESS_HPP_
#define IEC104_INFOADDRESS_HPP_

#include <cstdint>

class ByteStream;

namespace IEC104
{
    // Information object address according to IEC 60870-5-101 / -104
    class InfoAddress
    {
    public:
        // Construct from encoded memory
        InfoAddress(const uint8_t* apSource, int aAddressSize);

        // Default construct
        constexpr explicit InfoAddress() noexcept
            : mValue(0), mSize(0)
        {
        }

        // Construct unstructured IOA
        enum class Force {UNSTRUCTURED}; // Avoid confusion with structured constructors
        constexpr explicit InfoAddress(Force, int aValue, int aSize) noexcept
            : mValue(aValue), mSize(aSize)
        {
        }

        // Construct 1-byte IOA.
        constexpr explicit InfoAddress(uint8_t aValue) noexcept
            : mValue(aValue), mSize(1)
        {
        }

        // Construct 2-byte IOA
        InfoAddress(uint8_t aLowByte, uint8_t aHighByte) noexcept
            : mValue(aLowByte), mSize(2)
        {
            mValue |= (aHighByte << 8);
        }

        // Construct 3-byte IOA
        InfoAddress(uint8_t aLowByte, uint8_t aMediumByte, uint8_t aHighByte) noexcept
            : mValue(aLowByte), mSize(3)
        {
            mValue |= (aMediumByte << 8);
            mValue |= (aHighByte << 16);
        }

        constexpr int GetInt() const noexcept { return mValue; }

        InfoAddress(const InfoAddress&) = default;
        InfoAddress(InfoAddress&&) = default;
        InfoAddress& operator=(const InfoAddress&) = default;
        InfoAddress& operator=(InfoAddress&&) = default;

        constexpr bool operator< (const InfoAddress& arOther) const noexcept { return mValue < arOther.mValue; }
        constexpr bool operator> (const InfoAddress& arOther) const noexcept { return mValue > arOther.mValue; }
        constexpr bool operator==(const InfoAddress& arOther) const noexcept { return mValue == arOther.mValue; }
        constexpr bool operator!=(const InfoAddress& arOther) const noexcept { return mValue != arOther.mValue; }
        constexpr bool operator<=(const InfoAddress& arOther) const noexcept { return mValue <= arOther.mValue; }
        constexpr bool operator>=(const InfoAddress& arOther) const noexcept { return mValue >= arOther.mValue; }

        constexpr bool operator!() const noexcept { return !mValue; }

        int GetSize() const noexcept { return mSize; }
        void WriteTo(ByteStream& arOutput) const;

    private:
        int mValue;
        int mSize;
    };

}

#endif