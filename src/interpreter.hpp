#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP 1

#include <map>
#include <vector>
#include <string>
#include "bytecode.hpp"

namespace sum {

	class Puppet;

	class Interpreter {
		public:
			static int get_interrupt_id(const std::string& name);

		private:
			std::vector<bytecode::subprogram> programs;
			std::map<std::string, size_t> program_map;

			size_t get_program_id(const std::string& str) const;

		public:
			// Interpreter lép egyet, végrehajtja a következő utasításblokkot.
			// visszaadja, mennyi tick idő telt el a legutóbbi állapot óta (jelenleg konstans 100)
			unsigned int step();

			bool register_subprogram(const bytecode::subprogram& prog);
			void execute(const std::string& program) const;

			// Puppet regisztrációja: a puppet bekerül a végrehajtási sorba. A Puppet viselkedése végtelen NOP
			// True, ha sikeres, False, ha a puppet már regisztrálva volt.
			bool register_puppet(Puppet& puppet);

			// Puppet kivétele a végrehajtási sorból.
			// Nem feltétlen kell jelentse a puppet halálát (paralízis, stb)
			bool unregister_puppet(Puppet& puppet);

			// Puppet viselkedésének beállítása.
			// True, ha sikeres, False ha a viselkedés nem létezik, vagy ha a Puppet nem regisztrált.
			bool set_behaviour(Puppet& puppet, const std::string& behaviour);

			std::string get_behaviour(Puppet& puppet) const;
	};
}

#endif
