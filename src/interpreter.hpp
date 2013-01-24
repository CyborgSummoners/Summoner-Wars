#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP 1

#include <map>
#include <vector>
#include <list>
#include <string>
#include "bytecode.hpp"
#include "serverlogic.hpp"

namespace sum {
	using sum::Logic::Puppet;

	namespace stack_machine{
		class Stack;
	}

	class Interpreter {
		private:
			struct puppet_brain {
				Puppet& puppet;
				size_t program;
				size_t program_counter;
				size_t base_pointer;
				size_t delay;
				stack_machine::Stack* stack;
				std::map<size_t, size_t> overrides;

				puppet_brain(Puppet& puppet);
				~puppet_brain();
			};

		public:
			static int get_interrupt_id(const std::string& name);

		private:
			unsigned int clock;
			std::list<puppet_brain*> puppets;
			void enqueue_puppet(puppet_brain* puppet);
			puppet_brain* dequeue_puppet();

//			void adjust_priorities();
		private:
			std::vector<bytecode::subprogram> programs;
			std::map<std::string, size_t> program_map;

			size_t get_program_id(const std::string& str, const std::string& owner = "") const;

		public:
			Interpreter();

			//advance simulation by ticks many ticks, return true if anything meaningful happened, false otherwise.
			bool step(unsigned int ticks);

			bool register_subprogram(const bytecode::subprogram& prog);
			//void execute(const std::string& program) const;	// valahogy meg kéne oldani

			// Puppet regisztrációja: a puppet bekerül a végrehajtási sorba. A Puppet viselkedése végtelen NOP
			// True, ha sikeres, False, ha a puppet már regisztrálva volt.
			bool register_puppet(Puppet& puppet);

			// Puppet kivétele a végrehajtási sorból.
			// True, ha sikeres, False ha a puppet már nem volt benne a végrehajtási sorban.
			bool unregister_puppet(Puppet& puppet);

			// Puppet viselkedésének beállítása.
			// True, ha sikeres, False ha a viselkedés nem létezik, vagy ha a Puppet nem regisztrált.
			bool set_behaviour(Puppet& puppet, const std::string& behaviour, const std::string& owner = "");

			std::string get_behaviour(Puppet& puppet) const;

		private:
			// executes the instruction in program at program_counter, using the specified stack, with given base pointer
			unsigned int execute_instruction(Puppet& self, size_t& program_id, stack_machine::Stack& stack, size_t& pc, size_t& bp) const;
	};
}

#endif
