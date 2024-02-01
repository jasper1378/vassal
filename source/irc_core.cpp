#include "irc_core.hpp"

#include "irc_message.hpp"
#include "irc_numeric_message.hpp"
#include "irc_standard_message.hpp"

#include "color_codes.hpp"
#include "output_mutex.hpp"

#include "liblocket/liblocket.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

vassal::irc::core::core(const std::string &server_address, uint16_t port_num,
                        const std::string &nick, const std::string &realname,
                        liblocket::inet_socket_addr::ip_version
                            ip_version /*= inet_socket_addr::ip_version::ipv4*/,
                        const std::string &server_password /*= ""*/)
    : m_server_address{(
          (ip_version == liblocket::inet_socket_addr::ip_version::ipv4)
              ? (static_cast<liblocket::inet_socket_addr *>(
                    new liblocket::inet4_socket_addr{server_address, port_num}))
              : (static_cast<liblocket::inet_socket_addr *>(
                    new liblocket::inet6_socket_addr{server_address,
                                                     port_num})))},
      m_nick{nick},
      m_socket{liblocket::socket::dummy_type_connect{}, m_server_address},
      m_unread_responses{}, m_new_unread_response{false}, m_listener_thread{},
      m_listener_thread_kill_yourself{false} {
  m_listener_thread = std::thread{&irc::core::listen, this};

  if (server_password != "") {
    send_message_pass(server_password);
  }
  send_message_nick(nick);
  send_message_user(nick, realname);
}

vassal::irc::core::core(core &&other)
    : m_server_address{other.m_server_address}, m_nick{std::move(other.m_nick)},
      m_socket{std::move(other.m_socket)}, m_unread_responses{},
      m_new_unread_response{std::move(other.m_new_unread_response.load())},
      m_listener_thread{std::move(other.m_listener_thread)},
      m_listener_thread_kill_yourself{
          std::move(other.m_listener_thread_kill_yourself.load())} {
  other.m_server_address = nullptr;

  m_unread_responses.resize(other.m_unread_responses.size(), nullptr);
  for (size_t i{0}; i < other.m_unread_responses.size(); ++i) {
    m_unread_responses[i] = other.m_unread_responses[i];
    other.m_unread_responses[i] = nullptr;
  }
}

vassal::irc::core::~core() {
  m_listener_thread_kill_yourself = true;
  m_listener_thread.join();

  delete m_server_address;
  m_server_address = nullptr;

  for (size_t i{0}; i < m_unread_responses.size(); ++i) {
    delete m_unread_responses[i];
    m_unread_responses[i] = nullptr;
  }
}

vassal::irc::message *vassal::irc::core::recv_response() {
  while (true) {
    std::unique_lock<std::mutex> m_unread_responses_mutex_lock{
        m_unread_responses_mutex};
    m_unread_responses_cv.wait(m_unread_responses_mutex_lock, [this] {
      return m_new_unread_response == true;
    });

    if (m_unread_responses.size() == 0) {
      m_new_unread_response = false;
      continue;
    }

    message *temp{m_unread_responses.front()};
    m_unread_responses.front() = nullptr;
    m_unread_responses.pop_front();

    return temp;
  }
}

void vassal::irc::core::send_message_pass(const std::string &password) {
  std::string message{std::string{"PASS "} + password};
  send_message(message);
}

void vassal::irc::core::send_message_nick(const std::string &nickname) {
  std::string message{std::string{"NICK "} + nickname};
  send_message(message);
}

void vassal::irc::core::send_message_user(
    const std::string &username, const std::string &realname,
    const std::pair<user_mode, user_mode>
        modes /*= {user_mode::max, user_mode::max}*/) {
  std::string message{std::string{"USER "} + username + " "};

  static constexpr uint8_t bitmask_user_mode_i{0b1000};
  static constexpr uint8_t bitmask_user_mode_w{0b0100};
  uint8_t modes_bitmask{0};

  auto set_bitmask{[=, &modes_bitmask](user_mode mode) -> void {
    switch (mode) {
    case user_mode::max:
      break;
    case user_mode::i:
      modes_bitmask |= bitmask_user_mode_i;
      break;
    case user_mode::w:
      modes_bitmask |= bitmask_user_mode_w;
      break;
    default:
      throw std::runtime_error{
          "only user modes 'i' and 'w' may be set through USER command"};
      break;
    }
  }};

  set_bitmask(modes.first);
  set_bitmask(modes.second);

  message.append(
      std::string{std::to_string(modes_bitmask) + " * :" + realname});

  send_message(message);
}

