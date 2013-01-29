#ifndef FILEHANDLING_HPP
#define FILEHANDLING_HPP 1

#include <istream>
#include <string>
#include <set>


namespace sum {
namespace filehandling {
	bool dir_exists(const std::string& dir);
	std::set<std::string> get_subdirs(const std::string& dir);
	std::set<std::string> get_files(const std::string& dir);
	void read(const std::string& file, std::ostream& out);

}
}

#endif
