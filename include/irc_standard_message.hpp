#ifndef VASSAL_IRC_STANDARD_MESSAGE_HPP
#define VASSAL_IRC_STANDARD_MESSAGE_HPP

#include "irc_message.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace irc {
class standard_message : public message {
private:
  std::string m_command;

public:
  standard_message();
  explicit standard_message(const std::string_view raw_message);
  standard_message(const standard_message &other);
  standard_message(standard_message &&other) noexcept;

  virtual ~standard_message() override;

public:
  virtual standard_message *create_new() const override;
  virtual standard_message *create_clone() const override;

  virtual type get_type() const override;
  virtual std::string get_keyword() const override;

  std::string get_command() const;

public:
  standard_message &operator=(const standard_message &other);
  standard_message &operator=(standard_message &&other) noexcept(
      std::is_nothrow_move_assignable_v<message>
          &&std::is_nothrow_move_assignable_v<std::string>);

protected:
  virtual void print(std::ostream &out) const override;

private:
  void parse_command(const std::string_view raw_message);
};
} // namespace irc

#endif
