#include "interpreter.hpp"
#include "util/debug.hpp"
#include "bytecode.hpp"
#include <ostream>
#include <stdexcept>

namespace sum {
	using namespace bytecode;

	namespace stack_machine {
		namespace except {
			struct corrupted_stack : public std::exception {};
			struct incompatible_types : public std::exception {};
			struct division_by_zero : public std::exception {};
			struct subprogram_does_not_exist : public std::exception {};
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

		struct ActivationRecord : public Cell {
			const size_t prog;
			const size_t pc;
			const size_t bp;

			ActivationRecord(size_t prog, size_t pc, size_t bp) : prog(prog), pc(pc), bp(bp) {
				this->tag = none;
			}
			virtual void print(std::ostream& out) const {
				out << "Activation record\n" << " \tprog: " << prog << "\n\tpc: " << pc << "\n\tbp: " << bp << std::endl;
			}
			virtual ActivationRecord* clone() const {
				return new ActivationRecord(this->prog, this->pc, this->bp);
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
				size_t get_stack_pointer() const;
				void set_stack_pointer(size_t sp);

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

		size_t Stack::get_stack_pointer() const {
			return stack.size();
		}
		void Stack::set_stack_pointer(size_t sp) {
			if(sp > stack.size()) {
				stack.resize(sp, 0); //~ stack.reserve(stack.size() - loc);
				return;
			}
			else if( sp == stack.size() ) return;

			for(size_t i=stack.size()-1; sp <= i; --i) {
				delete stack[i];
			}
			stack.resize(sp);
		}

		void Stack::reserve(size_t var_num) {
			stack.resize(stack.size()+var_num, 0);
			dout << "reserving " << var_num << " spaces, total " << stack.size() << std::endl;
		}




//*******************
//*** Interrupts ***
//*******************
		struct Interrupt {
			static const Interrupt* comparisons[];
			static const Interrupt* operators[];
			static const std::vector<Interrupt*> list;
			static const std::map<std::string, size_t> mapping;


			virtual void operator()(Stack& stack) const = 0;
			virtual const char* get_name() const = 0;
		};

		namespace interrupt {
			//*******************
			//*** Comparisons ***
			//*******************
			// operator==
			struct eq : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == r2->tag) {
						if(r1->tag == integer) {
							stack.push( new BooleanValue( static_cast<IntegerValue*>(r2)->value == static_cast<IntegerValue*>(r1)->value ) );
							done = true;
						}
						else if(r1->tag == boolean) {
							stack.push( new BooleanValue( static_cast<BooleanValue*>(r2)->value == static_cast<BooleanValue*>(r1)->value ) );
							done = true;
						}
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};
			// operator!=
			struct neq : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == r2->tag) {
						if(r1->tag == integer) {
							stack.push( new BooleanValue( static_cast<IntegerValue*>(r2)->value != static_cast<IntegerValue*>(r1)->value ) );
							done = true;
						}
						else if(r1->tag == boolean) {
							stack.push( new BooleanValue( static_cast<BooleanValue*>(r2)->value != static_cast<BooleanValue*>(r1)->value ) );
							done = true;
						}
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};
			// operator<
			struct less : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == r2->tag) {
						if(r1->tag == integer) {
							stack.push( new BooleanValue( static_cast<IntegerValue*>(r2)->value < static_cast<IntegerValue*>(r1)->value ) );
							done = true;
						}
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};
			// operator>
			struct greater : public Interrupt  {
				const char* get_name() const {
					return "";
				}
				void operator()(Stack& stack) const {
					bool done=false;
					Cell* r1 = stack.pop();
					Cell* r2 = stack.pop();
					if( r1->tag == r2->tag) {
						if(r1->tag == integer) {
							stack.push( new BooleanValue( static_cast<IntegerValue*>(r2)->value > static_cast<IntegerValue*>(r1)->value ) );
							done = true;
						}
					}
					delete r1;
					delete r2;
					if(!done) throw except::incompatible_types();
				}
			};

