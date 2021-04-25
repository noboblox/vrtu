#ifndef CORE_UTIL_HPP_
#define CORE_UTIL_HPP_

#include <stdexcept>

namespace UTIL
{
    template <typename CompareType>
    void AssertRange(CompareType aMin, CompareType aMax, CompareType aChecked)
    {
        if (aChecked < aMin || aChecked > aMax)
            throw std::invalid_argument("Value out of range");
    }
}


#endif
