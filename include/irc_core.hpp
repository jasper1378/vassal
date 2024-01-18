#ifndef VASSAL_IRC_CORE_HPP
#define VASSAL_IRC_CORE_HPP

#include "message.hpp"
#include "numeric_message.hpp"
#include "standard_message.hpp"

#include "liblocket.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace irc {
class core {
public:
  enum class channel_mode {
    O = 0, // give "channel creator" status
    o,     // give/take channel operator privilege
    v,     // give/take the voice privilege
    a,     // toggle the anonymous channel flag
    i,     // toggle the invite-only channel flag
    m,     // toggle the moderated channel
    n,     // toggle the no messages to channel from clients on the outside
    q,     // toggle the quiet channel flag
    p,     // toggle the private channel flag
    s,     // toggle the secret channel flag
    r,     // toggle the server reop channel flag
    t,     // toggle the topic settable by channel operator only flag
    k,     // set/remove the channel key (password)
    l,     // set/remove the user limit to channel
    b,     // set/remove ban mask to keep users out
    e,     // set/remove an exception mask to override a ban mask
    I,     // set/remove an invitation mask to automatically override the
           // invite-only flag
    MAX,
  };

  enum class user_mode {
    a = 0, // user is flagged as away
    i,     // marks a users as invisible
    w,     // user receives wallops
    r,     // restricted user connection
    o,     // operator flag
    O,     // local operator flag
    s,     // marks a user for receipt of server notices
    MAX,
  };

  enum class mode_operation {
    ADD = 0,
    REMOVE,
    MAX,
  };

  enum class stats_query {
    l = 0, // returns a list of the server's connections, showing how long each
           // connection has been established and the traffic over that
           // connection in Kbytes and messages for each direction
    m, // returns the usage count for each of commands supported by the server;
       // commands for which the usage count is zero MAY be omitted
    o, // returns a list of configured privileged users, operators
    u, // returns a string showing how long the server has been up
    MAX,
  };

private:
  liblocket::inet_socket_addr *m_server_address;
  std::string m_nick;

  liblocket::client_stream_socket m_socket;
  std::mutex m_socket_mutex_write;

  std::deque<message *> m_unread_responses;
  std::atomic<bool> m_new_unread_response;
  std::mutex m_unread_responses_mutex;
  std::condition_variable m_unread_responses_cv;

  std::thread m_listener_thread;
  std::atomic<bool> m_listener_thread_kill_yourself;

private:
  static constexpr int m_k_max_message_length{510};
  static constexpr std::string m_k_delimiter{"\r\n"};

  static constexpr std::array<char, static_cast<size_t>(channel_mode::MAX)>
      m_k_channel_mode_lut{'O', 'o', 'v', 'a', 'i', 'm', 'n', 'q', 'p',
                           's', 'r', 't', 'k', 'l', 'b', 'e', 'I'};
  static constexpr std::array<char, static_cast<size_t>(user_mode::MAX)>
      m_k_user_mode_lut{'a', 'i', 'w', 'r', 'o', 'O', 's'};
  static constexpr std::array<char, static_cast<size_t>(mode_operation::MAX)>
      m_k_mode_operation_lut{'+', '-'};
  static constexpr std::array<char, static_cast<size_t>(stats_query::MAX)>
      m_k_stats_query_lut{'l', 'm', 'o', 'u'};

public:
  core(const std::string &server_address, uint16_t port_num,
       const std::string &nick, const std::string &realname,
       liblocket::inet_socket_addr::ip_version ip_version =
           liblocket::inet_socket_addr::ip_version::IPv4,
       const std::string &server_password = "");
  core(core &&other) /*TODO: noexcept()*/;

  core(const core &other) = delete;

  ~core();

public:
  message *recv_response();

  void send_message_pass(const std::string &password);
  void send_message_nick(const std::string &nickname);
  void send_message_user(const std::string &username,
                         const std::string &realname,
                         const std::pair<user_mode, user_mode> modes = {
                             user_mode::MAX, user_mode::MAX});
  void send_message_oper(const std::string &name, const std::string &password);
  void
  send_message_user_mode(const std::string &nickname,
                         const user_mode mode = user_mode::MAX,
                         const mode_operation operation = mode_operation::MAX);
  void send_message_quit(const std::string &quit_message);
  void send_message_squit(const std::string &server,
                          const std::string &comment);

