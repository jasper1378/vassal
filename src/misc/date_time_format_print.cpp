#include "date_time_format_print.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

std::string misc::to_string(const std::chrono::year& year)
{
    std::string year_str{ std::to_string(static_cast<int>(year)) };
    static constexpr int year_digits{ 4 };
    static constexpr char year_pad{ '0' };
    year_str.insert(0, (year_digits - year_str.size()), year_pad);

    if (!(year.ok()))
    {
        throw std::runtime_error{ year_str + " is not a valid year" };
    }

    return year_str;
}

std::string misc::to_string(const std::chrono::month& month)
{
    std::string month_str{ std::to_string(static_cast<unsigned int>(month)) };
    static constexpr int month_digits{ 2 };
    static constexpr char month_pad{ '0' };
    month_str.insert(0, (month_digits - month_str.size()), month_pad);

    if (!(month.ok()))
    {
        throw std::runtime_error{ month_str + " is not a valid month" };
    }

    return month_str;
}

std::string misc::to_string(const std::chrono::day& day)
{
    std::string day_str{ std::to_string(static_cast<unsigned int>(day)) };
    static constexpr int day_digits{ 2 };
    static constexpr char day_pad{ '0' };
    day_str.insert(0, (day_digits - day_str.size()), day_pad);

    if (!(day.ok()))
    {
        throw std::runtime_error{ day_str + " is not a valid day" };
    }

    return day_str;
}

std::string misc::to_string(const std::chrono::year_month_day& ymd)
{
    static constexpr char separator{ '-' };
    std::string ymd_str{ to_string(ymd.year()) + separator + to_string(ymd.month()) + separator + to_string(ymd.day()) };

    if (!(ymd.ok()))
    {
        throw std::runtime_error{ ymd_str + " is not a valid date" };
    }

    return ymd_str;
}

std::ostream& misc::operator<<(std::ostream& out, const std::chrono::year& year)
{
    out << to_string(year);
    return out;
}

std::ostream& misc::operator<<(std::ostream& out, const std::chrono::month& month)
{
    out << to_string(month);
    return out;
}

std::ostream& misc::operator<<(std::ostream& out, const std::chrono::day& day)
{
    out << to_string(day);
    return out;
}

std::ostream& misc::operator<<(std::ostream& out, const std::chrono::year_month_day& ymd)
{
    out << to_string(ymd);
    return out;

    if (!(ymd.ok()))
    {
        out << " is not a valid date";
    }

    return out;
}
