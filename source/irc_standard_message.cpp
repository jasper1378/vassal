#include "irc_standard_message.hpp"

#include "irc_message.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

irc::standard_message::standard_message() : message{}, m_command{} {}

irc::standard_message::standard_message(const std::string &raw_message)
    : message{raw_message}, m_command{} {
  parse_command(raw_message);
}

irc::standard_message::standard_message(const standard_message &other)
    : message{other}, m_command{other.m_command} {}

irc::standard_message::standard_message(standard_message &&other) noexcept
    : message{std::move(other)}, m_command{std::move(other.m_command)} {}

irc::standard_message::~standard_message() {}

irc::standard_message *irc::standard_message::create_new() const {
  return new standard_message{};
}

irc::standard_message *irc::standard_message::create_clone() const {
  return new standard_message{*this};
}

irc::message::type irc::standard_message::get_type() const {
  return type::STANDARD;
}

std::string irc::standard_message::get_keyword() const { return m_command; }

std::string irc::standard_message::get_command() const { return m_command; }

irc::standard_message &
irc::standard_message::operator=(const standard_message &other) {
  message::operator=(other);
  m_command = other.m_command;

  return *this;
}

irc::standard_message &
irc::standard_message::operator=(standard_message &&other) noexcept(
    std::is_nothrow_move_assignable_v<message>
        &&std::is_nothrow_move_assignable_v<std::string>) {
  message::operator=(std::move(other));
  m_command = std::move(other.m_command);

  return *this;
}

void irc::standard_message::print(std::ostream &out) const {
  message::print(out);
}

void irc::standard_message::parse_command(const std::string &raw_message) {
  static constexpr std::string::size_type command_pos_word{2};
  static constexpr char delimiter_space{' '};

  std::string::size_type pos_last{std::string::npos};
  std::string::size_type pos_cur{std::string::npos};

  for (size_t i{0}; i < command_pos_word; ++i) {
    pos_last = pos_cur;
    pos_cur = raw_message.find(delimiter_space, (pos_last + 1));
  }

  m_command = raw_message.substr((pos_last + 1), ((pos_cur) - (pos_last + 1)));
}
