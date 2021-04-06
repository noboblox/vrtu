#include "errornomoredata.hpp"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

ErrorNoMoreData::ErrorNoMoreData(size_t aBufferSize, const uint8_t* apContextBegin, const uint8_t* apContextEnd)
    : std::exception(),
    mBufferSize(aBufferSize),
    mContextSize(0), mContext(),
    mMsgLines({ "[ERROR] End of data reached unexpectedly"})
{
    if (apContextBegin && apContextEnd &&
        apContextBegin < apContextEnd)
    {
        mContextSize = std::min(std::distance(apContextBegin, apContextEnd),
            static_cast<int> (sizeof(mContext)));

        std::memcpy(mContext, apContextEnd - mContextSize, mContextSize);
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

char const* ErrorNoMoreData::what() const
{
    std::ostringstream result;

    // Error message lines
    for (const auto& msg : mMsgLines)
        result << msg << "\n";

    static constexpr char DETAIL_INDENT[] = "    ";

    // Buffer position line
    result << DETAIL_INDENT;

    for (int i = 0; i < mContextSize + 1; ++i)
        result << " | " << std::setw(4) << (mBufferSize - mContextSize + i);

    result << " |\n";

    // Data line

    result << DETAIL_INDENT;

    /*
    * ios_flags can be a bit stubborn with the hex base includung leading 0 padding. (e.g. 0x00)
    * So let's just show them, that we don't care
    */
    char byte[5];
    for (int i = 0; i < mContextSize; ++i)
    {
        std::snprintf(byte, sizeof(byte), "0x%02X", mContext[i]);
        result << " | " << byte;
    }

    result << " | " << std::setw(4) << "end" << " |\n";

    // Error indicator line
    result << DETAIL_INDENT;

    for (int i = 0; i < mContextSize; ++i)
        result << "   " << std::setw(4) << " ";

    result << "   " << std::setw(4) << "^^^" << "\n";

    mWhat = result.str();
    return mWhat.c_str();
}