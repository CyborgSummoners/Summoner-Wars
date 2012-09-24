#include "interpreter.hpp"
#include "bytecode.hpp"
#include <iostream>
#include <stdexcept>

namespace sum {
	enum type{ none, boolean, integer };


	namespace stack_machine {
		namespace except {
			struct incompatible_types : public std::exception {};
			struct division_by_zero : public std::exception {};
		}

		struct Cell {
			type tag;
			virtual Cell* less(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* greater(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* eq(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* neq(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* add(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* sub(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* mul(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* div(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* mod(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* land(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* lor(Cell* left) {
				throw except::incompatible_types();
			};
			virtual Cell* lnot() {
				throw except::incompatible_types();
			};

			virtual void print(std::ostream& out) const = 0;
			virtual Cell* clone() const = 0;
		};


		struct BooleanValue : public Cell {
			const bool value;
			BooleanValue(bool val) : value(val) {
				this->tag = boolean;
			}
			virtual Cell* land(Cell* left) {
				if(left->tag == boolean) return new BooleanValue( (dynamic_cast<BooleanValue*>(left)->value) && (this->value) );

				throw except::incompatible_types();
				return 0;
			}
			virtual Cell* lor(Cell* left) {
				if(left->tag == boolean) return new BooleanValue( dynamic_cast<BooleanValue*>(left)->value || this->value );

				throw except::incompatible_types();
				return 0;
			}
			virtual Cell* lnot() {
				return new BooleanValue( !(this->value) );
			};
			virtual Cell* eq(Cell* left) {
				if(left->tag == boolean) return new BooleanValue( dynamic_cast<BooleanValue*>(left)->value == this->value );

				throw except::incompatible_types();
				return 0;
			}
			virtual Cell* neq(Cell* left) {
				if(left->tag == boolean) return new BooleanValue( dynamic_cast<BooleanValue*>(left)->value != this->value );

				throw except::incompatible_types();
				return 0;
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

			virtual Cell* less(Cell* left) {
				if(left->tag == integer) return new BooleanValue( dynamic_cast<IntegerValue*>(left)->value < this->value );

				throw except::incompatible_types();
				return 0;
			}
			virtual Cell* greater(Cell* left) {
				if(left->tag == integer) return new BooleanValue( dynamic_cast<IntegerValue*>(left)->value > this->value );

				throw except::incompatible_types();
				return 0;
			}
			virtual Cell* eq(Cell* left) {
				if(left->tag == integer) return new BooleanValue( dynamic_cast<IntegerValue*>(left)->value == this->value );

				throw except::incompatible_types();
				return 0;
			}
			virtual Cell* neq(Cell* left) {
				if(left->tag == integer) return new BooleanValue( dynamic_cast<IntegerValue*>(left)->value != this->value );

				throw except::incompatible_types();
				return 0;
			}

			virtual Cell* add(Cell* left) {
				if(left->tag == integer) return new IntegerValue( this->value + dynamic_cast<IntegerValue*>(left)->value );

				throw except::incompatible_types();
				return 0;
			}
			virtual Cell* sub(Cell* left) {
				if(left->tag == integer) return new IntegerValue( dynamic_cast<IntegerValue*>(left)->value - this->value );

				throw except::incompatible_types();
				return 0;
			}
			virtual Cell* mul(Cell* left) {
				if(left->tag == integer) return new IntegerValue( dynamic_cast<IntegerValue*>(left)->value * this->value );

				throw except::incompatible_types();
				return 0;
			}
			virtual Cell* div(Cell* left) {
				if(left->tag == integer) {
					if(this->value == 0) throw except::division_by_zero();
					return new IntegerValue( dynamic_cast<IntegerValue*>(left)->value / this->value );
				}

				throw except::incompatible_types();
				return 0;
			}
			virtual Cell* mod(Cell* left) {
				if(left->tag == integer) {
					if(this->value == 0) throw except::division_by_zero();
					return new IntegerValue( dynamic_cast<IntegerValue*>(left)->value % this->value );
				}

				throw except::incompatible_types();
				return 0;
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
			std::cout << "pushed ";
			cell->print(std::cout);
		}

		const Cell* Stack::var_at(size_t loc) const {
			return stack.at(loc);
		}

		void Stack::set_var_at(size_t loc, Cell* cell) {
			delete stack[loc];
			stack[loc] = cell;
			std::cout << "into "<< loc << " stored " << std::endl;
			stack[loc]->print(std::cout);
		}

		void Stack::reserve(size_t var_num) {
			stack.resize(stack.size()+var_num, 0);
			std::cout << "reserving " << var_num << " spaces, total " << stack.size() << std::endl;
		}

	}


	bool Interpreter::register_subprogram(const bytecode::subprogram& prog) {
		// do we already have a program with this name?
		std::map<std::string, program_id>::iterator it = program_map.find(prog.get_name());
		if(it != program_map.end()) return false;

		// if not, let's register it.
		program_map.insert( make_pair( prog.get_name(), programs.size() ) );
		programs.push_back(prog);

		return true;
	}


	void Interpreter::execute(const std::string& program) const {
		using namespace stack_machine;
		using namespace bytecode;

		program_id prog_id = program_map.find(program) -> second;
		Stack stack;
		size_t pc=0;	//program counter.
		Cell* r1;
		Cell* r2;

		while(pc < programs[prog_id].len) {
			byte opcode = programs[prog_id].code[pc];
			++pc;

			std::cout << "#" << pc << " Opcode: " << (int)opcode <<std::endl;
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
					std::cout << "jumping to " << pc <<std::endl;
					break;
				case JMPTRUE:  //41
					r1 = stack.pop();
					if(r1->tag == boolean && dynamic_cast<BooleanValue*>(r1)->value == true) {
						pc = programs[prog_id].get_int(pc);
						std::cout << "jumping to " << pc <<std::endl;
					} else programs[prog_id].get_int(pc);	//mindenképp be kell olvasni
					delete r1;
					break;
				case JMPFALSE: //42
					r1 = stack.pop();
					if(r1->tag == boolean && dynamic_cast<BooleanValue*>(r1)->value == false) {
						pc = programs[prog_id].get_int(pc);
						std::cout << "jumping to " << pc <<std::endl;
					} else programs[prog_id].get_int(pc);
					delete r1;
					break;

				// comparisons
				case LESS:    //31
					r1=stack.pop();
					r2=stack.pop();
					stack.push ( r1 -> less( r2 ) );
					delete r1;
					delete r2;
					break;
				case GREATER: //32
					r1=stack.pop();
					r2=stack.pop();
					stack.push ( r1 -> greater( r2 ) );
					delete r1;
					delete r2;
					break;
				case EQ:      //33
					r1=stack.pop();
					r2=stack.pop();
					stack.push ( r1 -> eq( r2 ) );
					delete r1;
					delete r2;
					break;
				case NEQ:     //34
					r1=stack.pop();
					r2=stack.pop();
					stack.push (  r1-> neq( r2 ) );
					delete r1;
					delete r2;
					break;

				// operations
				case ADDI:    //60
					r1=stack.pop();
					r2=stack.pop();
					stack.push ( r1 -> add( r2 ) );
					delete r1;
					delete r2;
					break;
				case SUBI:    //61
					r1=stack.pop();
					r2=stack.pop();
					stack.push ( r1 -> sub( r2 ) );
					delete r1;
					delete r2;
					break;
				case MULI:    //62
					r1=stack.pop();
					r2=stack.pop();
					stack.push ( r1 -> mul( r2 ) );
					delete r1;
					delete r2;
					break;
				case DIVI:    //63
					r1=stack.pop();
					r2=stack.pop();
					stack.push ( r1 -> div( r2 ) );
					delete r1;
					delete r2;
					break;
				case MODI:    //64
					r1=stack.pop();
					r2=stack.pop();
					stack.push ( r1 -> mod( r2 ) );
					delete r1;
					delete r2;
					break;
				case AND:     //80
					r1=stack.pop();
					r2=stack.pop();
					stack.push ( r1 -> land( r2 ) );
					delete r1;
					delete r2;
					break;
				case OR:      //81
					r1=stack.pop();
					r2=stack.pop();
					stack.push ( r1 -> lor( r2 ) );
					delete r1;
					delete r2;
					break;
				case NOT:     //81
					r1=stack.pop();
					stack.push ( r1 -> lnot() );
					delete r1;
					break;

				default:
					std::cerr << "unknown opcode " << opcode << std::endl;
					break;
			} //switch
		} // while
		stack.var_at(0)->print(std::cout);
	} // Interpreter::execute
}
