#include "interpreter.hpp"
#include "util/debug.hpp"
#include "bytecode.hpp"
#include <ostream>
#include <stdexcept>

namespace sum {
	using namespace bytecode;

	namespace stack_machine {
		namespace except {
			struct incompatible_types : public std::exception {};
			struct division_by_zero : public std::exception {};
		}

		struct Cell {
			type tag;
			virtual void print(std::ostream& out) const = 0;
			virtual Cell* clone() const = 0;
		};

		struct BooleanValue : public Cell {
			const bool value;
			BooleanValue(bool val) : value(val) {
				this->tag = boolean;
			}
			virtual void print(std::ostream& out) const {
				out << (this->value? "true" : "false") << std::endl;
			}
			virtual BooleanValue* clone() const {
				return new BooleanValue(this->value);
			}
		};


		struct IntegerValue : public Cell {
			const int value;

			IntegerValue(int val) : value(val) {
				this->tag = integer;
			}
			virtual void print(std::ostream& out) const {
				out << value << std::endl;
			}
			virtual IntegerValue* clone() const {
				return new IntegerValue(this->value);
			}
		};


		class Stack {
			private:
				std::vector<Cell*> stack;

			public:
				Cell* pop();
				Cell* top();
				void  push(Cell* cell);

				const Cell* var_at(size_t loc) const;
				void set_var_at(size_t loc, Cell* cell);

				void reserve(size_t var_num);

				~Stack(){
					for(size_t i=0; i<stack.size();++i) {
						if(stack[i]) delete stack[i];
					}
				}
		};

		Cell* Stack::pop() {
			Cell* Result = stack.back();
			stack.resize(stack.size()-1);
			return Result;
		}
		Cell* Stack::top() {
			return stack.back();
		}
		void  Stack::push(Cell* cell) {
			stack.push_back(cell);
			dout << "pushed ";
			cell->print(dout);
		}

		const Cell* Stack::var_at(size_t loc) const {
			return stack.at(loc);
		}

		void Stack::set_var_at(size_t loc, Cell* cell) {
			delete stack[loc];
			stack[loc] = cell;
			dout << "into "<< loc << " stored " << std::endl;
			stack[loc]->print(dout);
		}

		void Stack::reserve(size_t var_num) {
			stack.resize(stack.size()+var_num, 0);
			dout << "reserving " << var_num << " spaces, total " << stack.size() << std::endl;
		}


//*******************
//*** Interrupts ***
//*******************
		struct Interrupt {
			static const std::vector<Interrupt*> list;
			static const std::map<std::string, size_t> mapping;


			virtual void operator()(Stack& stack) const = 0;
			virtual const char* get_name() const = 0;
		};

		namespace interrupt {

			// operator+
			struct add : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == integer && r2->tag == integer ) {
						stack.push( new IntegerValue( static_cast<IntegerValue*>(r2)->value + static_cast<IntegerValue*>(r1)->value ) );
						done=true;
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};

			// operator-
			struct sub : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == integer && r2->tag == integer ) {
						stack.push( new IntegerValue( static_cast<IntegerValue*>(r2)->value - static_cast<IntegerValue*>(r1)->value ) );
						done=true;
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};

			// operator*
			struct mul : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == integer && r2->tag == integer ) {
						stack.push( new IntegerValue( static_cast<IntegerValue*>(r2)->value * static_cast<IntegerValue*>(r1)->value ) );
						done=true;
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};

			// operator /
			struct div : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == integer && r2->tag == integer ) {
						stack.push( new IntegerValue( static_cast<IntegerValue*>(r2)->value / static_cast<IntegerValue*>(r1)->value ) );
						done=true;
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};

			// operator %
			struct mod : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == integer && r2->tag == integer ) {
						stack.push( new IntegerValue( static_cast<IntegerValue*>(r2)->value % static_cast<IntegerValue*>(r1)->value ) );
						done=true;
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};

			// operator &&
			struct land : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == boolean && r2->tag == boolean ) {
						stack.push( new BooleanValue( static_cast<BooleanValue*>(r2)->value && static_cast<BooleanValue*>(r1)->value ) );
						done=true;
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};

			// operator ||
			struct lor : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == boolean && r2->tag == boolean ) {
						stack.push( new BooleanValue( static_cast<BooleanValue*>(r2)->value || static_cast<BooleanValue*>(r1)->value ) );
						done=true;
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};

			// operator !
			struct lnot : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					if( r1->tag == boolean ) {
						stack.push( new BooleanValue( !(static_cast<BooleanValue*>(r1)->value) ) );
						done=true;
					}
					delete r1;
					if(!done) throw except::incompatible_types();
				}
			};

			// builtin: print
			struct print : public Interrupt  {
				const char* get_name() const {
					return "PRINT";
				}
				void operator()(Stack& stack) const {
					Cell* r1 = stack.pop();
					r1->print(std::cout);
					delete r1;
				}
			};

			const std::vector<Interrupt*> list_init() {
				std::vector<Interrupt*> Result;
				Result.push_back( new interrupt::add() );
				Result.push_back( new interrupt::sub() );
				Result.push_back( new interrupt::mul() );
				Result.push_back( new interrupt::div() );
				Result.push_back( new interrupt::mod() );
				Result.push_back( new interrupt::land() );
				Result.push_back( new interrupt::lor() );
				Result.push_back( new interrupt::lnot() );
				Result.push_back( new interrupt::print() );
				return Result;
			}
			const std::map<std::string, size_t> mapping_init() {
				std::map<std::string, size_t> Result;
				for(size_t i=0; i<Interrupt::list.size(); ++i) Result.insert( std::make_pair( Interrupt::list[i]->get_name(), i ) );
				return Result;
			}
		} // namespace interrupt

		const std::vector<Interrupt*> Interrupt::list = interrupt::list_init();
		const std::map<std::string, size_t> Interrupt::mapping = interrupt::mapping_init();


	} // namespace stack_machine


