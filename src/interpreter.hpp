#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP 1

#include <map>
#include <vector>
#include <string>
#include "bytecode.hpp"

namespace sum {

	class Interpreter {
		public:
			static int get_interrupt_id(const std::string& name);

		private:
			std::vector<bytecode::subprogram> programs;
			std::map<std::string, size_t> program_map;

		public:
			bool register_subprogram(const bytecode::subprogram& prog);

			void execute(const std::string& program) const;
	};
}

#endif
