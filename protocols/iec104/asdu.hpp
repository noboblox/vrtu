#ifndef IEC104_ASDU_HPP_
#define IEC104_ASDU_HPP_

#include "core/data.hpp"
#include "protocols/iec104/data104.hpp"
#include "protocols/iec104/infoobjects.hpp"

class ByteStream;

namespace IEC104
{
    class AsduConfig
    {
    public:
        AsduConfig(int aReasonSize, int aCASize, int aIOASize);

        AsduConfig(const AsduConfig& arOther) = default;
        AsduConfig(AsduConfig&& arOther) = default;
        AsduConfig& operator=(const AsduConfig& arOther) = default;
        AsduConfig& operator=(AsduConfig&& arOther) = default;

        static AsduConfig Defaults;

        int GetReasonSize() const noexcept { return mReasonSize; }
        void SetReasonSize(int aValue);

        int GetCASize() const noexcept { return mCASize; }
        void SetCASize(int aValue);

        int GetIOASize() const noexcept { return mIOASize; }
        void SetIOASize(int aValue);

    private:
        void RequireRange(int aMin, int aMax, int aChecked) const;

        int mReasonSize;
        int mCASize;
        int mIOASize;
    };

    class Asdu : public DataStruct
    {
    public:
        Asdu(const AsduConfig& arConfig);

        void ReadFrom(ByteStream& arBuffer);
        void WriteTo(ByteStream& arBuffer) const;

        // TODO
        bool HasMoreSpace() const;
        int Append(const SharedInfoObject& arInfoObj);

    private:
        void ReadHeader(ByteStream& arBuffer);
        void WriteHeader(ByteStream& arBuffer) const;
        int GetExpectedSize() const;

    private:
        AsduConfig mConfig;

        DataEnum<TypeEnum> mType;
        DataInt    mSize;
        DataBool   mIsSequence;
        DataReason mReason;
        DataInt    mOrigin;
        DataInt    mCommonAddress;
        DataArray<BaseInfoObject> mObjects;
    };
}

#endif

