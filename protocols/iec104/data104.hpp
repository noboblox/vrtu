#ifndef IEC104_DATA104_HPP_
#define IEC104_DATA104_HPP_

#include "core/data.hpp"

namespace IEC104
{
    class DataInfoAddress : public BaseData
    {
    public:
        DataInfoAddress(BaseData& arParent, const std::string& arName);

        DataInfoAddress& operator=(const InfoAddress& arData);
        const InfoAddress& operator*() const;

        void WriteJson(JSON::ValueHandle& arHandle) const override;

    private:
        InfoAddress mData;
    };

    class DataQuality : public BaseData
    {
    public:
        DataQuality(BaseData& arParent, const std::string& arName);

        DataQuality& operator=(const Quality& arData);
        const Quality& operator*() const;

        void WriteJson(JSON::ValueHandle& arHandle) const override;

    private:
        Quality mData;
    };

    using DataReason = DummyData<IEC104::Reason>;
}

#endif

