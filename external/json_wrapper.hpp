#ifndef JSON_JSON_HPP_
#define JSON_JSON_HPP_

#include <stdexcept>
#include <string>
#include <type_traits>

/**
 * This file wraps the external JSON library 
 */

namespace JSON
{
    class DocumentImpl;
    class ValueHandle;
    class ValueHandleImpl;

    class Document
    {
    public:
        /// Construct a JSON document which contains an empty root of value type NULL
        explicit Document(); 
        ~Document();

        /// TODO Parsing strategies ...
        std::ostream& Serialize(std::ostream& arOut);

        // Access the content of root
        ValueHandle GetValueHandle();

    private:
        uint8_t mImpl[120]; // Opague implementation object is stored in place
    };

    class ValueHandle
    {
    public:
        ValueHandle(const ValueHandle& arOther);
        ValueHandle(ValueHandle&& arOther) noexcept;
        

        ~ValueHandle();
        // A value will receive a concrete type on first assignment. 
        // Afterwards it is not allowed to change its type, except it is explicitly set to NULL again

        /// Does the handle point to a place inside the JSON document?
        bool IsValid() const noexcept;

    /// Value type: Object
        ValueHandle AddMember(const std::string& arName);
        ValueHandle GetMember(const std::string& arName);

    /// Value type: Array
        size_t GetSize() const;
        bool IsEmpty() const;

        ValueHandle PushBack();
        ValueHandle First();
        ValueHandle Last();
        ValueHandle Get(size_t aIndex);

    /// Leaf value types

        /// Setter with auto deduction.
        /// This can be done for JSON types int, float, bool, NULL or string.
        /// This is checked at compile time
        /// Type NULL is passed as nullptr. A simple NULL does not work as the check is performed on nullptr_t
        template <typename JsonLeafType>
        void inline Set(const JsonLeafType& arValue)
        {
            static_assert(std::is_arithmetic<JsonLeafType>::value   ||
                          std::is_null_pointer<JsonLeafType>::value ||
                          std::is_same<std::string, JsonLeafType>::value,
                          "Type is not a valid JSON leaf-type");

            //  The requested type has no template-specialization (below). Should not happen, but better safe than sorry
            throw std::invalid_argument("No template-specialization for type. This is a compile-time problem");
        }

        void SetInt(int aValue);
        void SetInt(intmax_t aValue);
        void SetInt(unsigned aValue);
        void SetInt(uintmax_t aValue);

        void SetFloat(float aValue);
        void SetFloat(double aValue);

        void SetBool(bool aValue);
        void SetString(const std::string& arValue);

        void SetNull();

    private:
        friend class DocumentImpl;
        friend class ValueHandleImpl;
        
        ValueHandle();
        uint8_t mImpl[16]; // Opague implementation object is stored in place
    };

    template<> void inline ValueHandle::Set<int8_t>(const int8_t& arValue) { SetInt(arValue); }
    template<> void inline ValueHandle::Set<int16_t>(const int16_t& arValue) { SetInt(arValue); }
    template<> void inline ValueHandle::Set<int32_t>(const int32_t& arValue) { SetInt(arValue); }
    template<> void inline ValueHandle::Set<int64_t>(const int64_t& arValue) { SetInt(arValue); }
    template<> void inline ValueHandle::Set<uint8_t>(const uint8_t& arValue) { SetInt(arValue); }
    template<> void inline ValueHandle::Set<uint16_t>(const uint16_t& arValue) { SetInt(arValue); }
    template<> void inline ValueHandle::Set<uint32_t>(const uint32_t& arValue) { SetInt(arValue); }
    template<> void inline ValueHandle::Set<uint64_t>(const uint64_t& arValue) { SetInt(arValue); }
    template<> void inline ValueHandle::Set<float>(const float& arValue) { SetFloat(arValue); }
    template<> void inline ValueHandle::Set<double>(const double& arValue) { SetFloat(arValue); }
    template<> void inline ValueHandle::Set<bool>(const bool& arValue) { SetBool(arValue); }
    template<> void inline ValueHandle::Set<nullptr_t>(const nullptr_t&) { SetNull(); }
    template<> void inline ValueHandle::Set<std::string>(const std::string& arValue) { SetString(arValue); }
}

#endif

