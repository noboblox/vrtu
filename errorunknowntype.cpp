#include "errorunknowntype.hpp"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "core/bytestream.hpp"
#include "external/style.hpp"

namespace IEC104
{
    ErrorUnknownType::ErrorUnknownType(const ByteStream& arBuffer, const DataEnum<TypeEnum>& arType)
        : std::exception(),
        mErrorPos(std::distance(arBuffer.Begin(), arBuffer.Iterator())),
        mErrorValue(arBuffer.BufferSize() ? *arBuffer.Iterator() : 0),
        mContextSize(0), mContext(),
        mErrorMsg(), mErrorSource(arType.GetPath())
    {
        if (arBuffer.BufferSize() != 0)
        {
            mContextSize = std::min(mErrorPos, sizeof(mContext));
            std::memcpy(mContext, arBuffer.Iterator() - mContextSize, mContextSize);
        }

        std::stringstream msg;
        msg << "[ERROR] Type id not supported: \"" << (*arType).GetLabel(true) << " (" << static_cast<int>((*arType).GetValue()) << ")\"";
        mErrorMsg = msg.str();
    }

    void ErrorUnknownType::SetErrorSource(const BaseData& arData)
    {
        mErrorSource = arData.GetPath();
    }

    char const* ErrorUnknownType::what() const noexcept
    {
        try
        {
            std::stringstream result;
            PrintError(result);
            mWhat = result.str();
            return mWhat.c_str();
        }
        catch (...) 
        {
            return "Error during error handling";
        }
    }

    std::ostream& ErrorUnknownType::PrintError(std::ostream& arOutput) const
    {
        // Error message lines
        arOutput << STYLE::red << mErrorMsg << "\n";

        if (!mErrorSource.empty())
            arOutput << "While decoding element \"" << mErrorSource << "\"\n";

        static constexpr char DETAIL_INDENT[] = "    ";

        // Buffer position line
        arOutput << STYLE::reset << DETAIL_INDENT;

        for (int i = 0; i < mContextSize + 1; ++i)
            arOutput << " | " << std::setw(4) << (mErrorPos - mContextSize + i);

        arOutput << " |\n";

        // Data line

        arOutput << DETAIL_INDENT;

        /*
        * ios_flags can be a bit stubborn with the hex base includung leading 0 padding. (e.g. 0x00)
        * So let's just show them, that we don't care
        */
        char byte[5];
        for (int i = 0; i < mContextSize; ++i)
        {
            std::snprintf(byte, sizeof(byte), "0x%02X", mContext[i]);
            arOutput << " | " << byte;
        }

        std::snprintf(byte, sizeof(byte), "0x%02X", mErrorValue);
        arOutput << " | " << std::setw(4) << byte << " |\n";

        // Error indicator line
        arOutput << DETAIL_INDENT;

        for (int i = 0; i < mContextSize; ++i)
            arOutput << "   " << std::setw(4) << " ";

        arOutput << "   " << std::setw(4) << STYLE::red << "^^^^" << STYLE::reset << "\n";
        return arOutput;
    }
}