#include "irc_numeric_message.hpp"

#include "irc_message.hpp"

#include <charconv>
#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

const std::unordered_map<int, std::string>
    vassal::irc::numeric_message::m_k_code_string_hash_table{
        {001, "RPL_WELCOME"},
        {002, "RPL_YOURHOST"},
        {003, "RPL_CREATED"},
        {004, "RPL_MYINFO"},
        {005, "RPL_BOUNCE"},
        {200, "RPL_TRACELINK"},
        {201, "RPL_TRACECONNECTING"},
        {202, "RPL_TRACEHANDSHAKE"},
        {203, "RPL_TRACEUNKNOWN"},
        {204, "RPL_TRACEOPERATOR"},
        {205, "RPL_TRACEUSER"},
        {206, "RPL_TRACESERVER"},
        {207, "RPL_TRACESERVICE"},
        {208, "RPL_TRACENEWTYPE"},
        {209, "RPL_TRACECLASS"},
        {210, "RPL_TRACERECONNECT"},
        {211, "RPL_STATSLINKINFO"},
        {212, "RPL_STATSCOMMANDS"},
        {213, "RPL_STATSCLINE"},
        {214, "RPL_STATSNLINE"},
        {215, "RPL_STATSILINE"},
        {216, "RPL_STATSKLINE"},
        {217, "RPL_STATSQLINE"},
        {218, "RPL_STATSYLINE"},
        {219, "RPL_ENDOFSTATS"},
        {221, "RPL_UMODEIS"},
        {231, "RPL_SERVICEINFO"},
        {232, "RPL_ENDOFSERVICES"},
        {233, "RPL_SERVICE"},
        {234, "RPL_SERVLIST"},
        {235, "RPL_SERVLISTEND"},
        {240, "RPL_STATSVLINE"},
        {241, "RPL_STATSLLINE"},
        {242, "RPL_STATSUPTIME"},
        {243, "RPL_STATSOLINE"},
        {244, "RPL_STATSHLINE"},
        {244, "RPL_STATSSLINE"},
        {246, "RPL_STATSPING"},
        {247, "RPL_STATSBLINE"},
        {250, "RPL_STATSDLINE"},
        {251, "RPL_LUSERCLIENT"},
        {252, "RPL_LUSEROP"},
        {253, "RPL_LUSERUNKNOWN"},
        {254, "RPL_LUSERCHANNELS"},
        {255, "RPL_LUSERME"},
        {256, "RPL_ADMINME"},
        {257, "RPL_ADMINLOC1"},
        {258, "RPL_ADMINLOC2"},
        {259, "RPL_ADMINEMAIL"},
        {261, "RPL_TRACELOG"},
        {262, "RPL_TRACEEND"},
        {263, "RPL_TRYAGAIN"},
        {300, "RPL_NONE"},
        {301, "RPL_AWAY"},
        {302, "RPL_USERHOST"},
        {303, "RPL_ISON"},
        {305, "RPL_UNAWAY"},
        {306, "RPL_NOWAWAY"},
        {311, "RPL_WHOISUSER"},
        {312, "RPL_WHOISSERVER"},
        {313, "RPL_WHOISOPERATOR"},
        {314, "RPL_WHOWASUSER"},
        {315, "RPL_ENDOFWHO"},
        {316, "RPL_WHOISCHANOP"},
        {317, "RPL_WHOISIDLE"},
        {318, "RPL_ENDOFWHOIS"},
        {319, "RPL_WHOISCHANNELS"},
        {321, "RPL_LISTSTART"},
        {322, "RPL_LIST"},
        {323, "RPL_LISTEND"},
        {324, "RPL_CHANNELMODEIS"},
        {325, "RPL_UNIQOPIS"},
        {331, "RPL_NOTOPIC"},
        {332, "RPL_TOPIC"},
        {341, "RPL_INVITING"},
        {342, "RPL_SUMMONING"},
        {346, "RPL_INVITELIST"},
        {347, "RPL_ENDOFINVITELIST"},
        {348, "RPL_EXCEPTLIST"},
        {349, "RPL_ENDOFEXCEPTLIST"},
        {351, "RPL_VERSION"},
        {352, "RPL_WHOREPLY"},
        {353, "RPL_NAMREPLY"},
        {361, "RPL_KILLDONE"},
        {362, "RPL_CLOSING"},
        {363, "RPL_CLOSEEND"},
        {364, "RPL_LINKS"},
        {365, "RPL_ENDOFLINKS"},
        {366, "RPL_ENDOFNAMES"},
        {367, "RPL_BANLIST"},
        {368, "RPL_ENDOFBANLIST"},
        {369, "RPL_ENDOFWHOWAS"},
        {371, "RPL_INFO"},
        {372, "RPL_MOTD"},
        {373, "RPL_INFOSTART"},
        {374, "RPL_ENDOFINFO"},
        {375, "RPL_MOTDSTART"},
        {376, "RPL_ENDOFMOTD"},
        {381, "RPL_YOUREOPER"},
        {382, "RPL_REHASHING"},
        {383, "RPL_YOURESERVICE"},
        {384, "RPL_MYPORTIS"},
        {391, "RPL_TIME"},
        {392, "RPL_USERSSTART"},
        {393, "RPL_USERS"},
        {394, "RPL_ENDOFUSERS"},
        {395, "RPL_NOUSERS"},
        {401, "ERR_NOSUCHNICK"},
        {402, "ERR_NOSUCHSERVER"},
        {403, "ERR_NOSUCHCHANNEL"},
        {404, "ERR_CANNOTSENDTOCHAN"},
        {405, "ERR_TOOMANYCHANNELS"},
        {406, "ERR_WASNOSUCHNICK"},
        {407, "ERR_TOOMANYTARGETS"},
        {408, "ERR_NOSUCHSERVICE"},
        {409, "ERR_NOORIGIN"},
        {411, "ERR_NORECIPIENT"},
        {412, "ERR_NOTEXTTOSEND"},
        {413, "ERR_NOTOPLEVEL"},
        {414, "ERR_WILDTOPLEVEL"},
        {415, "ERR_BADMASK"},
        {421, "ERR_UNKNOWNCOMMAND"},
        {422, "ERR_NOMOTD"},
        {423, "ERR_NOADMININFO"},
        {424, "ERR_FILEERROR"},
        {431, "ERR_NONICKNAMEGIVEN"},
        {432, "ERR_ERRONEUSNICKNAME"},
        {433, "ERR_NICKNAMEINUSE"},
        {436, "ERR_NICKCOLLISION"},
        {437, "ERR_UNAVAILRESOURCE"},
        {441, "ERR_USERNOTINCHANNEL"},
        {442, "ERR_NOTONCHANNEL"},
        {443, "ERR_USERONCHANNEL"},
        {444, "ERR_NOLOGIN"},
        {445, "ERR_SUMMONDISABLED"},
        {446, "ERR_USERSDISABLED"},
        {451, "ERR_NOTREGISTERED"},
        {461, "ERR_NEEDMOREPARAMS"},
        {462, "ERR_ALREADYREGISTRED"},
        {463, "ERR_NOPERMFORHOST"},
        {464, "ERR_PASSWDMISMATCH"},
        {465, "ERR_YOUREBANNEDCREEP"},
        {466, "ERR_YOUWILLBEBANNED"},
        {467, "ERR_KEYSET"},
        {471, "ERR_CHANNELISFULL"},
        {472, "ERR_UNKNOWNMODE"},
        {473, "ERR_INVITEONLYCHAN"},
        {474, "ERR_BANNEDFROMCHAN"},
        {475, "ERR_BADCHANNELKEY"},
        {476, "ERR_BADCHANMASK"},
        {477, "ERR_NOCHANMODES"},
        {478, "ERR_BANLISTFULL"},
        {481, "ERR_NOPRIVILEGES"},
        {482, "ERR_CHANOPRIVSNEEDED"},
        {483, "ERR_CANTKILLSERVER"},
        {484, "ERR_RESTRICTED"},
        {485, "ERR_UNIQOPPRIVSNEEDED"},
        {491, "ERR_NOOPERHOST"},
        {492, "ERR_NOSERVICEHOST"},
        {501, "ERR_UMODEUNKNOWNFLAG"},
        {502, "ERR_USERSDONTMATCH"}};

