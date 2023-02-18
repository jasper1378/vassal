#ifndef IRC_MESSAGE_HPP
#define IRC_MESSAGE_HPP

#include <iostream>
#include <string>

namespace irc
{
    class message
    {
        public:

            enum class type
            {
                STANDARD,
                NUMERIC,
            };

            struct sender_info
            {
                std::string sender_nick;
                std::string sender_user;
                std::string sender_host;
            };

        private:

            sender_info m_sender_info;
            std::string m_recipient;
            std::string m_body;

        public:

            message();
            message(const std::string& raw_message);
            message(const message& other);
            message(message&& other);

            virtual ~message();

        public:

            virtual message* create_new() const =0;
            virtual message* create_clone() const =0;

            virtual type get_type() const =0;
            virtual std::string get_keyword() const =0;

            sender_info get_sender_info() const;
            std::string get_recipient() const;
            std::string get_body() const;

        public:

            static type check_type(const std::string& raw_message);

        public:

            message& operator=(const message& other);
            message& operator=(message&& other);

        public:

            friend std::ostream& operator<<(std::ostream& out, const message& m);

        protected:

            virtual void print(std::ostream& out) const;

        private:

            void parse_sender_info(const std::string& raw_message);
            void parse_recipient(const std::string& raw_message);
            void parse_body(const std::string& raw_message);
    };

    std::ostream& operator<<(std::ostream& out, const message& m);
}

#endif
