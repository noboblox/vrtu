#include "json.hpp"

#include <ostream>
#include <stdexcept>

#define RAPIDJSON_HAS_STDSTRING 1
#include "external/rapidjson/rapidjson.h"

#include "external/rapidjson/allocators.h"
#include "external/rapidjson/document.h"
#include "external/rapidjson/ostreamwrapper.h"
#include "external/rapidjson/prettywriter.h"
#include "external/rapidjson/writer.h"


namespace JSON
{

    static inline ValueHandleImpl& GetValueHandleImpl(void* apRaw) noexcept
    {
        return *reinterpret_cast<ValueHandleImpl*>(apRaw);
    }

    static inline const ValueHandleImpl& GetValueHandleImpl(const void* apRaw) noexcept
    {
        return *reinterpret_cast<const ValueHandleImpl*>(apRaw);
    }
    
    class ValueHandleImpl
    {
    public:
        ValueHandleImpl()
            : mpAlloc(nullptr), mpValue(nullptr) {}

        rapidjson::Document::AllocatorType* mpAlloc;
        rapidjson::Value* mpValue;


    public:
        static inline ValueHandle ConstructValueHandlePrivate(rapidjson::Value& arValue,
                                                              rapidjson::Document::AllocatorType& arAlloc)
        {
            ValueHandle result;
            GetValueHandleImpl(result.mImpl).mpAlloc = &arAlloc;
            GetValueHandleImpl(result.mImpl).mpValue = &arValue; // Decay to rapidjson::Value
            return result;
        }
    };
    class DocumentImpl
    {
    public:
        explicit DocumentImpl()
            : mDoc(), mRoot(ValueHandleImpl::ConstructValueHandlePrivate(mDoc, mDoc.GetAllocator())) 
        {
        }

        rapidjson::Document mDoc;
        ValueHandle mRoot;
    };

    static inline DocumentImpl& GetDocumentImpl(void* apRaw)
    {
        return *reinterpret_cast<DocumentImpl*>(apRaw);
    }

    Document::Document()
    {
        static_assert(sizeof(Document::mImpl) >= sizeof(DocumentImpl), "Buffer mImpl is to small to store a DocumentImpl object."
                                                                       "This is an internal error and should never happen. Consider increasing the buffer size locally");
        new (mImpl) DocumentImpl(); // Construct inside static buffer, not on heap
    }

    Document::~Document()
    {
        GetDocumentImpl(mImpl).~DocumentImpl();
    }

    std::ostream& Document::Serialize(std::ostream& arOut)
    {
        rapidjson::OStreamWrapper output(arOut);
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(output);
        GetDocumentImpl(mImpl).mDoc.Accept(writer);
        return arOut;
    }

    ValueHandle Document::GetValueHandle()
    {
        return GetDocumentImpl(mImpl).mRoot;
    }
    
    ValueHandle::ValueHandle()
    {
        static_assert(sizeof(ValueHandle::mImpl) >= sizeof(ValueHandleImpl), "Buffer mImpl is to small to store a ValueHandleImpl object."
                                                                 "This is an internal error and should never happen. Consider increasing the buffer size locally");
        new (mImpl) ValueHandleImpl(); // Construct inside static buffer, not on heap
    }

    ValueHandle::ValueHandle(const ValueHandle& arOther)
    {
        new (mImpl) ValueHandleImpl(GetValueHandleImpl(arOther.mImpl));
    }

    ValueHandle::ValueHandle(ValueHandle&& arOther) noexcept
    {
        new (mImpl) ValueHandleImpl(std::move(GetValueHandleImpl(arOther.mImpl)));
    }

    ValueHandle::~ValueHandle()
    {
        GetValueHandleImpl(mImpl).~ValueHandleImpl();
    }

    bool ValueHandle::IsValid() const noexcept
    {
        return GetValueHandleImpl(mImpl).mpValue != nullptr;
    }

