#ifndef VASSAL_DATE_TIME_FORMAT_PRINT_HPP
#define VASSAL_DATE_TIME_FORMAT_PRINT_HPP

#include <chrono>
#include <iostream>
#include <string>

namespace vassal {

namespace misc {
std::string to_string(const std::chrono::year &year);
std::string to_string(const std::chrono::month &month);
std::string to_string(const std::chrono::day &day);
std::string to_string(const std::chrono::year_month_day &ymd);

std::ostream &operator<<(std::ostream &out, const std::chrono::year &year);
std::ostream &operator<<(std::ostream &out, const std::chrono::month &month);
std::ostream &operator<<(std::ostream &out, const std::chrono::day &day);
std::ostream &operator<<(std::ostream &out,
                         const std::chrono::year_month_day &ymd);
} // namespace misc

} // namespace vassal
#endif
