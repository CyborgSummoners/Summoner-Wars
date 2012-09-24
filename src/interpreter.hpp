#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP 1

#include <map>
#include <vector>
#include <string>
#include "bytecode.hpp"

namespace sum {

	class Interpreter {
		public:
			typedef int program_id;

		private:
			std::vector<bytecode::subprogram> programs;
			std::map<std::string, program_id> program_map;

		public:
			bool register_subprogram(const bytecode::subprogram& prog);

			void execute(const std::string& program) const;
	};
}

#endif
