#ifndef DEBUG_MACROS_HPP
#define DEBUG_MACROS_HPP

#include <iostream>

#ifdef DEBUG_MACROS_ENABLED
	#define debug_var(var) std::cerr << (#var) << ": " << var << std::endl;
	static std::ostream& dout = std::cerr;

#else
	class debug_dummy : public std::ostream {
		public:
			template<class T>
			debug_dummy& operator<<(const T&) {
				return *this;
			}
			debug_dummy& operator<<(std::ostream& ( *pf )(std::ostream&)) {
				return *this;
			}
			debug_dummy& operator<<(std::ios& ( *pf )(std::ios&)) {
				return *this;
			}
			debug_dummy& operator<<(std::ios_base& ( *pf )(std::ios_base&)) {
				return *this;
			}
	};
	static debug_dummy dout;
	#define debug_var(var)
#endif


#endif
