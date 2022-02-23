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
        int mReasonSize;
        int mCASize;
        int mIOASize;
    };

    class Asdu : public DataStruct
    {
    public:
        using Iterator = DataArray<BaseInfoObject>::Iterator;

        Asdu(const AsduConfig& arConfig = AsduConfig::Defaults);

        void ReadFrom(ByteStream& arBuffer);
        void WriteTo(ByteStream& arBuffer) const;

        TypeEnum GetType() const;
        int GetNumberOfInfoObjects() const noexcept;

        bool IsSequence() const {return *mIsSequence;}
        int GetObjectCount() const {return *mSize;}
        ReasonCodeEnum GetReason() const {return *mReason;}
        int GetAddress() const {return *mCommonAddress;}

        Iterator Begin() const { return mObjects.Begin(); }
        Iterator End()  const { return mObjects.End(); }

        // TODO
        bool HasMoreSpace() const;
        int Append(const SharedInfoObject& arInfoObj);

    private:
        void ReadHeader(ByteStream& arBuffer);
        void WriteHeader(ByteStream& arBuffer) const;
        unsigned GetExpectedSize() const;
    private:
        AsduConfig mConfig;

        DataEnum<TypeEnum> mType;
        DataUnsigned mSize;
        DataBool   mIsSequence;
        DataEnum<ReasonCodeEnum> mReason;
        DataInt    mOrigin;
        DataInt    mCommonAddress;
        DataArray<BaseInfoObject> mObjects;
    };
}

#endif

