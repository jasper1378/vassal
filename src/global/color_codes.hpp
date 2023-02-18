#ifndef GLOBAl_COLOR_CODES_HPP
#define GLOBAL_COLOR_CODES_HPP

#include <string>

namespace global
{
    namespace color_codes
    {
        inline constexpr std::string foreground_black{ "\033[30m" };
        inline constexpr std::string foreground_red{ "\033[31m" };
        inline constexpr std::string foreground_green{ "\033[32m" };
        inline constexpr std::string foreground_yellow{ "\033[33m" };
        inline constexpr std::string foreground_blue{ "\033[34m" };
        inline constexpr std::string foreground_magenta{ "\033[35m" };
        inline constexpr std::string foreground_cyan{ "\033[36m" };
        inline constexpr std::string foreground_white{ "\033[37m" };

        inline constexpr std::string background_black{ "\033[40m" };
        inline constexpr std::string background_red{ "\033[41m" };
        inline constexpr std::string background_green{ "\033[42m" };
        inline constexpr std::string background_yellow{ "\033[43m" };
        inline constexpr std::string background_blue{ "\033[44m" };
        inline constexpr std::string background_magenta{ "\033[45m" };
        inline constexpr std::string background_cyan{ "\033[46m" };
        inline constexpr std::string background_white{ "\033[47m" };

        inline constexpr std::string reset{ "\033[0m" };
        inline constexpr std::string bold{ "\033[1m" };
        inline constexpr std::string underline{ "\033[4m" };
        inline constexpr std::string inverse{ "\033[7m" };
        inline constexpr std::string bold_off{ "\033[21m" };
        inline constexpr std::string underline_off{ "\033[24m" };
        inline constexpr std::string inverse_off{ "\033[27m" };
    }
}

#endif
