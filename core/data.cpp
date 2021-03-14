#include "core/data.hpp"

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
    return mpParent == nullptr;
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

