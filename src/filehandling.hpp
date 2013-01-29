#ifndef FILEHANDLING_HPP
#define FILEHANDLING_HPP 1

#include <string>
#include <set>


namespace sum {
namespace filehandling {
	bool dir_exists(const std::string& dir);
	std::set<std::string> get_subdirs(const std::string& dir);
	std::set<std::string> get_files(const std::string& dir);
}
}

#endif