vassal::irc::numeric_message::numeric_message()
    : message{}, m_code{}, m_code_string{}, m_is_error{},
      m_unknown_code_policy{}, m_is_code_known{} {}

vassal::irc::numeric_message::numeric_message(
    const std::string_view raw_message,
    const unknown_code_policy
        unknown_code_policy /*= unknown_code_policy::relaxed*/)
    : message{raw_message}, m_code{}, m_code_string{}, m_is_error{},
      m_unknown_code_policy{unknown_code_policy}, m_is_code_known{} {
  parse_code(raw_message);
}

vassal::irc::numeric_message::numeric_message(const numeric_message &other)
    : message{other}, m_code{other.m_code}, m_code_string{other.m_code_string},
      m_is_error{other.m_is_error},
      m_unknown_code_policy{other.m_unknown_code_policy},
      m_is_code_known{other.m_is_code_known} {}

vassal::irc::numeric_message::numeric_message(numeric_message &&other) noexcept
    : message{std::move(other)}, m_code{std::move(other.m_code)},
      m_code_string{std::move(other.m_code_string)},
      m_is_error{std::move(other.m_is_error)},
      m_unknown_code_policy{std::move(other.m_unknown_code_policy)},
      m_is_code_known{std::move(other.m_is_code_known)} {}

