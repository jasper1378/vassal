AM_CXXFLAGS = -std=c++20
bin_PROGRAMS = vassal
vassal_SOURCES =                   \
	color_codes.hpp            \
	date_time_format_print.cpp \
	date_time_format_print.hpp \
	irc_core.cpp               \
	irc_core.hpp               \
	irc_message.cpp            \
	irc_message.hpp            \
	irc_numeric_message.cpp    \
	irc_numeric_message.hpp    \
	irc_standard_message.cpp   \
	irc_standard_message.hpp   \
	main.cpp                   \
	output_mutex.cpp           \
	output_mutex.hpp
vassal_CPPFLAGS = -I$(top_srcdir)/deps/bits-and-bytes/include -I$(top_srcdir)/deps/libconfigfile/include -I$(top_srcdir)/deps/liblocket/include
vassal_LDADD = $(top_builddir)/deps/libconfigfile/src/libconfigfile.la $(top_builddir)/deps/liblocket/src/liblocket.la
