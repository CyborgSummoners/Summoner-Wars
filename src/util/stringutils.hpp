#ifndef UTIL_STRINGUTILS
#define UTIL_STRINGUTILS 1

#include <string>
#include <vector>

namespace sum {
namespace stringutils {

const std::string whitespace = "\t\n ";

std::vector<std::string> string_explode(const std::string& str, const char& ch);
std::vector<std::string> string_explode(const std::string& str, const std::string& chars);

}
}

#endif
