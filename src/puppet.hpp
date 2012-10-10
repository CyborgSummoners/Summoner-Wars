#ifndef PUPPET_HPP
#define PUPPET_HPP 1

#include <iostream>
#include <string>

namespace sum {

	class Puppet {
		size_t x, y;
		std::string name;

		public:
			Puppet(const std::string& name) : x(0), y(0), name(name) {};

			void move() {
				++x;
				std::cout << "I move forward to (" << x << "," << y << ")" << std::endl;
			}

			std::string get_name() const {
				return name;
			}

			bool operator==(const Puppet& other) const {
				return (this == &other);
			}
	};

}

#endif
