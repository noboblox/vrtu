#ifndef STYLE_STYLE_HPP_
#define STYLE_STYLE_HPP_

// The external API is straightforward but is wrapped anyway, to not pollute the headers with OS stuff.
// This is especailly needed on windows, where windows.h defines all kinds of stuff we don't need nor want

#include <iosfwd>

namespace STYLE
{
    std::ostream& reset(std::ostream& arStream);

    std::ostream& grey(std::ostream& arStream);
    std::ostream& red(std::ostream& arStream);
    std::ostream& green(std::ostream& arStream);
    std::ostream& yellow(std::ostream& arStream);
    std::ostream& blue(std::ostream& arStream);
    std::ostream& magenta(std::ostream& arStream);
    std::ostream& cyan(std::ostream& arStream);
    std::ostream& white(std::ostream& arStream);
    std::ostream& bright_grey(std::ostream& arStream);
    std::ostream& bright_red(std::ostream& arStream);
    std::ostream& bright_green(std::ostream& arStream);
    std::ostream& bright_yellow(std::ostream& arStream);
    std::ostream& bright_blue(std::ostream& arStream);
    std::ostream& bright_magenta(std::ostream& arStream);
    std::ostream& bright_cyan(std::ostream& arStream);
    std::ostream& bright_white(std::ostream& arStream);
}

#endif