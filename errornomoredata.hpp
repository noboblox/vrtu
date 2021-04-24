#ifndef ERRORNOMOREDATA_HPP_
#define ERRORNOMOREDATA_HPP_

#include <stdexcept>

#include <cstdint>
#include <string>
#include <vector>

class ByteStream;

class ErrorNoMoreData : public std::exception
{
public:
    /*
     * Exception for out of bounds access in bytestreams
     */
    ErrorNoMoreData(const ByteStream& arBuffer);

    void PrependLine(std::string&& arMsg);
    void AppendLine(std::string&& arMsg);
    void ClearLines() noexcept;

    virtual char const* what() const;

    /**
     * Print the error message directly to output stream
     * This is needed, because some OSses (windows) do not support color codes inside stringstreams.
     * 
     * This means, calls to what() don't have any coloring on such OSses.
     * On OSses, where ANSI escape chars are used, this should be not a problem.
     */
    std::ostream& PrintError(std::ostream& arOutput) const;

private:
    size_t mBufferSize;
    uint8_t mContextSize;
    uint8_t mContext[5];
    std::vector<std::string> mMsgLines;
    mutable std::string mWhat;
};

#endif

