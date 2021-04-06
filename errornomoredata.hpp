#ifndef ERRORNOMOREDATA_HPP_
#define ERRORNOMOREDATA_HPP_

#include <stdexcept>

#include <cstdint>
#include <string>
#include <vector>

class ErrorNoMoreData : public std::exception
{
public:
    /*
     * Exception for out of bounds access in bytestreams
     * @param aBufferSize Size of the buffer ( == Error location "end")
     * @param apContextBegin Begin of the context (the prevoius bytes)
     * @param apContextEnd First byte after the context (the error location itself)
     */
    ErrorNoMoreData(size_t aBufferSize, const uint8_t* apContextBegin, const uint8_t* apContextEnd);

    void PrependLine(std::string&& arMsg);
    void AppendLine(std::string&& arMsg);
    void ClearLines() noexcept;

    virtual char const* what() const;

private:
    size_t mBufferSize;
    uint8_t mContextSize;
    uint8_t mContext[5];
    std::vector<std::string> mMsgLines;

    mutable std::string mWhat;
};

#endif

