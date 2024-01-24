#include "irc_message.hpp"

#include <cctype>
#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

vassal::irc::message::message() : m_sender_info{}, m_recipient{}, m_body{} {}

vassal::irc::message::message(const std::string_view raw_message)
    : m_sender_info{}, m_recipient{}, m_body{} {
  parse_sender_info(raw_message);
  parse_recipient(raw_message);
  parse_body(raw_message);
}

vassal::irc::message::message(const message &other)
    : m_sender_info{other.m_sender_info}, m_recipient{other.m_recipient},
      m_body{other.m_body} {}

vassal::irc::message::message(message &&other) noexcept
    : m_sender_info{std::move(other.m_sender_info)},
      m_recipient{std::move(other.m_recipient)},
      m_body{std::move(other.m_body)} {}

vassal::irc::message::~message() {}

vassal::irc::message::sender_info
vassal::irc::message::get_sender_info() const {
  return m_sender_info;
}

std::string vassal::irc::message::get_recipient() const { return m_recipient; }

std::string vassal::irc::message::get_body() const { return m_body; }

vassal::irc::message::type
vassal::irc::message::check_type(const std::string_view raw_message) {
  static constexpr std::string::size_type type_pos_word{2};
  static constexpr char delimiter_space{' '};

  std::string::size_type pos_last{std::string::npos};
  std::string::size_type pos_cur{std::string::npos};

  for (size_t i{0}; i < type_pos_word; ++i) {
    pos_last = pos_cur;
    pos_cur = raw_message.find(delimiter_space, (pos_last + 1));
  }

  if (std::isdigit(raw_message[pos_last + 1]) == true) {
    return type::NUMERIC;
  } else {
    return type::STANDARD;
  }
}

vassal::irc::message &vassal::irc::message::operator=(const message &other) {
  m_sender_info = other.m_sender_info;
  m_recipient = other.m_recipient;
  m_body = other.m_body;

  return *this;
}

vassal::irc::message &vassal::irc::message::operator=(message &&other) noexcept(
    std::is_nothrow_move_assignable_v<std::string>) {
  m_sender_info = std::move(other.m_sender_info);
  m_recipient = std::move(other.m_recipient);
  m_body = std::move(other.m_body);

  return *this;
}

void vassal::irc::message::print(std::ostream &out) const {
  out << ':';
  if (m_sender_info.sender_nick != "") {
    out << m_sender_info.sender_nick << '!';
  }
  if (m_sender_info.sender_user != "") {
    out << m_sender_info.sender_user << '@';
  }
  out << m_sender_info.sender_host;

  out << ' ' << get_keyword();

  out << ' ' << m_recipient;

  out << ' ' << m_body;
}

void vassal::irc::message::parse_sender_info(
    const std::string_view raw_message) {
  static constexpr char delimiter_colon{':'};
  static constexpr char delimiter_exclamation_mark{'!'};
  static constexpr char delimiter_at_sign{'@'};
  static constexpr char delimiter_space{' '};

  const std::string_view sender_info_substr{
      raw_message.substr(0, raw_message.find(delimiter_space))};

  const std::string::size_type pos_colon{
      sender_info_substr.find(delimiter_colon)};
  const std::string::size_type pos_exclamation_mark{
      sender_info_substr.find(delimiter_exclamation_mark)};
  const std::string::size_type pos_at_sign{
      sender_info_substr.find(delimiter_at_sign)};

  if ((pos_exclamation_mark == std::string::npos) &&
      (pos_at_sign == std::string::npos)) {
    m_sender_info.sender_nick = "";
    m_sender_info.sender_user = "";
    m_sender_info.sender_host = sender_info_substr.substr((pos_colon + 1));
  } else if (!(pos_exclamation_mark == std::string::npos) !=
             !(pos_at_sign == std::string::npos)) {
    throw std::runtime_error{"if message sender contains one of [nick,user], "
                             "it should contain both"};
  } else {
    m_sender_info.sender_nick = sender_info_substr.substr(
        (pos_colon + 1), (pos_exclamation_mark - (pos_colon + 1)));
    m_sender_info.sender_user = sender_info_substr.substr(
        (pos_exclamation_mark + 1), (pos_at_sign - (pos_exclamation_mark + 1)));
    m_sender_info.sender_host = sender_info_substr.substr((pos_at_sign + 1));
  }
}

void vassal::irc::message::parse_recipient(const std::string_view raw_message) {
  static constexpr std::string::size_type recipient_pos_word{3};
  static constexpr char delimiter_space{' '};

  std::string::size_type pos_last{std::string::npos};
  std::string::size_type pos_cur{std::string::npos};

  for (size_t i{0}; i < recipient_pos_word; ++i) {
    pos_last = pos_cur;
    pos_cur = raw_message.find(delimiter_space, (pos_last + 1));
  }

  m_recipient =
      raw_message.substr((pos_last + 1), ((pos_cur) - (pos_last + 1)));
}

void vassal::irc::message::parse_body(const std::string_view raw_message) {
  static constexpr std::string::size_type body_pos_word{4};
  static constexpr char delimiter_space{' '};

  std::string::size_type pos_last{std::string::npos};
  std::string::size_type pos_cur{std::string::npos};

  for (size_t i{0}; i < body_pos_word; ++i) {
    pos_last = pos_cur;
    pos_cur = raw_message.find(delimiter_space, (pos_last + 1));
  }

  m_body = raw_message.substr((pos_last + 1));
}

std::ostream &vassal::irc::operator<<(std::ostream &out, const message &m) {
  m.print(out);
  return out;
}
