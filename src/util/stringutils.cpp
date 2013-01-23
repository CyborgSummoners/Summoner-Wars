#include "stringutils.hpp"

namespace sum {
namespace stringutils {


std::vector<std::string> string_explode(const std::string& str, const char& ch) {
    std::string next = "";
    std::vector<std::string> result;

    for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
    	if (*it == ch) {
    		if (next.length() > 0) {
    			result.push_back(next);
    			next = "";
    		}
    	} else {
    		next += *it;
    	}
    }
    result.push_back(next);

    return result;
}

std::vector<std::string> string_explode(const std::string& str, const std::string& chars) {
	std::string next = "";
    std::vector<std::string> result;

    for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
    	if ( chars.find(*it) != std::string::npos ) {
    		if (next.length() > 0) {
    			result.push_back(next);
    			next = "";
    		}
    	} else {
    		next += *it;
    	}
    }
    result.push_back(next);

    return result;
}

}
}
