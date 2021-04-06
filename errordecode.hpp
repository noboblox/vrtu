#ifndef ERROR_DECODE_HPP_
#define ERROR_DECODE_HPP_

#include <string>

#include "errornomoredata.hpp"

/*
 * This macro executes "expression" and catches errors emited from ByteStreams
 * caused by an out of bounds read.
 * If an error is catched, it appends the path to "basedata" as "currently decoded element"
 * to the exception and rethrows it.
 */
#define RETHROW_FAIL_AS_DECODE_ERROR(expression, basedata) \
try                                                        \
{                                                          \
    expression;                                            \
}                                                          \
catch (ErrorNoMoreData& error)                             \
{                                                          \
    std::string msg("While decoding element \"");          \
    msg.append(basedata.GetPath());                        \
    msg.append("\"");                                      \
    error.AppendLine(std::move(msg));                      \
    throw;                                                 \
}

#endif