//*******************
//*** Interpreter ***
//*******************
	int Interpreter::get_interrupt_id(const std::string& name) {
		std::map<std::string, size_t>::const_iterator it = stack_machine::Interrupt::mapping.find(name);
		if(it == stack_machine::Interrupt::mapping.end()) return -1;
		return it->second;
	}


	bool Interpreter::register_subprogram(const bytecode::subprogram& prog) {
		// do we already have a program with this name?
		std::map<std::string, size_t>::iterator it = program_map.find(prog.get_name());
		if(it != program_map.end()) return false;

		// if not, let's register it.
		program_map.insert( make_pair( prog.get_name(), programs.size() ) );
		programs.push_back(prog);

		return true;
	}


	void Interpreter::execute(const std::string& program) const {
		using namespace stack_machine;
		using namespace bytecode;
		size_t prog_id = program_map.find(program) -> second;
		Stack stack;
		size_t pc=0;	//program counter.
		Cell* r1;
		Cell* r2;
		size_t ri;

		while(pc < programs[prog_id].len) {
			byte opcode = programs[prog_id].get_byte(pc);

			dout << "#" << pc << " Opcode: " << (int)opcode <<std::endl;
			switch(opcode) {
				case NOP:	// 0
					break;

				// stack
				case PUSH:    // 1
					stack.push(new IntegerValue( programs[prog_id].get_int(pc) ));
					break;
				case PSHB:    // 2
					stack.push(new BooleanValue( programs[prog_id].get_int(pc) ));
					break;
				case ISP:     //10
					stack.reserve( programs[prog_id].get_int(pc) );
					break;
				case FETCH_X: //23
					stack.push( stack.var_at(programs[prog_id].get_int(pc))->clone() );
					break;
				case STORE_X: //23
					r1 = stack.pop();
					stack.set_var_at(programs[prog_id].get_int(pc), r1);
					break;

				// control flow
				case JMP:      //40
					pc = programs[prog_id].get_int(pc);
					dout << "jumping to " << pc <<std::endl;
					break;
				case JMPTRUE:  //41
					r1 = stack.pop();
					if(r1->tag == boolean && dynamic_cast<BooleanValue*>(r1)->value == true) {
						pc = programs[prog_id].get_int(pc);
						dout << "jumping to " << pc <<std::endl;
					} else programs[prog_id].get_int(pc);	//mindenképp be kell olvasni
					delete r1;
					break;
				case JMPFALSE: //42
					r1 = stack.pop();
					if(r1->tag == boolean && dynamic_cast<BooleanValue*>(r1)->value == false) {
						pc = programs[prog_id].get_int(pc);
						dout << "jumping to " << pc <<std::endl;
					} else programs[prog_id].get_int(pc);
					delete r1;
					break;

				case INTERRUPT: //45
					ri = programs[prog_id].get_int(pc);
					if(ri < Interrupt::list.size()) {
						(*Interrupt::list[ri])(stack);
					}

					break;
				// comparisons

				// operations
				case ADDI:    //60
				case SUBI:
				case MULI:
				case DIVI:
				case MODI:
				case AND:
				case OR:
				case NOT:
					(*Interrupt::list[opcode - ADDI])(stack);
					break;

				default:
					std::cerr << "unknown opcode " << (int)opcode << std::endl;
					break;
			} //switch
		} // while
	} // Interpreter::execute
}
