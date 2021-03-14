#include "protocols/iec104/data104.hpp"

namespace IEC104
{
    DataInfoAddress::DataInfoAddress(BaseData& arParent, const std::string& arName)
        : BaseData(arParent, arName), mData()
    {
    }

    DataInfoAddress& DataInfoAddress::operator=(const InfoAddress& arData)
    {
        mData = arData;
        SetValid(true);
        return *this;
    }

    const InfoAddress& DataInfoAddress::operator*() const
    {
        if (!IsValid())
            throw std::runtime_error("Tried to access invalid data");

        return mData;
    }

    void DataInfoAddress::WriteJson(JSON::ValueHandle& arHandle) const
    {
        if (IsValid())
        {
            auto address = arHandle.AddMember("value");
            address.Set(this->operator*().GetInt());

            auto size = arHandle.AddMember("size");
            size.Set(this->operator*().GetSize());
        }
    }

    DataQuality::DataQuality(BaseData& arParent, const std::string& arName)
        : BaseData(arParent, arName), mData()
    {
    }

    DataQuality& DataQuality::operator=(const Quality& arData)
    {
        mData = arData;
        SetValid(true);
        return *this;
    }

    const Quality& DataQuality::operator*() const
    {
        if (!IsValid())
            throw std::runtime_error("Tried to access invalid data");

        return mData;
    }

    void DataQuality::WriteJson(JSON::ValueHandle& arHandle) const
    {
        if (IsValid())
        {
            auto quality_iv = arHandle.AddMember("invalid");
            quality_iv.Set(mData.IsInvalid());

            auto quality_nt = arHandle.AddMember("not topical");
            quality_nt.Set(mData.IsNotTopical());

            auto quality_sb = arHandle.AddMember("substituted");
            quality_sb.Set(mData.IsSubstituted());

            auto quality_bl = arHandle.AddMember("blocked");
            quality_bl.Set(mData.IsBlocked());
        }
    }
}