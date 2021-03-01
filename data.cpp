#include "data.hpp"

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
    JSON::ValueHandle begin(root);
    WriteJson(begin);

    JSON::OStreamWrapper output(arOutput);
    JSON::PrettyWriter<JSON::OStreamWrapper> writer(output);
    root.Accept(writer);
    return arOutput;
}

void BaseData::AddChild(BaseData& arChild)
{
    mChilds.push_back(&arChild);
    arChild.mpParent = this;
}

void DataStruct::WriteJson(JSON::ValueHandle& arHandle) const
{
    JSON::Value& r_current = arHandle.GetValue();
    r_current.SetObject();

    for (const auto& rp_child : mChilds)
    {
        // Add new key/value pair with NULL value
        r_current.AddMember(JSON::StringRef(rp_child->GetName()), JSON::Value(), arHandle.GetAllocator());

        // Fill the json element
        auto child_iter = r_current.FindMember(rp_child->GetName());
        JSON::ValueHandle handle(child_iter->value, arHandle.GetAllocator());
        rp_child->WriteJson(handle);
    }
}

