#include "data104.hpp"

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
            auto& r_value = arHandle.GetValue();
            r_value.SetObject();

            r_value.AddMember(JSON::StringRef("value"), 
                              JSON::Value(this->operator*().GetInt()),
                              arHandle.GetAllocator());

            r_value.AddMember(JSON::StringRef("size"),
                JSON::Value(this->operator*().GetSize()),
                arHandle.GetAllocator());
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
            auto& r_value = arHandle.GetValue();
            r_value.SetObject();

            r_value.AddMember(JSON::StringRef("invalid"),
                JSON::Value(mData.IsInvalid()),
                arHandle.GetAllocator());

            r_value.AddMember(JSON::StringRef("not topical"),
                JSON::Value(mData.IsNotTopical()),
                arHandle.GetAllocator());

            r_value.AddMember(JSON::StringRef("substituted"),
                JSON::Value(mData.IsSubstituted()),
                arHandle.GetAllocator());

            r_value.AddMember(JSON::StringRef("blocked"),
                JSON::Value(mData.IsBlocked()),
                arHandle.GetAllocator());
        }
    }
}