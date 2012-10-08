#ifndef PUPPET_HPP
#define PUPPET_HPP 1

#include <iostream>
#include <string>

namespace sum {

	class Puppet {
		std::string name;

		public:
			Puppet(const std::string& name) : name(name) {};

			void move() {
				std::cout << "I move forward." << std::endl;
			}

			bool operator==(const Puppet& other) const {
				return (this == &other);
			}
	};

}

#endif
