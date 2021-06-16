#ifndef IEC104_CONNECTIONCONFIG_HPP_
#define IEC104_CONNECTIONCONFIG_HPP_

namespace IEC104
{
    class ConnectionConfig
    {
    public:
        static const ConnectionConfig DefaultConnectionConfig;

        ConnectionConfig(int aT0, int aT1, int aT2,
                         int aT3, int aK, int aW);

        void SetT0(int aT0);
        int GetT0() const noexcept { return mT0; }

        void SetT1(int aT1);
        int GetT1() const noexcept { return mT1; }
        
        void SetT2(int aT2);
        int GetT2() const noexcept { return mT2; }

        void SetT3(int aT3);
        int GetT3() const noexcept { return mT3; }

        void SetK(int aK);
        int GetK() const noexcept { return mK; }

        void SetW(int aW);
        int GetW() const noexcept { return mW; }

    private:
        int mT0; //!< Active connect timeout [1s - 255s]
        int mT1;
        int mT2;
        int mT3;
        int mK;
        int mW;
    };

}
#endif

