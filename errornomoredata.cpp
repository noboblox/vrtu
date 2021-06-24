#include "errornomoredata.hpp"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "core/bytestream.hpp"
#include "external/style.hpp"

ErrorNoMoreData::ErrorNoMoreData(const ByteStream& arBuffer)
    : std::exception(),
    mBufferSize(arBuffer.BufferSize()),
    mContextSize(0), mContext(),
    mMsgLines({ "[ERROR] End of data reached unexpectedly"})
{
    if (mBufferSize != 0)
    {
        mContextSize = std::min(static_cast<size_t> (std::distance(arBuffer.Begin(), arBuffer.End())),
                                sizeof(mContext));

        std::memcpy(mContext, arBuffer.End() - mContextSize, mContextSize);
    }
}

void ErrorNoMoreData::PrependLine(std::string&& arMsg)
{
    mMsgLines.emplace(mMsgLines.begin(), std::move(arMsg));
}

void ErrorNoMoreData::AppendLine(std::string&& arMsg)
{
    mMsgLines.emplace_back(std::move(arMsg));
}
void ErrorNoMoreData::ClearLines() noexcept
{
    mMsgLines.clear(); 
}

char const* ErrorNoMoreData::what() const noexcept
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

std::ostream& ErrorNoMoreData::PrintError(std::ostream & arOutput) const
{
    // Error message lines
    for (const auto& msg : mMsgLines)
        arOutput << STYLE::red << msg  << "\n";

    static constexpr char DETAIL_INDENT[] = "    ";

    // Buffer position line
    arOutput << STYLE::reset << DETAIL_INDENT;

    for (int i = 0; i < mContextSize + 1; ++i)
        arOutput << " | " << std::setw(4) << (mBufferSize - mContextSize + i);

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

    arOutput << " | " << std::setw(4) << "end" << " |\n";

    // Error indicator line
    arOutput << DETAIL_INDENT;

    for (int i = 0; i < mContextSize; ++i)
        arOutput << "   " << std::setw(4) << " ";

    arOutput << "   " << std::setw(4) << STYLE::red << "^^^" << STYLE::reset << "\n";
    return arOutput;
}