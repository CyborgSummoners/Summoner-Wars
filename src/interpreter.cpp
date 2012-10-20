#include "interpreter.hpp"
#include "util/debug.hpp"
#include "bytecode.hpp"
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace sum {
	using namespace bytecode;

	namespace stack_machine {
		namespace except {
			struct corrupted_stack : public std::exception {};
			struct incompatible_types : public std::exception {};
			struct division_by_zero : public std::exception {};
			struct subprogram_does_not_exist : public std::exception {};
			struct index_out_of_range : public std::exception {};
		}

		struct Cell {
			type tag;
			virtual std::string to_str() const = 0;
			virtual Cell* clone() const = 0;
			virtual Cell* copy() const = 0;
			virtual ~Cell() {};
		};

		struct Reference : public Cell {
			virtual Reference* copy() const = 0;
			virtual ~Reference() {};
		};

		struct NullValue : public Cell {
			NullValue() {
				this->tag = none;
			}
			virtual std::string to_str() const {
				return "Null";
			}
			virtual NullValue* clone() const {
				return new NullValue();
			}
			virtual NullValue* copy() const {
				return new NullValue();
			}
		};

		struct PuppetValue : public Cell {
			Puppet& value;
			PuppetValue(Puppet& value) : value(value) {
				this->tag = puppet;
			}
			virtual std::string to_str() const {
				return "Puppet '" + value.get_name() + "'";
			}
			virtual PuppetValue* clone() const {
				return new PuppetValue(value);
			}
			virtual PuppetValue* copy() const {
				return new PuppetValue(value);
			}
		};

		struct BooleanValue : public Cell {
			const bool value;
			BooleanValue(bool val) : value(val) {
				this->tag = boolean;
			}
			virtual std::string to_str() const {
				return (this->value? "true" : "false");
			}
			virtual BooleanValue* clone() const {
				return new BooleanValue(this->value);
			}
			virtual BooleanValue* copy() const {
				return new BooleanValue(this->value);
			}
		};

		struct IntegerValue : public Cell {
			const int value;

			IntegerValue(int val) : value(val) {
				this->tag = integer;
			}
			virtual std::string to_str() const {
				std::ostringstream n;
				n << value;
				return n.str();
			}
			virtual IntegerValue* clone() const {
				return new IntegerValue(this->value);
			}
			virtual IntegerValue* copy() const {
				return new IntegerValue(this->value);
			}
		};

		struct ListValue {
			size_t len;
			std::vector<Cell*> value;
			size_t refcount;

			ListValue(const std::vector<Cell*>& val) : len(val.size()), value(val), refcount(0) {}
			ListValue(Cell** cells, size_t len) : len(len), value(len), refcount(0) {
				for(size_t i=0; i<len; ++i) value[i] = cells[i];
			}

			virtual std::string to_str() const {
				std::ostringstream s;
				s << "[";
				for(size_t i=0; i<value.size(); ++i) {
					if(i!=0) s << ",";
					s << value[i]->to_str();
				}
				s << "]";
				return s.str();
			}
			virtual ListValue* clone() const {
				Cell** ncells = new Cell*[len];
				for(size_t i=0; i<len; ++i) {
					ncells[i] = value[i]->clone();
				}
				ListValue* result = new ListValue(ncells, len);
				delete[] ncells;
				return result;
			}

			virtual ~ListValue() {
				for(size_t i=0; i<value.size(); ++i) delete value[i];
			}

			void addref() {
				++refcount;
			}
			void delref() {
				--refcount;
				if(refcount == 0) delete this;
			}
		};

		struct ListRef : public Reference {
			ListValue* value;

			ListRef(ListValue* value) : value(value) {
				this->tag = list;
				value->addref();
			}
			virtual std::string to_str() const {
				return value->to_str();
			}
			virtual ListRef* clone() const {
				return new ListRef(value);
			}
			virtual ListRef* copy() const {
				return new ListRef(value->clone());
			}
			virtual void set_element(Cell* idx, Cell* elem) {
				int index;
				if(idx->tag == integer)
					index = static_cast<IntegerValue*>(idx)->value;
				else throw stack_machine::except::incompatible_types();

				if(index < 0) {
					index = value->len+index;
					if(index<0) throw except::index_out_of_range();
				}
				if(static_cast<size_t>(index) >= value->len) throw except::index_out_of_range();

				delete value->value[index];
				value->value[index] = elem;
				delete idx;
			}
			virtual const Cell* get_element(Cell* idx) const {
				int index;
				if(idx->tag == integer)
					index = static_cast<IntegerValue*>(idx)->value;
				else throw stack_machine::except::incompatible_types();

				if(index < 0) {
					index = value->len+index;
					if(index<0) throw except::index_out_of_range();
				}
				if(static_cast<size_t>(index) >= value->len) throw except::index_out_of_range();

				return value->value[index];
			}
			virtual ~ListRef() {
				value->delref();
			}
		};

		struct StringValue : public Cell {
			const std::string value;

			StringValue(std::string val) : value(val) {
				this->tag = string;
			}
			virtual std::string to_str() const {
				return value;
			}
			virtual StringValue* clone() const {
				return new StringValue(this->value);
			}
			virtual StringValue* copy() const {
				return new StringValue(this->value);
			}
		};

		struct ActivationRecord : public Cell {
			const size_t prog;
			const size_t pc;
			const size_t bp;

			ActivationRecord(size_t prog, size_t pc, size_t bp) : prog(prog), pc(pc), bp(bp) {
				this->tag = none;
			}
			virtual std::string to_str() const {
				std::ostringstream out;
				out << "Activation record\n" << " \tprog: " << prog << "\n\tpc: " << pc << "\n\tbp: " << bp;
				return out.str();
			}
			virtual ActivationRecord* clone() const {
				return new ActivationRecord(this->prog, this->pc, this->bp);
			}
			virtual ActivationRecord* copy() const {
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

				Cell* var_at(size_t loc) const;
				void set_var_at(size_t loc, Cell* cell);
				size_t get_stack_pointer() const;
				void set_stack_pointer(size_t sp);

				void reserve(size_t var_num);

				~Stack(){
					for(size_t i=0; i<stack.size();++i) {
						if(stack[i]) delete stack[i];
					}
				}

				void print(std::ostream& out) const;
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
		}

		Cell* Stack::var_at(size_t loc) const {
			return stack.at(loc);
		}

		void Stack::set_var_at(size_t loc, Cell* cell) {
			delete stack[loc];
			stack[loc] = cell;
		}

		size_t Stack::get_stack_pointer() const {
			return stack.size();
		}
		void Stack::set_stack_pointer(size_t sp) {
			if(sp > stack.size()) {
				stack.reserve(stack.size() - sp);
				return;
			}
			else if( sp == stack.size() ) return;

			for(size_t i=stack.size(); i-->sp; ) {
				delete stack[i];
			}
			stack.resize(sp);
		}

		void Stack::reserve(size_t num) {
			size_t currsiz = stack.size();
			stack.resize(stack.size()+num, 0);
			for(size_t i=currsiz; i<num; ++i) stack[i] = new NullValue();

			dout << "reserving " << num << " spaces, total " << stack.size() << std::endl;
		}

		void Stack::print(std::ostream& out) const {
			for(std::vector<Cell*>::const_reverse_iterator rit = stack.rbegin(); rit<stack.rend(); ++rit) out << (*rit)->to_str() << std::endl;
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
					else if( r1->tag == string && r2->tag == string ) {
						stack.push( new StringValue( static_cast<StringValue*>(r2)->value + static_cast<StringValue*>(r1)->value ) );
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

			// negation: logical negation and unary minus
			struct neg : public Interrupt  {
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
					else if( r1->tag == integer ) {
						stack.push( new IntegerValue( -(static_cast<IntegerValue*>(r1)->value) ) );
						done=true;
					}
					delete r1;
					if(!done) throw except::incompatible_types();
				}
			};

			//****************************
			//*** Built-in subroutines ***
			//****************************
			struct print : public Interrupt  {
				const char* get_name() const {
					return "PRINT";
				}
				void operator()(Stack& stack) const {
					Cell* r1 = stack.pop();
					std::cout << r1->to_str() << std::endl;
					delete r1;
				}
			};

			struct tostring : public Interrupt  {
				const char* get_name() const {
					return "STRING";
				}
				void operator()(Stack& stack) const {
					Cell* r1 = stack.pop();
					stack.push( new StringValue(r1->to_str()) );
					delete r1;
				}
			};

			//***************
			//*** Methods ***
			//***************
			struct self_move : public Interrupt  {
				const char* get_name() const {
					return "SELF::MOVE";
				}
				void operator()(Stack& stack) const {
					Cell* r1 = stack.pop(); //supposed to be self. need to check.
					if(r1->tag != puppet) throw except::incompatible_types();
					static_cast<PuppetValue*>(r1)->value.move();
					delete r1;
				}
			};
			struct self_turn_left : public Interrupt  {
				const char* get_name() const {
					return "SELF::TURN_LEFT";
				}
				void operator()(Stack& stack) const {
					Cell* r1 = stack.pop(); //supposed to be self. need to check.
					if(r1->tag != puppet) throw except::incompatible_types();
					static_cast<PuppetValue*>(r1)->value.turn_left();
					delete r1;
				}
			};
			struct self_turn_right : public Interrupt  {
				const char* get_name() const {
					return "SELF::TURN_RIGHT";
				}
				void operator()(Stack& stack) const {
					Cell* r1 = stack.pop(); //supposed to be self. need to check.
					if(r1->tag != puppet) throw except::incompatible_types();
					static_cast<PuppetValue*>(r1)->value.turn_right();
					delete r1;
				}
			};

			const std::vector<Interrupt*> list_init() {
				std::vector<Interrupt*> Result;
				Result.push_back( new interrupt::print() );
				Result.push_back( new interrupt::tostring() );
				Result.push_back( new interrupt::self_move() );
				Result.push_back( new interrupt::self_turn_left() );
				Result.push_back( new interrupt::self_turn_right() );
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
			new interrupt::neg()
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
	Interpreter::puppet_brain::puppet_brain(Puppet& puppet) : puppet(puppet), program(0), program_counter(0), base_pointer(0), delay(0) {
		stack = new stack_machine::Stack();
	}
	Interpreter::puppet_brain::~puppet_brain() {
		delete stack;
	}

	Interpreter::Interpreter() {
		// default behaviour: delay 100;
		bytecode::byte* code = new bytecode::byte[6];
		code[0] = bytecode::NOP;
		code[1] = bytecode::DELAY;
		code[2] = code[3] = code[4] = 0;
		code[5] = 100;
		programs.push_back(bytecode::subprogram("NOP", 0, code, 5, false));
	}

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


	bool Interpreter::step(unsigned int ticks) {
		//find the next puppet with a delay < ticks
		std::list<puppet_brain*>::iterator it = puppets.begin();
		if( it==puppets.end() || (*it)->delay > ticks ) return false;

		puppet_brain* puppet = *it;

//		while(puppet->delay < ticks) {
			if(puppet->program_counter >= programs[ puppet->program ].len) puppet->program_counter=0;
			execute_instruction(puppet->puppet, puppet->program, *(puppet->stack), puppet->program_counter, puppet->base_pointer);
//		}

		return true;
	}


	void Interpreter::execute(const std::string& program) const {
		Puppet p("Hamis Baba");
		size_t prog_id = get_program_id(program);
		stack_machine::Stack stack;
		size_t pc=0;
		size_t bp=0;

		while(pc < programs[prog_id].len) {
			execute_instruction(p, prog_id, stack, pc, bp);
		}
	}


	size_t Interpreter::execute_instruction(Puppet& self, size_t& program_id, stack_machine::Stack& stack, size_t& pc, size_t& bp) const {
		using namespace stack_machine;
		using namespace bytecode;

		// segédregiszterek (kiemelni objektumváltozókká?)
		Cell* r1;
		Cell* r2;
		Cell* retval;
		Cell** rarr;
		ActivationRecord* ar;
		size_t rs;
		int ri, rj;

		byte opcode = programs[program_id].get_byte(pc);
		switch(opcode) {
			case NOP:	// 0
				break;
			// stack
			case PUSH:    // 1
				stack.push(new IntegerValue( programs[program_id].get_int(pc) ));
				break;
			case PSHB:    // 2
				stack.push(new BooleanValue( programs[program_id].get_byte(pc) ));
				break;
			case PSHS:    // 3
				stack.push(new StringValue( programs[program_id].get_string(pc) ));
				break;
			case PUSH_SELF:    // 4
				stack.push( new PuppetValue(self) );
				break;
			case COPY:
				r1 = stack.pop();
				stack.push( r1->copy() );
				delete r1;
				break;
			case RSRV:     //10
				stack.reserve( programs[program_id].get_byte(pc) );
				break;
			case FETCH_X: //23
				ri = programs[program_id].get_byte(pc) - programs[program_id].get_argc();
				if(ri < 0) ri= -(programs[program_id].get_argc()+ri) - 2; // ez kicsit gázos
				stack.push( stack.var_at(bp + ri)->clone() );
				break;
			case STORE_X: //23
				r1 = stack.pop();
				ri = programs[program_id].get_byte(pc) - programs[program_id].get_argc();
				if(ri < 0) ri= -(programs[program_id].get_argc()+ri) - 2;
				stack.set_var_at(bp + ri, r1);
				break;
			// control flow
			case JMP:      //40
				pc = programs[program_id].get_int(pc);
				dout << "jumping to " << pc <<std::endl;
				break;
			case JMPTRUE:  //41
				r1 = stack.pop();
				if(r1->tag == boolean && dynamic_cast<BooleanValue*>(r1)->value == true) {
					pc = programs[program_id].get_int(pc);
					dout << "jumping to " << pc <<std::endl;
				} else programs[program_id].get_int(pc);	//mindenképp be kell olvasni
				delete r1;
				break;
			case JMPFALSE: //42
				r1 = stack.pop();
				if(r1->tag == boolean && dynamic_cast<BooleanValue*>(r1)->value == false) {
					pc = programs[program_id].get_int(pc);
					dout << "jumping to " << pc <<std::endl;
				} else programs[program_id].get_int(pc);
				delete r1;
				break;
			case CALL:      //43
				rs = get_program_id( programs[program_id].get_string(pc) );	// get callee's id.
				// parameters are on the top of the stack. callee knows how many

				// return to which program, at which point, and what was the base pointer?
				stack.push( new ActivationRecord(program_id, pc, bp) );

				bp = stack.get_stack_pointer(); // new 0 is the top of the stack.
				program_id = rs; // jump to new program
				pc = 0; // start at the beginning.
				break;
			case RET:		//44
				if(bp == 0) break;	// ez kicsit hack.
				stack.set_stack_pointer(bp); // rewind the stack all the way down to 0, to before the local vars
				r1 = stack.pop(); // get back the activation record.
				if((ar = dynamic_cast<ActivationRecord*>(r1))==0) throw stack_machine::except::corrupted_stack();
				stack.set_stack_pointer(stack.get_stack_pointer() - programs[program_id].get_argc());	// consume the arguments, if any.
				bp = ar->bp;
				pc = ar->pc;
				program_id = ar->prog;
				delete ar;
				break;
			case RETV:      //45
				if(bp == 0) break; // igen, ezt majd a kezdeti activation recorddal.
				retval = stack.pop();

				stack.set_stack_pointer(bp); // rewind the stack
				r1 = stack.pop(); // get back the activation record.
				if((ar = dynamic_cast<ActivationRecord*>(r1))==0) throw stack_machine::except::corrupted_stack();
				stack.set_stack_pointer(stack.get_stack_pointer() - programs[program_id].get_argc());	// consume the arguments, if any.
				bp = ar->bp;
				pc = ar->pc;
				program_id = ar->prog;
				delete ar;

				stack.push(retval);
				break;
			case INTERRUPT: //46
				rs = programs[program_id].get_int(pc);
				if(rs < Interrupt::list.size()) {
					(*Interrupt::list[rs])(stack);
				}
				break;
			case APPLY: //47
				r1 = stack.pop();
				rs = get_program_id( subprogram::normalize_name(r1->to_str()) );
				stack.push( new ActivationRecord(program_id, pc, bp) );
				bp = stack.get_stack_pointer();
				program_id = rs;
				pc = 0; // start at the beginning.
				delete r1;
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
			case NEG:
				(*Interrupt::operators[opcode - ADDI])(stack);
				break;
			case LIST:
				ri = programs[program_id].get_int(pc);
				rarr = new Cell*[ ri ];
				rj = 0;
				while(rj < ri) rarr[rj++] = stack.pop();
				stack.push( new ListRef(new ListValue(rarr, ri)) );
				delete[] rarr;
				break;
			case FETCH_IDX:
				r1 = stack.pop();
				if(r1->tag == list) {
					stack.push( static_cast<ListRef*>(r1)->get_element( stack.pop() )->clone() );
				} else throw stack_machine::except::incompatible_types();
				break;
			case STORE_IDX:
				r1 = stack.pop(); //listref
				if(r1->tag == list) {
					r2 = stack.pop(); //index
					static_cast<ListRef*>( r1 )->set_element(r2, stack.pop());
				} else throw stack_machine::except::incompatible_types();
				break;
			case DELAY:
				return programs[program_id].get_int(pc);
			default:
				std::cerr << "unknown opcode " << (int)opcode << std::endl;
				break;
		}
		return 100;
	}

	bool Interpreter::register_puppet(Puppet& puppet) {
		for(std::list<puppet_brain*>::iterator it = puppets.begin(); it!=puppets.end(); ++it) {
			if( (*it)->puppet == puppet) return false;
		}
		puppets.push_front( new puppet_brain(puppet) );
		return true;
	}
	bool Interpreter::unregister_puppet(Puppet& puppet) {
		return true;
	}
	bool Interpreter::set_behaviour(Puppet& puppet, const std::string& behaviour) {
		try {
			for(std::list<puppet_brain*>::iterator it = puppets.begin(); it!=puppets.end(); ++it) {
				if( (*it)->puppet == puppet ) {
					(*it)->overrides[0] = get_program_id(behaviour);
					if( (*it)->program == 0 )  (*it)->program = (*it)->overrides[0];
					return true;
				}
			}
		} catch(stack_machine::except::subprogram_does_not_exist& e) {
			return false;
		}
		return false;
	}

	std::string Interpreter::get_behaviour(Puppet& puppet) const {
		return "Hát, valami";
	}
}