    ValueHandle ValueHandle::AddMember(const std::string& arName)
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);

        if (r_handle.mpValue->IsNull())
            r_handle.mpValue->SetObject();

        rapidjson::Value name(arName, *r_handle.mpAlloc);
        r_handle.mpValue->AddMember(std::move(name), rapidjson::Value(), *r_handle.mpAlloc);
        auto it_member = r_handle.mpValue->FindMember(arName);

        return ValueHandleImpl::ConstructValueHandlePrivate(it_member->value, *r_handle.mpAlloc);
    }

    ValueHandle ValueHandle::GetMember(const std::string& arName)
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);
        auto it_member = r_handle.mpValue->FindMember(arName);

        if (it_member != r_handle.mpValue->MemberEnd())
        {
            return ValueHandleImpl::ConstructValueHandlePrivate(it_member->value, *r_handle.mpAlloc);
        }
        else
        {
            return ValueHandle();
        }
    }

    size_t ValueHandle::GetSize() const
    {
        const ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);
        return r_handle.mpValue->Size();
    }

    bool ValueHandle::IsEmpty() const
    {
        return GetSize() == 0;
    }

    ValueHandle ValueHandle::PushBack()
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);

        if (r_handle.mpValue->IsNull())
            r_handle.mpValue->SetArray();

        r_handle.mpValue->PushBack(rapidjson::Value(), *r_handle.mpAlloc);
        return Last();
    }

    ValueHandle ValueHandle::First()
    {
        if (IsEmpty())
            return ValueHandle();

        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);
        auto p_begin = r_handle.mpValue->Begin();
        return ValueHandleImpl::ConstructValueHandlePrivate(*p_begin, *r_handle.mpAlloc);
    }

    ValueHandle ValueHandle::Last()
    {
        if (IsEmpty())
            return ValueHandle();

        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);
        auto p_rbegin = (r_handle.mpValue->End() - 1); // Reverse begin
        return ValueHandleImpl::ConstructValueHandlePrivate(*p_rbegin, *r_handle.mpAlloc);
    }

    ValueHandle ValueHandle::Get(size_t aIndex)
    {
        if (GetSize() >= aIndex)
            return ValueHandle();

        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);
        auto& r_found = (*r_handle.mpValue)[aIndex];
        return ValueHandleImpl::ConstructValueHandlePrivate(r_found, *r_handle.mpAlloc);
    }

    void ValueHandle::SetInt(int aValue)
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);

        if (!r_handle.mpValue->IsNull() && !r_handle.mpValue->IsNumber())
            throw std::invalid_argument("Tried to replace a existing value with an new value of different type");
        r_handle.mpValue->SetInt(aValue);
    }

    void ValueHandle::SetInt(intmax_t aValue)
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);

        if (!r_handle.mpValue->IsNull() && !r_handle.mpValue->IsNumber())
            throw std::invalid_argument("Tried to replace a existing value with an new value of different type");
        r_handle.mpValue->SetInt64(aValue);
    }

    void ValueHandle::SetInt(unsigned aValue)
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);

        if (!r_handle.mpValue->IsNull() && !r_handle.mpValue->IsNumber())
            throw std::invalid_argument("Tried to replace a existing value with an new value of different type");
        r_handle.mpValue->SetUint(aValue);
    }

    void ValueHandle::SetInt(uintmax_t aValue)
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);

        if (!r_handle.mpValue->IsNull() && !r_handle.mpValue->IsNumber())
            throw std::invalid_argument("Tried to replace a existing value with an new value of different type");
        r_handle.mpValue->SetUint64(aValue);
    }

    void ValueHandle::SetFloat(float aValue)
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);

        if (!r_handle.mpValue->IsNull() && !r_handle.mpValue->IsNumber())
            throw std::invalid_argument("Tried to replace a existing value with an new value of different type");
        r_handle.mpValue->SetFloat(aValue);
    }

    void ValueHandle::SetFloat(double aValue)
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);

        if (!r_handle.mpValue->IsNull() && !r_handle.mpValue->IsNumber())
            throw std::invalid_argument("Tried to replace a existing value with an new value of different type");
        r_handle.mpValue->SetDouble(aValue);
    }

    void ValueHandle::SetBool(bool aValue)
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);

        if (!r_handle.mpValue->IsNull() && !r_handle.mpValue->IsBool())
            throw std::invalid_argument("Tried to replace a existing value with an new value of different type");
        r_handle.mpValue->SetBool(aValue);
    }

    void ValueHandle::SetString(const std::string& arValue)
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);

        if (!r_handle.mpValue->IsNull() && !r_handle.mpValue->IsString())
            throw std::invalid_argument("Tried to replace a existing value with an new value of different type");
        r_handle.mpValue->SetString(arValue, *r_handle.mpAlloc);
    }

    void ValueHandle::SetNull()
    {
        ValueHandleImpl& r_handle = GetValueHandleImpl(mImpl);
        r_handle.mpValue->SetNull();
    }
}