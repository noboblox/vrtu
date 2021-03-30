#include "core/data.hpp"

#include <algorithm>
#include <ostream>

BaseData::BaseData(const std::string& arName) noexcept
    : mpParent(nullptr), mChilds(), mName(arName), mValid(false)
{
}

BaseData::BaseData(BaseData& arParent, const std::string arName)
    : mpParent(&arParent), mChilds(), mName(arName), mValid(false)
{
    arParent.AddChild(*this);
}

BaseData::~BaseData()
{
}

bool BaseData::IsRoot() const noexcept
{
    return GetParent() == nullptr;
}

const BaseData* BaseData::GetParent() const noexcept
{
    return mpParent;
}

BaseData* BaseData::GetParent() noexcept
{
    return mpParent;
}

std::string BaseData::GetPath(char aSeparator) const
{
    std::string result(GetName());

    if (!IsRoot())
        mpParent->PrependPathForChild(*this, aSeparator, result);
    
    return result;
}

void BaseData::PrependPathForChild(const BaseData& arChild, char aSeparator, std::string& arModifiedPath) const
{
    if (std::find(mChilds.cbegin(), mChilds.cend(), &arChild) == mChilds.cend())
        throw std::invalid_argument("Data element is not a child of this instance.");

    arModifiedPath.insert(0, 1, aSeparator);
    arModifiedPath.insert(0, GetName());

    if (!IsRoot())
        GetParent()->PrependPathForChild(*this, aSeparator, arModifiedPath);
}

std::ostream& BaseData::Serialize(std::ostream& arOutput) const
{
    JSON::Document root;
    auto handle = root.GetValueHandle();
    WriteJson(handle);

    return root.Serialize(arOutput);
}

void BaseData::AddChild(BaseData& arChild)
{
    mChilds.push_back(&arChild);
    arChild.mpParent = this;
}

void DataStruct::WriteJson(JSON::ValueHandle& arHandle) const
{
    for (const auto& rp_child : mChilds)
    {
        auto child_json = arHandle.AddMember(rp_child->GetName());
        rp_child->WriteJson(child_json);
    }
}


