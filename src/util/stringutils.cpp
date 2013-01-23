#include "stringutils.hpp"
#include <stdexcept>

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

bool to_unsigned(const std::string& str, unsigned int& Result) {
	Result = 0;

	for(size_t i=0; i<str.size(); ++i) {
		if(str[i]<'0' || str[i]>'9') return false;
		Result = Result*10 + (str[i] - '0');
	}

	return true;
}

}
}
