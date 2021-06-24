#ifndef IEC104_ERRORUNKNOWNTYPE_HPP_
#define IEC104_ERRORUNKNOWNTYPE_HPP_

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "protocols/iec104/data104.hpp"

class BaseData;
class ByteStream;

namespace IEC104
{
    class ErrorUnknownType : std::exception
    {
    public:
        /*
         * Exception thrown when decoding ASDUs with unknown TypeID
         */
        ErrorUnknownType(const ByteStream& arBuffer, const DataEnum<TypeEnum>& arType);
        
        /// Set a BaseData element responsible for the error
        void SetErrorSource(const BaseData& arData);

        virtual char const* what() const noexcept;

        std::ostream& PrintError(std::ostream& arOutput) const;

    private:
        size_t  mErrorPos;
        uint8_t mErrorValue;
        uint8_t mContextSize;
        uint8_t mContext[5];

        std::string mErrorMsg;
        std::string mErrorSource;

        mutable std::string mWhat;
    };
}

#endif
