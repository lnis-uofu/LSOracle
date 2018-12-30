#ifndef FORMAT_HH
#define FORMAT_HH

#include <stdexcept>
#include <sstream>
#include <string>

void format(std::ostringstream &ss, const char *s)
{
	while (*s) {
		if (*s == '$')
			throw std::runtime_error("too few arguments provided to format");

		ss << *s++;
	}
}

template<typename t, typename... Args>
void format(std::ostringstream &ss, const char *s, t &value, Args... args)
{
	while (*s) {
		if (*s == '$') {
			ss << value;
			format(ss, ++s, args...);
			return;
		}

		ss << *s++;
	}

	throw std::runtime_error("too many arguments provided to format");
}

template<typename... Args>
std::string format(const char *s, Args... args)
{
	std::ostringstream ss;
	format(ss, s, args...);
	return ss.str();
}

#endif
