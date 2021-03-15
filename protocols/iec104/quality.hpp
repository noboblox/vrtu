#ifndef IEC_104_QUALITY_HPP_
#define IEC_104_QUALITY_HPP_

#include <cstdint>

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

        Quality(uint8_t aEncoded)
            : mFlags(aEncoded) {}

        // Construct default: Quality == good
        explicit Quality()
            : mFlags(0) {}

        uint8_t GetEncoded() const noexcept { return mFlags; }

        bool IsGood() const { return mFlags == 0; }
        bool ClearAllFlags() { mFlags = 0; }

        void SetInvalid(bool aState) { aState ? mFlags |= FLAG_INVALID : mFlags &= (~FLAG_INVALID); }
        bool IsInvalid() const { return mFlags & FLAG_INVALID; }

        void SetNotTopical(bool aState) { aState ? mFlags |= FLAG_NOT_TOPICAL : mFlags &= (~FLAG_NOT_TOPICAL); }
        bool IsNotTopical() const { return mFlags & FLAG_NOT_TOPICAL; }

        void SetSubstituted(bool aState) { aState ? mFlags |= FLAG_SUBSTITUTED : mFlags &= (~FLAG_SUBSTITUTED); }
        bool IsSubstituted() const { return mFlags & FLAG_SUBSTITUTED; }

        void SetBlocked(bool aState) { aState ? mFlags |= FLAG_BLOCKED : mFlags &= (~FLAG_BLOCKED); }
        bool IsBlocked() const { return mFlags & FLAG_BLOCKED; }

    private:
        uint8_t mFlags;
    };
}

#endif
