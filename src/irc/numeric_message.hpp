#ifndef IRC_NUMERIC_MESSAGE_HPP
#define IRC_NUMERIC_MESSAGE_HPP

#include "message.hpp"

#include <iostream>
#include <string>
#include <unordered_map>

namespace irc {
class numeric_message : public message {
public:
  enum class unknown_code_policy {
    RELAXED,
    STRICT,
  };

private:
  int m_code;
  std::string m_code_string;
  bool m_is_error;
  unknown_code_policy m_unknown_code_policy;
  bool m_is_code_known;

private:
  static const std::unordered_map<int, std::string> m_k_code_string_hash_table;

public:
  numeric_message();
  explicit numeric_message(const std::string &raw_message,
                           const unknown_code_policy unknown_code_policy =
                               unknown_code_policy::RELAXED);
  numeric_message(const numeric_message &other);
  numeric_message(numeric_message &&other);

  virtual ~numeric_message() override;

public:
  virtual numeric_message *create_new() const override;
  virtual numeric_message *create_clone() const override;

  virtual type get_type() const override;
  virtual std::string get_keyword() const override;

  int get_code() const;
  std::string get_code_string() const;
  bool get_is_error() const;
  unknown_code_policy get_unknown_code_policy() const;
  bool get_is_code_known() const;

public:
  numeric_message &operator=(const numeric_message &other);
  numeric_message &operator=(numeric_message &&other);

protected:
  virtual void print(std::ostream &out) const override;

private:
  void parse_code(const std::string &raw_message);
};
} // namespace irc

#endif
