#ifndef DATA_HPP_
#define DATA_HPP_

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <vector>

#include "core/bytestream.hpp"
#include "external/json_wrapper.hpp"

class BaseData
{
public:
    using ChildContainer = std::vector<BaseData*>;

public:
    virtual ~BaseData();

    /// Check if this element contains valid data
    virtual bool IsValid() const noexcept { return mValid; }

    /// Check if this element has a parent (== is not the root element)
    bool IsRoot() const noexcept;
    const BaseData* GetParent() const noexcept;
    BaseData* GetParent() noexcept;

    /// Stream this object as root object into an ostream
    std::ostream& Serialize(std::ostream& arOutput) const;
    
    /// Write this object into a JSON value
    virtual void WriteJson(JSON::ValueHandle& arValue) const = 0;

    const std::string& GetName() const noexcept { return mName; }
    
    /// Get the full path to this data instance within the tree
    std::string GetPath(char aSeparator = '/') const;

    virtual void Clear() noexcept { SetValid(false); }

    /// Prepend path own subpath originating from a specific child node.
    /// This function should be called recursively, while navigating upwards.
    /// It can be overriden for special casing (e.g. array with [] indexing)
    virtual void PrependPathForChild(const BaseData& arChild, char aSeparator, std::string& arModifiedPath) const;

protected:
    explicit BaseData(const std::string& arName) noexcept;
    explicit BaseData(BaseData& arParent, const std::string arName);

    void AddChild(BaseData& arChild);
    void SetValid(bool aState) noexcept { mValid = aState; }

protected:
    BaseData* mpParent;
    std::vector<BaseData*> mChilds;

private:
    std::string mName;
    bool mValid;
};

class DataStruct : public BaseData
{
public:
    explicit DataStruct(const std::string& arName) noexcept
        : BaseData(arName) {}

    explicit DataStruct(BaseData& arParent, const std::string& arName)
        : BaseData(arParent, arName) {}

protected:
    void WriteJson(JSON::ValueHandle& arHandle) const override;
};

template <typename DATA>
class DataArray : public BaseData
{
public:
    explicit DataArray(const std::string& arName) noexcept
        : BaseData(arName) {}

    explicit DataArray(BaseData& arParent, const std::string& arName)
        : BaseData(arParent, arName) {}

    void Append(std::shared_ptr<DATA> apData)
    {
        if (!apData)
            throw std::invalid_argument("nullptr");

        if (!apData->IsRoot())
            throw std::invalid_argument("Element already has a parent");

        mData.push_back(apData);
        AddChild(*apData);
    }

    DATA& Get(size_t aIndex) { return *(mData.at(aIndex)); }
    const DATA& Get(size_t aIndex) const { return *(mData.at(aIndex)); }
    size_t GetSize() const noexcept { return mData.size(); }
    bool IsEmpty() const noexcept { return mData.empty(); }

    void WriteJson(JSON::ValueHandle& arHandle) const override
    {
        for (const auto& rp_child : mChilds)
        {
            auto added = arHandle.PushBack();
            rp_child->WriteJson(added);
        }
    }

    void PrependPathForChild(const BaseData& arChild, char aSeparator, std::string& arModifiedPath) const override
    {
        auto it_child = std::find(mChilds.cbegin(), mChilds.cend(), &arChild);

        if (it_child == mChilds.cend())
            throw std::invalid_argument("Data element is not a child of this instance.");

        std::string subpath(GetName());
        subpath += '[';
        subpath += std::to_string(std::distance(mChilds.cbegin(), it_child));
        subpath += ']';

        arModifiedPath.insert(0, 1, aSeparator);
        arModifiedPath.insert(0, subpath);

        if (!IsRoot())
            GetParent()->PrependPathForChild(*this, aSeparator, arModifiedPath);
    }

private:
    std::vector<std::shared_ptr<DATA>> mData;
};

/*
* Tree managing class for data which can be default constructed and simply assigned, without any special treatment
*/
template<typename DATA>
class SimpleData : public BaseData
{
public:
    explicit SimpleData(BaseData& arParent, const std::string& arName)
        : BaseData(arParent, arName), mData() {}

    using data_type = DATA;

    SimpleData<data_type>& operator=(const data_type& arData)
    {
        mData = arData;
        SetValid(true);
        return *this;
    }

    const data_type& operator*() const 
    {
        if (!IsValid())
            throw std::runtime_error("Tried to access invalid data");
        return mData; 
    }

    void WriteJson(JSON::ValueHandle& arHandle) const override
    {
        if (IsValid())
        {
            arHandle.Set(mData);
        }
    }

private:
    data_type mData;
};

// TODO Remove
template<typename DATA>
class DummyData : public BaseData
{
public:
    explicit DummyData(BaseData& arParent, const std::string& arName)
        : BaseData(arParent, arName), mData() {}

    using data_type = DATA;

    DummyData<data_type>& operator=(const data_type& arData)
    {
        mData = arData;
        SetValid(true);
        return *this;
    }

    const data_type& operator*() const
    {
        if (!IsValid())
            throw std::runtime_error("Tried to access invalid data");
        return mData;
    }

    void WriteJson(JSON::ValueHandle& arValue) const override
    {
    }

private:
    data_type mData;
};

// general data
using DataInt  = SimpleData<int>;
using DataByte = SimpleData<uint8_t>;
using DataFloat = SimpleData<float>;
using DataBool = SimpleData<bool>;

template <typename NAMED_ENUM>
class DataEnum : public BaseData
{
public:
    explicit DataEnum(BaseData& arParent, const std::string& arName)
        : BaseData(arParent, arName), mData() {}

    using enum_type = NAMED_ENUM;

    DataEnum<enum_type>& operator=(const enum_type& arData)
    {
        mData = arData;
        SetValid(true);
        return *this;
    }

    const enum_type& operator*() const
    {
        if (!IsValid())
            throw std::runtime_error("Tried to access invalid data");
        return mData;
    }

    void WriteJson(JSON::ValueHandle& arHandle) const override
    {
        if (IsValid())
        {
            auto label = arHandle.AddMember("label");
            label.Set(mData.GetLabel());

            auto value = arHandle.AddMember("value");
            value.Set(static_cast<intmax_t>(mData.GetValue()));
        }
    }

private:
    enum_type mData;
};
#endif
