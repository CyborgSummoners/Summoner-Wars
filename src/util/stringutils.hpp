#ifndef UTIL_STRINGUTILS
#define UTIL_STRINGUTILS 1

#include <string>
#include <vector>

namespace sum {
namespace stringutils {

const std::string whitespace = "\t\n ";

std::vector<std::string> string_explode(const std::string& str, const char& ch);
std::vector<std::string> string_explode(const std::string& str, const std::string& chars);
std::string trim(const std::string& str);

bool to_unsigned(const std::string& str, unsigned int& Result);

std::string float_to_string(float f);

}
}

#endif
