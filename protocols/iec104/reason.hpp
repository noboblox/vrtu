#ifndef IEC104_REASON_HPP_
#define IEC104_REASON_HPP_

#include "protocols/iec104/104enums.hpp"

namespace IEC104
{
    class Reason
    {
    public:
        explicit Reason()
            : mCode(ReasonCode::SPONTANEOUS), mIsTest(false), mIsNegative(false)
        {
        }
        
        Reason(ReasonCodeEnum aReasonCode, bool aIsNegative = false, bool aIsTest = false)
            : mCode(aReasonCode), mIsTest(aIsTest), mIsNegative(aIsNegative)
        {
        }

        bool IsTest() const noexcept { return mIsTest; }
        bool IsNegative() const noexcept { return mIsNegative; }

        ReasonCodeEnum GetReasonCode() const noexcept { return mCode; }
        void ChangeReasonCode(ReasonCodeEnum aNewCode) { mCode = aNewCode; }

    private:
        ReasonCodeEnum mCode;
        bool mIsTest;
        bool mIsNegative;
    };
}

#endif