void vassal::irc::core::send_message_oper(const std::string &name,
                                          const std::string &password) {
  std::string message{std::string{"OPER "} + name + " " + password};
  send_message(message);
}

void vassal::irc::core::send_message_user_mode(
    const std::string &nickname, const user_mode mode /*= user_mode::max*/,
    const mode_operation operation /*= mode_operation::max*/) {
  if ((mode != user_mode::max) && (operation != mode_operation::max)) {
    switch (mode) {
    case user_mode::a:
      throw std::runtime_error{
          "user mode '[+,-][a]' can only be set through AWAY command"};
      break;
    case user_mode::o:
      [[fallthrough]];
    case user_mode::O:
      switch (operation) {
      case mode_operation::add:
        throw std::runtime_error{
            "user mode '[+][o,O]' can only be set through OPER command"};
        break;
      default:
        break;
      }
      break;
    case user_mode::r:
      switch (operation) {
      case mode_operation::remove:
        throw std::runtime_error{"user mode '[-][r]' can not be set"};
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }

    std::string message{std::string{"MODE "} + nickname + " " +
                        m_k_mode_operation_lut[static_cast<size_t>(operation)] +
                        m_k_user_mode_lut[static_cast<size_t>(mode)]};
    send_message(message);
  } else if ((mode == user_mode::max) && (operation == mode_operation::max)) {
    std::string message{std::string{"MODE "} + nickname};
    send_message(message);
  } else if ((mode == user_mode::max) && (operation != mode_operation::max)) {
    throw std::runtime_error{"mode is not specified"};
  } else if ((mode != user_mode::max) && (operation == mode_operation::max)) {
    throw std::runtime_error{"operation is not specified"};
  } else {
    throw std::runtime_error{
        "this should be impossible... someone fucked up their boolean logic"};
  }
}

void vassal::irc::core::send_message_quit(const std::string &quit_message) {
  std::string message{std::string{"QUIT :"} + quit_message};
  send_message(message);
}

void vassal::irc::core::send_message_squit(const std::string &server,
                                           const std::string &comment) {
  std::string message{std::string{"SQUIT "} + server + " :" + comment};
  send_message(message);
}

void vassal::irc::core::send_message_join(const std::string &channel,
                                          const std::string &key /*= ""*/) {
  std::string message{std::string{"JOIN "} + channel};

  if (key != "") {
    message.append(" ");
    message.append(key);
  }

  send_message(message);
}

void vassal::irc::core::send_message_join(
    const std::vector<std::pair<std::string, std::string>> &channels_and_keys) {
  std::string message{"JOIN "};

  for (size_t i{0}; i < channels_and_keys.size(); ++i) {
    if ((i > 0) && (channels_and_keys[i].first != "")) {
      message.append(",");
    }
    message.append(channels_and_keys[i].first);
  }

  message.append(" ");

  for (size_t i{0}; i < channels_and_keys.size(); ++i) {
    if ((i > 0) && (channels_and_keys[i].second != "")) {
      message.append(",");
    }
    message.append(channels_and_keys[i].second);
  }

  send_message(message);
}

void vassal::irc::core::send_message_part(
    const std::string &channel, const std::string &part_message /*= ""*/) {
  std::string message{std::string{"PART "} + channel};

  if (part_message != "") {
    message.append(std::string{" :"} + part_message);
  }

  send_message(message);
}

void vassal::irc::core::send_message_part(
    const std::vector<std::string> &channels,
    const std::string &part_message /*= ""*/) {
  std::string message{"PART "};

  for (size_t i{0}; i < channels.size(); ++i) {
    if ((i > 0) && (channels[i] != "")) {
      message.append(",");
    }
    message.append(channels[i]);
  }

  if (part_message != "") {
    message.append(std::string{" :"} + part_message);
  }

  send_message(message);
}

void vassal::irc::core::send_message_part_all() {
  std::string message{"JOIN 0"};
  send_message(message);
}

void vassal::irc::core::send_message_channel_mode(
    const std::string &channel, const channel_mode mode,
    const mode_operation operation /*= mode_operation::max*/,
    const std::string &options /*= ""*/) {
  std::string message{std::string{"MODE "} + channel};

  if (operation != mode_operation::max) {
    message.append(std::string{" "} +
                   m_k_mode_operation_lut[static_cast<size_t>(operation)]);
  }

  message.append(std::string{m_k_channel_mode_lut[static_cast<size_t>(mode)]});

  if (options != "") {
    message.append(std::string{" "} + options);
  }

  send_message(message);
}

void vassal::irc::core::send_message_topic(const std::string &channel,
                                           const std::string &topic /*= ""*/) {
  std::string message{std::string{"TOPIC "} + channel};

  if (topic != "") {
    message.append(std::string{" :" + topic});
  }

  send_message(message);
}

void vassal::irc::core::send_message_names(const std::string &channel /*= ""*/,
                                           const std::string &target /*= ""*/) {
  std::string message{"NAMES"};

  if (channel != "") {
    message.append(std::string{" "} + channel);

    if (target != "") {
      message.append(std::string{" "} + target);
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_names(
    const std::vector<std::string> &channels /*= {}*/,
    const std::string &target /*= ""*/) {
  std::string message{"NAMES"};

  if (channels.size() != 0) {
    message.append(" ");

    for (size_t i{0}; i < channels.size(); ++i) {
      if ((i > 0) && (channels[i] != "")) {
        message.append(",");
      }

      message.append(channels[i]);
    }

    if (target != "") {
      message.append(std::string{" "} + target);
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_list(const std::string &channel /*= ""*/,
                                          const std::string &target /*= ""*/) {
  std::string message{"LIST"};

  if (channel != "") {
    message.append(std::string{" "} + channel);

    if (target != "") {
      message.append(std::string{" "} + target);
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_list(
    const std::vector<std::string> &channels /*= {}*/,
    const std::string &target /*= ""*/) {
  std::string message{"LIST"};

  if (channels.size() != 0) {
    message.append(" ");

    for (size_t i{0}; i < channels.size(); ++i) {
      if ((i > 0) && (channels[i] != "")) {
        message.append(",");
      }

      message.append(channels[i]);
    }

    if (target != "") {
      message.append(std::string{" "} + target);
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_invite(const std::string &nickname,
                                            const std::string &channel) {
  std::string message{"INVITE " + nickname + " " + channel};
  send_message(message);
}

void vassal::irc::core::send_message_kick(const std::string &channel,
                                          const std::string &user,
                                          const std::string &comment /*= ""*/) {
  std::string message{"KICK " + channel + " " + user};

  if (comment != "") {
    message.append(std::string{" :" + comment});
  }

  send_message(message);
}

void vassal::irc::core::send_message_kick(const std::string &channel,
                                          const std::vector<std::string> &users,
                                          const std::string &comment /*= ""*/) {
  std::string message{"KICK " + channel + " "};

  for (size_t i{0}; i < users.size(); ++i) {
    if ((i > 0) && (users[i] != "")) {
      message.append(",");
    }
    message.append(users[i]);
  }

  if (comment != "") {
    message.append(std::string{" :" + comment});
  }

  send_message(message);
}

void vassal::irc::core::send_message_kick(
    const std::vector<std::string> &channels,
    const std::vector<std::string> &users,
    const std::string &comment /*= ""*/) {
  if (channels.size() != users.size()) {
    throw std::runtime_error{"the number of channel parameters and user "
                             "parameters must be the same"};
  }

  std::string message{"KICK "};

  for (size_t i{0}; i < channels.size(); ++i) {
    if ((i > 0) && (channels[i] != "")) {
      message.append(",");
    }
    message.append(users[i]);
  }

  message.append(" ");

  for (size_t i{0}; i << users.size(); ++i) {
    if ((i > 0) && (users[i] != "")) {
      message.append(",");
    }
    message.append(users[i]);
  }

  if (comment != "") {
    message.append(std::string{" :" + comment});
  }

  send_message(message);
}

void vassal::irc::core::send_message_privmsg(
    const std::string &receiver, const std::string &text_to_be_sent) {
  std::string message{std::string{"PRIVMSG "} + receiver + " :" +
                      text_to_be_sent};
  send_message(message);
}

void vassal::irc::core::send_message_notice(
    const std::string &receiver, const std::string &text_to_be_sent) {
  std::string message{std::string{"PRIVMSG "} + receiver + " :" +
                      text_to_be_sent};
  send_message(message);
}

void vassal::irc::core::send_message_motd(const std::string &target /*= ""*/) {
  std::string message{"MOTD"};

  if (target != "") {
    message.append(std::string{" " + target});
  }

  send_message(message);
}

void vassal::irc::core::send_message_lusers(
    const std::string &mask /*= ""*/, const std::string &target /*= ""*/) {
  std::string message{"LUSERS"};

  if (mask != "") {
    message.append(std::string{" " + mask});

    if (target != "") {
      message.append(std::string{" " + target});
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_version(
    const std::string &target /*= ""*/) {
  std::string message{"VERSION"};

  if (target != "") {
    message.append(std::string{" " + target});
  }

  send_message(message);
}

void vassal::irc::core::send_message_stats(
    stats_query query /*= stats_query::max*/,
    const std::string &target /*= ""*/) {
  std::string message{"STATS"};

  if (query != stats_query::max) {
    message.append(std::string{" "} +
                   m_k_stats_query_lut[static_cast<size_t>(query)]);

    if (target != "") {
      message.append(std::string{" " + target});
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_links(
    const std::string &remote_server /*= ""*/,
    const std::string &server_mask /*= ""*/) {
  std::string message{"LINKS"};

  if (server_mask != "") {
    if (remote_server != "") {
      message.append({" " + remote_server});
    }

    message.append(std::string{" " + server_mask});
  }

  send_message(message);
}

void vassal::irc::core::send_message_time(const std::string &target /*= ""*/) {
  std::string message{"TIME"};

  if (target != "") {
    message.append(std::string{" " + target});
  }

  send_message(message);
}

void vassal::irc::core::send_message_connect(
    const std::string &target_server, const std::string &port,
    const std::string &remote_server /*= ""*/) {
  std::string message{std::string{"CONNECT "} + target_server + " " + port};

  if (remote_server != "") {
    message.append(std::string{" " + remote_server});
  }

  send_message(message);
}

void vassal::irc::core::send_message_trace(const std::string &target /*= ""*/) {
  std::string message{"TRACE"};

  if (target != "") {
    message.append(std::string{" " + target});
  }

  send_message(message);
}

void vassal::irc::core::send_message_admin(const std::string &target /*= ""*/) {
  std::string message{"ADMIN"};

  if (target != "") {
    message.append(std::string{" " + target});
  }

  send_message(message);
}

void vassal::irc::core::send_message_info(const std::string &target /*= ""*/) {
  std::string message{"INFO"};

  if (target != "") {
    message.append(std::string{" " + target});
  }

  send_message(message);
}

void vassal::irc::core::send_message_servlist(
    const std::string &mask /*= ""*/, const std::string &type /*= ""*/) {
  std::string message{"SERVLIST"};

  if (mask != "") {
    message.append(std::string{" " + mask});

    if (type != "") {
      message.append(std::string{" " + type});
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_squery(const std::string &service_name,
                                            const std::string &text) {
  std::string message{std::string{"SQUERY "} + service_name + " " + text};
  send_message(message);
}

void vassal::irc::core::send_message_who(const std::string &mask /*= ""*/,
                                         bool only_opers /*= false*/) {
  std::string message{"WHO"};

  if (mask != "") {
    message.append(std::string{" " + mask});

    if (only_opers == true) {
      message.append(" o");
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_whois(const std::string &mask,
                                           const std::string &target /*= ""*/) {
  std::string message{"WHOIS "};

  if (target != "") {
    message.append(std::string{target + " "});
  }

  message.append(mask);

  send_message(message);
}

void vassal::irc::core::send_message_whois(
    const std::vector<std::string> &masks, const std::string &target /*= ""*/) {
  std::string message{"WHOIS "};

  if (target != "") {
    message.append(std::string{target + " "});
  }

  for (size_t i{0}; i < masks.size(); ++i) {
    if ((i > 0) && (masks[i] != "")) {
      message.append(",");
    }

    message.append(masks[i]);
  }

  send_message(message);
}

void vassal::irc::core::send_message_whowas(
    const std::string &nickname, const int count /*= -1*/,
    const std::string &target /*= ""*/) {
  std::string message{"WHOWAS " + nickname};

  if (count != -1) {
    message.append(std::string{" " + std::to_string(count)});

    if (target != "") {
      message.append(std::string{" " + target});
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_whowas(
    const std::vector<std::string> &nicknames, const int count /*= -1*/,
    const std::string &target /*= ""*/) {
  std::string message{"WHOWAS "};

  for (size_t i{0}; i < nicknames.size(); ++i) {
    if ((i > 0) && (nicknames[i] != "")) {
      message.append(",");
    }
    message.append(nicknames[i]);
  }

  if (count != -1) {
    message.append(std::string{" " + std::to_string(count)});

    if (target != "") {
      message.append(std::string{" " + target});
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_kill(const std::string &nickname,
                                          const std::string &comment) {
  std::string message{std::string{"KILL "} + nickname + " :" + comment};
  send_message(message);
}

bool vassal::irc::core::send_message_pong(
    const std::string &possible_ping_message) {
  static const std::string ping{"PING"};
  static const std::string pong{"PONG"};

  std::string::size_type pos{possible_ping_message.find(ping)};

  if (pos != std::string::npos) {
    if (possible_ping_message.find(std::string{ping + " :"}) !=
        std::string::npos) {
      std::string pong_response{pong};
      pong_response.append(possible_ping_message.substr(pos + ping.size()));

      send_message(pong_response);

      return true;
    }
  }

  return false;
}

void vassal::irc::core::send_message_away(const std::string &text /*= ""*/) {
  std::string message{"AWAY"};

  if (text != "") {
    message.append(std::string{" :" + text});
  }

  send_message(message);
}

void vassal::irc::core::send_message_rehash() {
  std::string message{"REHASH"};
  send_message(message);
}

void vassal::irc::core::send_message_die() {
  std::string message{"DIE"};
  send_message(message);
}

void vassal::irc::core::send_message_restart() {
  std::string message{"RESTART"};
  send_message(message);
}

void vassal::irc::core::send_message_summon(
    const std::string &user, const std::string &target /*= ""*/,
    const std::string &channel /*= ""*/) {
  std::string message{std::string{"MESSAGE "} + user};

  if (target != "") {
    message.append(std::string{" " + target});

    if (channel != "") {
      message.append(std::string{" " + channel});
    }
  }

  send_message(message);
}

void vassal::irc::core::send_message_users(const std::string &target /*= ""*/) {
  std::string message{"USERS"};

  if (target != "") {
    message.append(std::string{" " + target});
  }

  send_message(message);
}

void vassal::irc::core::send_message_wallops(const std::string &text) {
  std::string message{"WALLOPS :" + text};
  send_message(message);
}

void vassal::irc::core::send_message_userhost(const std::string &nickname) {
  std::string message{std::string{"USERHOST "} + nickname};
  send_message(message);
}

void vassal::irc::core::send_message_userhost(
    const std::vector<std::string> &nicknames) {
  static constexpr size_t max_nick_list_size{5};

  if (nicknames.size() > max_nick_list_size) {
    throw std::runtime_error{
        "USERHOST can only accept a list of up to 5 nicknames"};
  }

  std::string message{"USERHOST"};

  for (size_t i{0}; i < nicknames.size(); ++i) {
    message.append(std::string{" " + nicknames[i]});
  }

  send_message(message);
}

void vassal::irc::core::send_message_ison(const std::string &nickname) {
  std::string message{std::string{"ISON "} + nickname};
  send_message(message);
}

void vassal::irc::core::send_message_ison(
    const std::vector<std::string> &nicknames) {
  std::string message{"ISON"};

  for (size_t i{0}; i < nicknames.size(); ++i) {
    message.append(std::string{" " + nicknames[i]});
  }

  send_message(message);
}

vassal::irc::core &vassal::irc::core::operator=(core &&other) {
  if (this == &other) {
    return *this;
  }

  delete m_server_address;
  m_server_address = nullptr;
  m_server_address = other.m_server_address;
  other.m_server_address = nullptr;

  m_nick = std::move(other.m_nick);
  m_socket = std::move(other.m_socket);

  for (size_t i{0}; i < m_unread_responses.size(); ++i) {
    delete m_unread_responses[i];
    m_unread_responses[i] = nullptr;
  }
  m_unread_responses.resize(other.m_unread_responses.size(), nullptr);
  for (size_t i{0}; i < m_unread_responses.size(); ++i) {
    m_unread_responses[i] = other.m_unread_responses[i];
    other.m_unread_responses[i] = nullptr;
    ;
  }

  m_new_unread_response = std::move(other.m_new_unread_response.load());
  m_listener_thread = std::move(other.m_listener_thread);
  m_listener_thread_kill_yourself =
      std::move(other.m_listener_thread_kill_yourself.load());

  return *this;
}

void vassal::irc::core::listen() {
  std::string message_fragment{""};

  while (m_listener_thread_kill_yourself == false) {
    std::string received_message{m_socket.recv()};
    if (message_fragment != "") {
      received_message.insert(0, message_fragment);
    }

    std::pair<std::deque<std::string>, std::string> new_messages_raw{
        split_messages(received_message)};
    message_fragment = new_messages_raw.second;

#ifdef DEBUG
    {
      for (size_t i{0}; i < new_messages_raw.first.size(); ++i) {
        std::cout << color_codes::foreground_yellow
                  << "[DEBUG]: [RECEIVED MESSAGE]: "
                  << new_messages_raw.first[i] << color_codes::reset << '\n';
      }
    }
#endif // DEBUG

    std::vector<size_t> messages_to_erase{};
    for (size_t i{0}; i < new_messages_raw.first.size(); ++i) {
      if (send_message_pong(new_messages_raw.first[i]) == true) {
        messages_to_erase.push_back(i);
      }
    }
    for (size_t i{0}; i < messages_to_erase.size(); ++i) {
      new_messages_raw.first.erase(new_messages_raw.first.begin() +
                                   messages_to_erase[i]);
    }

    std::deque<message *> new_messages_parsed{};
    new_messages_parsed.resize(new_messages_raw.first.size(), nullptr);

    for (size_t i{0}; i < new_messages_parsed.size(); ++i) {
      switch (message::check_type(new_messages_raw.first[i])) {
      case message::type::standard:
        new_messages_parsed[i] =
            new standard_message{new_messages_raw.first[i]};
        break;
      case message::type::numeric:
        new_messages_parsed[i] = new numeric_message{new_messages_raw.first[i]};
        break;
      }
    }

    {
      std::unique_lock<std::mutex> m_unread_responses_mutex_lock{
          m_unread_responses_mutex};

      m_unread_responses.insert(m_unread_responses.end(),
                                new_messages_parsed.begin(),
                                new_messages_parsed.end());

      for (size_t i{0}; i < new_messages_parsed.size(); ++i) {
        new_messages_parsed[i] = nullptr;
      }

      m_new_unread_response = true;
    }

    m_unread_responses_cv.notify_all();
  }
}

void vassal::irc::core::send_message(const std::string &message) {
  if (message.size() > m_k_max_message_length) {
    throw std::runtime_error{"message is too long (" +
                             std::to_string(message.size()) + " chars)"};
  }

  std::unique_lock<std::mutex> socket_mutex_write_lock{m_socket_mutex_write};

#ifdef DEBUG
  {
    std::unique_lock<std::mutex> output_mutex_lock{output_mutex::output_mutex};
    std::cerr << color_codes::foreground_red
              << "[DEBUG]: [SENDING MESSAGE]: " << message << color_codes::reset
              << '\n';
  }
#endif // DEBUG
  m_socket.send(std::string{message + m_k_delimiter});
}

std::pair<std::deque<std::string>, std::string>
vassal::irc::core::split_messages(const std::string &messages_combined) {
  std::string message_fragment{""};

  if (messages_combined.ends_with(m_k_delimiter) == false) {
    message_fragment = messages_combined.substr(
        (messages_combined.rfind(m_k_delimiter) + m_k_delimiter.size()),
        std::string::npos);
  }

  std::deque<std::string> messages_split{};
  std::string current_message{};
  std::string::size_type pos_next{0};
  std::string::size_type pos_last{0};

  while ((pos_next = messages_combined.find(m_k_delimiter, pos_last)) !=
         std::string::npos) {
    messages_split.push_back(
        messages_combined.substr(pos_last, (pos_next - pos_last)));
    pos_last = pos_next + m_k_delimiter.size();
  }

  return {std::move(messages_split), std::move(message_fragment)};
}
