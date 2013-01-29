#include "filehandling.hpp"
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>

namespace sum {
namespace filehandling {

bool dir_exists(const std::string& dir) {
	std::string dn = "./"+dir;
	DIR* d = opendir( dn.c_str() );
	if(d == 0) return false;
	closedir(d);
	return true;
}

std::set<std::string> get_subdirs(const std::string& dir) {
	std::set<std::string> Result;
	std::string dn = "./"+dir;
	DIR* d = opendir( dn.c_str() );

	if(d == 0) return Result;

	dirent* entry = readdir(d);
	while(entry) {
		if(entry->d_type == DT_DIR && strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0 ) {
			Result.insert(entry->d_name);
		}
		entry = readdir(d);
	}

	closedir(d);
	return Result;
}

std::set<std::string> get_files(const std::string& dir) {
	std::set<std::string> Result;
	std::string dn = "./"+dir;
	DIR* d = opendir( dn.c_str() );

	if(d == 0) return Result;

	dirent* entry = readdir(d);
	while(entry) {
		if(entry->d_type == DT_REG) {
			Result.insert(entry->d_name);
		}
		entry = readdir(d);
	}

	closedir(d);
	return Result;
}


void read(const std::string& file, std::ostream& out) {
	std::ifstream f(file.c_str());
	if(!f) {
		std::stringstream msg;
		msg << "Could not open real file '." << file << "' for reading";
		throw std::runtime_error(msg.str().c_str());
		return;
	}
	out << f.rdbuf();
	f.close();
}


} // filehandling
} // sum
