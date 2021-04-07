#include "external/style.hpp"
#include "external/private/termcolor/termcolor.hpp"

namespace STYLE
{
    std::ostream& reset(std::ostream& arStream)
    {
        return termcolor::reset(arStream);
    }

    std::ostream& grey(std::ostream& arStream)
    {
        return termcolor::grey(arStream);
    }

    std::ostream& red(std::ostream& arStream)
    {
        return termcolor::red(arStream);
    }

    std::ostream& green(std::ostream& arStream)
    {
        return termcolor::green(arStream);
    }

    std::ostream& yellow(std::ostream& arStream)
    {
        return termcolor::yellow(arStream);
    }

    std::ostream& blue(std::ostream& arStream)
    {
        return termcolor::blue(arStream);
    }

    std::ostream& magenta(std::ostream& arStream)
    {
        return termcolor::magenta(arStream);
    }

    std::ostream& cyan(std::ostream& arStream)
    {
        return termcolor::cyan(arStream);
    }

    std::ostream& white(std::ostream& arStream)
    {
        return termcolor::white(arStream);
    }

    std::ostream& bright_grey(std::ostream& arStream)
    {
        return termcolor::bright_grey(arStream);
    }

    std::ostream& bright_red(std::ostream& arStream)
    {
        return termcolor::bright_red(arStream);
    }

    std::ostream& bright_green(std::ostream& arStream)
    {
        return termcolor::bright_green(arStream);
    }

    std::ostream& bright_yellow(std::ostream& arStream)
    {
        return termcolor::bright_yellow(arStream);
    }

    std::ostream& bright_blue(std::ostream& arStream)
    {
        return termcolor::bright_blue(arStream);
    }

    std::ostream& bright_magenta(std::ostream& arStream)
    {
        return termcolor::bright_magenta(arStream);
    }

    std::ostream& bright_cyan(std::ostream& arStream)
    {
        return termcolor::bright_cyan(arStream);
    }

    std::ostream& bright_white(std::ostream& arStream)
    {
        return termcolor::bright_white(arStream);
    }
}