  void send_message_join(const std::string &channel,
                         const std::string &key = "");
  void send_message_join(const std::vector<std::pair<std::string, std::string>>
                             &channels_and_keys);
  void send_message_part(const std::vector<std::string> &channels,
                         const std::string &part_message = "");
  void send_message_part(const std::string &channel,
                         const std::string &part_message = "");
  void send_message_part_all();
  void send_message_channel_mode(
      const std::string &channel, const channel_mode mode,
      const mode_operation operation = mode_operation::MAX,
      const std::string &options = "");
  void send_message_topic(const std::string &channel,
                          const std::string &topic = "");
  void send_message_names(const std::string &channel = "",
                          const std::string &target = "");
  void send_message_names(const std::vector<std::string> &channels = {},
                          const std::string &target = "");
  void send_message_list(const std::string &channel = "",
                         const std::string &target = "");
  void send_message_list(const std::vector<std::string> &channels = {},
                         const std::string &target = "");
  void send_message_invite(const std::string &nickname,
                           const std::string &channel);
  void send_message_kick(const std::string &channel, const std::string &user,
                         const std::string &comment = "");
  void send_message_kick(const std::string &channel,
                         const std::vector<std::string> &users,
                         const std::string &comment = "");
  void send_message_kick(const std::vector<std::string> &channels,
                         const std::vector<std::string> &users,
                         const std::string &comment = "");

  void send_message_privmsg(const std::string &receiver,
                            const std::string &text_to_be_sent);
  void send_message_notice(const std::string &receiver,
                           const std::string &text);

  void send_message_motd(const std::string &target = "");
  void send_message_lusers(const std::string &mask = "",
                           const std::string &target = "");
  void send_message_version(const std::string &target = "");
  void send_message_stats(stats_query query = stats_query::MAX,
                          const std::string &target = "");
  void send_message_links(const std::string &remote_server = "",
                          const std::string &server_mask = "");
  void send_message_time(const std::string &target = "");
  void send_message_connect(const std::string &target_server,
                            const std::string &port,
                            const std::string &remote_server = "");
  void send_message_trace(const std::string &target = "");
  void send_message_admin(const std::string &target = "");
  void send_message_info(const std::string &target = "");

  void send_message_servlist(const std::string &mask = "",
                             const std::string &type = "");
  void send_message_squery(const std::string &service_name,
                           const std::string &text);

  void send_message_who(const std::string &mask = "", bool only_opers = false);
  void send_message_whois(const std::string &mask,
                          const std::string &target = "");
  void send_message_whois(const std::vector<std::string> &mask,
                          const std::string &target = "");
  void send_message_whowas(const std::string &nickname, const int count = -1,
                           const std::string &target = "");
  void send_message_whowas(const std::vector<std::string> &nicknames,
                           const int count = -1,
                           const std::string &target = "");

  void send_message_kill(const std::string &nickname,
                         const std::string &comment);
  bool send_message_pong(const std::string &possible_ping_message);

  void send_message_away(const std::string &text = "");
  void send_message_rehash();
  void send_message_die();
  void send_message_restart();
  void send_message_summon(const std::string &user,
                           const std::string &target = "",
                           const std::string &channel = "");
  void send_message_users(const std::string &target = "");
  void send_message_wallops(const std::string &text);
  void send_message_userhost(const std::string &nickname);
  void send_message_userhost(const std::vector<std::string> &nicknames);
  void send_message_ison(const std::string &nickname);
  void send_message_ison(const std::vector<std::string> &nicknames);

public:
  core &operator=(core &&other) /*TODO: noexcept()*/;

  core &operator=(const core &other) = delete;

public:
  void listen();
  void send_message(const std::string &message);

private:
  static std::pair<std::deque<std::string>, std::string>
  split_messages(const std::string &message);
};
} // namespace irc

#endif
