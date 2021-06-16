#include "connectionconfig.hpp"

#include "core/util.hpp"

namespace IEC104
{
    const ConnectionConfig ConnectionConfig::DefaultConnectionConfig =
        ConnectionConfig(30, 15, 10, 30, 12, 8);


    ConnectionConfig::ConnectionConfig(int aT0, int aT1, int aT2,
                                       int aT3, int aK, int aW)
      : mT0(0), mT1(0), mT2(0), mT3(0),
        mK(0), mW(0)
    {
        SetT0(aT0);
        SetT1(aT1);
        SetT2(aT2);
        SetT3(aT3);
        SetK(aK);
        SetW(aW);
    }

    void ConnectionConfig::SetT0(int aT0)
    {
        UTIL::AssertRange(1, 255, aT0);
        mT0 = aT0;
    }

    void ConnectionConfig::SetT1(int aT1)
    {
        UTIL::AssertRange(mT2 + 1, 255, aT1);
        mT1 = aT1;
    }

    void ConnectionConfig::SetT2(int aT2)
    {
        UTIL::AssertRange(1, mT1 - 1, aT2);
        mT2 = aT2;
    }

    void ConnectionConfig::SetT3(int aT3)
    {
        UTIL::AssertRange(0, 10000, aT3);
        mT3 = aT3;
    }

    void ConnectionConfig::SetK(int aK)
    {
        UTIL::AssertRange(mW + 1, 1000, aK);
        mK = aK;
    }

    void ConnectionConfig::SetW(int aW)
    {
        UTIL::AssertRange(1, std::min(800, (mK - 1)), aW);
        mW = aW;
    }
}