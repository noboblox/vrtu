#ifndef IEC_104_QUALITY_HPP_
#define IEC_104_QUALITY_HPP_

#include <cstdint>
#include <string>

namespace IEC104
{
    class Quality
    {
    public:
        enum Flags
        {
            FLAG_OVERFLOW = 0x01,
            FLAG_BLOCKED = 0x10,
            FLAG_SUBSTITUTED = 0x20,
            FLAG_NOT_TOPICAL = 0x40,
            FLAG_INVALID = 0x80,
        };

        Quality(uint8_t aEncoded) noexcept
            : mFlags(aEncoded) {}

        // Construct default: Quality == good
        explicit Quality() noexcept
            : mFlags(0) {}

        uint8_t GetEncoded() const noexcept { return mFlags; }

        bool IsGood() const noexcept { return mFlags == 0; }
        void ClearAllFlags() noexcept { mFlags = 0; }

        void SetInvalid(bool aState) noexcept { aState ? mFlags |= FLAG_INVALID : mFlags &= (~FLAG_INVALID); }
        bool IsInvalid() const noexcept { return mFlags & FLAG_INVALID; }

        void SetNotTopical(bool aState) noexcept { aState ? mFlags |= FLAG_NOT_TOPICAL : mFlags &= (~FLAG_NOT_TOPICAL); }
        bool IsNotTopical() const noexcept { return mFlags & FLAG_NOT_TOPICAL; }

        void SetSubstituted(bool aState) noexcept { aState ? mFlags |= FLAG_SUBSTITUTED : mFlags &= (~FLAG_SUBSTITUTED); }
        bool IsSubstituted() const noexcept { return mFlags & FLAG_SUBSTITUTED; }

        void SetBlocked(bool aState) noexcept { aState ? mFlags |= FLAG_BLOCKED : mFlags &= (~FLAG_BLOCKED); }
        bool IsBlocked() const noexcept { return mFlags & FLAG_BLOCKED; }

        void SetOverflow(bool aState) noexcept { aState ? mFlags |= FLAG_OVERFLOW : mFlags &= (~FLAG_OVERFLOW); }
        bool IsOverflow() const noexcept { return mFlags & FLAG_OVERFLOW; }

        std::string ToString() const
        {
            static constexpr size_t LABEL_DISTANCE = 4;

            std::string result("{--, --, --, --, --}");
            size_t pos = 1;

            if (IsInvalid()) result.replace(pos, 2, "IV");
            pos += LABEL_DISTANCE;

            if (IsNotTopical()) result.replace(pos, 2, "NT");
            pos += LABEL_DISTANCE;

            if (IsSubstituted()) result.replace(pos, 2, "SB");
            pos += LABEL_DISTANCE;

            if (IsBlocked()) result.replace(pos, 2, "BL");
            pos += LABEL_DISTANCE;

            if (IsOverflow()) result.replace(pos, 2, "OV");

            return result;
        }

    private:
        uint8_t mFlags;
    };
}

#endif