vassal::irc::numeric_message::~numeric_message() {}

vassal::irc::numeric_message *vassal::irc::numeric_message::create_new() const {
  return new numeric_message{};
}

vassal::irc::numeric_message *
vassal::irc::numeric_message::create_clone() const {
  return new numeric_message{*this};
}

vassal::irc::message::type vassal::irc::numeric_message::get_type() const {
  return type::numeric;
}

std::string vassal::irc::numeric_message::get_keyword() const {
  return std::to_string(m_code);
}

int vassal::irc::numeric_message::get_code() const { return m_code; }

std::string vassal::irc::numeric_message::get_code_string() const {
  return m_code_string;
}

bool vassal::irc::numeric_message::get_is_error() const { return m_is_error; }

vassal::irc::numeric_message::unknown_code_policy
vassal::irc::numeric_message::get_unknown_code_policy() const {
  return m_unknown_code_policy;
}

bool vassal::irc::numeric_message::get_is_code_known() const {
  return m_is_code_known;
}

vassal::irc::numeric_message &
vassal::irc::numeric_message::operator=(const numeric_message &other) {
  message::operator=(other);
  m_code = other.m_code;
  m_code_string = other.m_code_string;
  m_is_error = other.m_is_error;
  m_unknown_code_policy = other.m_unknown_code_policy;
  m_is_code_known = other.m_is_code_known;

  return *this;
}

vassal::irc::numeric_message &
vassal::irc::numeric_message::operator=(numeric_message &&other) noexcept(
    std::is_nothrow_move_assignable_v<message>
        &&std::is_nothrow_move_assignable_v<std::string>) {
  message::operator=(std::move(other));
  m_code = std::move(other.m_code);
  m_code_string = std::move(other.m_code_string);
  m_is_error = std::move(other.m_is_error);
  m_unknown_code_policy = std::move(other.m_unknown_code_policy);
  m_is_code_known = std::move(other.m_is_code_known);

  return *this;
}

void vassal::irc::numeric_message::print(std::ostream &out) const {
  message::print(out);
}

void vassal::irc::numeric_message::parse_code(
    const std::string_view raw_message) {
  static constexpr std::string::size_type code_pos_word{2};
  static constexpr char delimiter_space{' '};

  std::string::size_type pos_last{std::string::npos};
  std::string::size_type pos_cur{std::string::npos};

  for (size_t i{0}; i < code_pos_word; ++i) {
    pos_last = pos_cur;
    pos_cur = raw_message.find(delimiter_space, (pos_last + 1));
  }

  std::from_chars((raw_message.data() + pos_last + 1),
                  (raw_message.data() + pos_cur), m_code);

  if (m_k_code_string_hash_table.contains(m_code) == false) {
    m_is_code_known = false;

    switch (m_unknown_code_policy) {
    case unknown_code_policy::relaxed:
      m_code_string = "";
      break;
    case unknown_code_policy::strict:
      throw std::runtime_error{"message contains unknown numeric code " +
                               std::to_string(m_code)};
      break;
    }
  } else {
    m_is_code_known = true;

    m_code_string = m_k_code_string_hash_table.at(m_code);
  }

  static constexpr int client_only_reply_range_start{1};
  static constexpr int client_only_reply_range_end{99};
  static constexpr int command_reply_range_start{200};
  static constexpr int command_reply_range_end{399};
  static constexpr int error_range_start{400};
  static constexpr int error_range_end{599};

  if ((m_code >= error_range_start) && (m_code <= error_range_end)) {
    m_is_error = true;
  } else if (((m_code >= client_only_reply_range_start) &&
              (m_code <= client_only_reply_range_end)) ||
             ((m_code >= command_reply_range_start) &&
              (m_code <= command_reply_range_end))) {
    m_is_error = false;
  } else {
    switch (m_unknown_code_policy) {
    case unknown_code_policy::relaxed:
      m_is_error = false;
      break;
    case unknown_code_policy::strict:
      throw std::runtime_error{"message contains unknown numeric code " +
                               std::to_string(m_code)};
      break;
    }
  }
}