			//*****************
			//*** Operators ***
			//*****************
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
						if(static_cast<IntegerValue*>(r1)->value == 0) {
							delete r1;
							delete r2;
							throw except::division_by_zero();
						}
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
						if(static_cast<IntegerValue*>(r1)->value == 0) {
							delete r1;
							delete r2;
							throw except::division_by_zero();
						}
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
				Result.push_back( new interrupt::print() );
				return Result;
			}
			const std::map<std::string, size_t> mapping_init() {
				std::map<std::string, size_t> Result;
				for(size_t i=0; i<Interrupt::list.size(); ++i) Result.insert( std::make_pair( Interrupt::list[i]->get_name(), i ) );
				return Result;
			}
		} // namespace interrupt

		const Interrupt* Interrupt::operators[] = {
			new interrupt::add(),
			new interrupt::sub(),
			new interrupt::mul(),
			new interrupt::div(),
			new interrupt::mod(),
			new interrupt::land(),
			new interrupt::lor(),
			new interrupt::lnot()
		};
		const Interrupt* Interrupt::comparisons[] = {
			new interrupt::eq(),
			new interrupt::neq(),
			new interrupt::less(),
			new interrupt::greater()
		};
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


	size_t Interpreter::get_program_id(const std::string& str) const {
		std::map<std::string, size_t>::const_iterator it = program_map.find(str);
		if(it == program_map.end()) throw stack_machine::except::subprogram_does_not_exist();
		return it->second;
	}

	bool Interpreter::register_subprogram(const bytecode::subprogram& prog) {
		debugf("Registering subprogram %s...", prog.get_name().c_str());

		// do we already have a program with this name?
		std::map<std::string, size_t>::iterator it = program_map.find(prog.get_name());
		if(it != program_map.end()) return false;

		// if not, let's register it.
		program_map.insert( make_pair( prog.get_name(), programs.size() ) );
		programs.push_back(prog);

		debugf("done.\n");
		return true;
	}


	void Interpreter::execute(const std::string& program) const {
		using namespace stack_machine;
		using namespace bytecode;

		size_t prog_id = get_program_id(program);
		Stack stack;
		size_t pc=0;	//program counter.
		size_t bp=0;	//base pointer.
		Cell* r1;
		ActivationRecord* ar;
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
					stack.push( stack.var_at(bp + programs[prog_id].get_int(pc))->clone() );
					break;
				case STORE_X: //23
					r1 = stack.pop();
					stack.set_var_at(bp + programs[prog_id].get_int(pc), r1);
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
				case CALL:      //43
					ri = get_program_id( programs[prog_id].get_string(pc) );	// get callee's id.
					// parameters are on the top of the stack. callee will know how many

					// reserve space for return value

					// return to which program, at which point, and what was the base pointer?
					stack.push( new ActivationRecord(prog_id, pc, bp) );

					bp = stack.get_stack_pointer(); // new 0 is the top of the stack.
					prog_id = ri; // jump to new program
					pc = 0; // start at the beginning.
					break;
				case RET:		//44
					if(bp == 0) break;	// ez kicsit hack.

					stack.set_stack_pointer(bp); // rewind the stack all the way down to 0, to before the local vars
					r1 = stack.pop(); // get back the activation record.
					if((ar = dynamic_cast<ActivationRecord*>(r1))==0) throw stack_machine::except::corrupted_stack();
					// pop retval to r1, if applicable.
					prog_id = ar->prog;
					pc = ar->pc;
					bp = ar->bp;
					// don't forget to subtract the number of parameters from the bp!
					delete ar;
					break;
				case INTERRUPT: //45
					ri = programs[prog_id].get_int(pc);
					if(ri < Interrupt::list.size()) {
						(*Interrupt::list[ri])(stack);
					}

					break;
				// comparisons
				case EQ:
				case NEQ:
				case LESS:
				case GREATER:
					(*Interrupt::comparisons[opcode - EQ])(stack);
					break;
				// operations
				case ADDI:    //60
				case SUBI:
				case MULI:
				case DIVI:
				case MODI:
				case AND:
				case OR:
				case NOT:
					(*Interrupt::operators[opcode - ADDI])(stack);
					break;

				default:
					std::cerr << "unknown opcode " << (int)opcode << std::endl;
					break;
			} //switch
		} // while
	} // Interpreter::execute
}
