#ifndef DATA_HPP_
#define DATA_HPP_

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <vector>

#include "external/json_wrapper.hpp"

#include "bytestream.hpp"
#include "infoaddress.hpp"
#include "reason.hpp"

class BaseData
{
  // TODO: Separate this class tree from direct references to extrernal JSON library?

public:
    using ChildContainer = std::vector<BaseData*>;

public:
    virtual ~BaseData();

    /// Check if this element contains valid data
    virtual bool IsValid() const noexcept { return mValid; }

    /// Check if this element has a parent (== is not the root element)
    bool IsRoot() const noexcept;

    /// Stream this object as root object into an ostream
    std::ostream& Serialize(std::ostream& arOutput) const;
    
    /// Write this object into a JSON value
    virtual void WriteJson(JSON::ValueHandle& arValue) const = 0;

    const std::string& GetName() const noexcept { return mName; }
    virtual void Clear() noexcept { SetValid(false); }

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
        auto& r_current = arHandle.GetValue();
        r_current.SetArray();

        for (const auto& rp_child : mChilds)
        {
            r_current.PushBack(JSON::Value(), arHandle.GetAllocator());

            auto p_child = (r_current.End() - 1); // Reverse begin
            JSON::ValueHandle handle(*p_child, arHandle.GetAllocator());
            rp_child->WriteJson(handle);
        }
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
            auto& r_value = arHandle.GetValue();
            r_value.Set(mData);
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
            auto& r_value = arHandle.GetValue();

            r_value.SetObject();
            r_value.AddMember(JSON::StringRef("label"), 
                              JSON::Value(JSON::StringRef(mData.GetLabel())), 
                              arHandle.GetAllocator()); // Data is assumed to be unchanged until the document is serialized!
            r_value.AddMember(JSON::StringRef("value"), JSON::Value(static_cast<intmax_t>(mData.GetValue())), // Convert into the biggest possible integer type
                              arHandle.GetAllocator());
        }
    }

private:
    enum_type mData;
};

class Quality
{
public:
    enum Flags
    {
        FLAG_OVERFLOW    = 0x01,
        FLAG_BLOCKED     = 0x10,
        FLAG_SUBSTITUTED = 0x20,
        FLAG_NOT_TOPICAL = 0x40,
        FLAG_INVALID     = 0x80,
    };

    Quality(uint8_t aEncoded)
        : mFlags(aEncoded) {}

    // Construct default: Quality == good
    explicit Quality()
        : mFlags(0) {}

    uint8_t GetEncoded() const noexcept { return mFlags; }

    bool IsGood() const { return mFlags == 0; }
    bool ClearAllFlags() { mFlags = 0; }

    void SetInvalid(bool aState) { aState ? mFlags |= FLAG_INVALID : mFlags &= (~FLAG_INVALID); }
    bool IsInvalid() const { return mFlags & FLAG_INVALID; }

    void SetNotTopical(bool aState) { aState ? mFlags |= FLAG_NOT_TOPICAL : mFlags &= (~FLAG_NOT_TOPICAL); }
    bool IsNotTopical() const { return mFlags & FLAG_NOT_TOPICAL; }

    void SetSubstituted(bool aState) { aState ? mFlags |= FLAG_SUBSTITUTED : mFlags &= (~FLAG_SUBSTITUTED); }
    bool IsSubstituted() const { return mFlags & FLAG_SUBSTITUTED; }

    void SetBlocked(bool aState) { aState ? mFlags |= FLAG_BLOCKED : mFlags &= (~FLAG_BLOCKED); }
    bool IsBlocked() const { return mFlags & FLAG_BLOCKED; }

private:
    uint8_t mFlags;
};
#endif
