// This header wraps configuration and inclusion of the external JSON library (currently rapidjson)

#ifndef JSON_HPP_
#define JSON_HPP_

#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/rapidjson.h"

#include "rapidjson/allocators.h"
#include "rapidjson/document.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"

namespace JSON = rapidjson;
namespace rapidjson
{
    /**
        * @brief Current position inside a JSON document.
        *
        * - The current value can be accessed via GetValue()
        * - The handle stores additional meta data needed to parse the document
        */
    class ValueHandle
    {
    public:
        explicit ValueHandle(Document& arRoot)
            : mrAlloc(arRoot.GetAllocator()),
              mrValue(arRoot) // In rapidjson a document is a value
        {
        }

        ValueHandle(Value& arValue, Document::AllocatorType& arAlloc)
            : mrAlloc(arAlloc), mrValue(arValue)
        {
        }

        Value& GetValue() { return mrValue; }
        Document::AllocatorType& GetAllocator() { return mrAlloc; }

    private:
        Document::AllocatorType& mrAlloc;
        Value& mrValue;
    };

}

#endif


