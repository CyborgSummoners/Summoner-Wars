#ifndef PUPPET_HPP
#define PUPPET_HPP 1

#include <iostream>
#include <string>

namespace sum {

	class Puppet {
		size_t direction;	// 0 North, 1 ne, 2 e, 3 se, 4 s, 5 sw, 6 w, 7 nw. Should be an enum, I guess.
		int x, y;
		std::string name;

		public:
			Puppet(const std::string& name) : direction(0), x(0), y(0), name(name) {};

			unsigned int turn_left() {
				direction = (direction-1) % 8;
				std::cout << "Puppet '"<< name <<"': " << "I turned to the left, now facing " << direction << std::endl;
				return 20;
			}
			unsigned int turn_right() {
				direction = (direction+1) % 8;
				std::cout << "Puppet '"<< name <<"': " << "I turned to the right, now facing " << direction << std::endl;
				return 20;
			}

			unsigned int move() {
				int xchange=0, ychange=0;
				if(direction<2 || direction>6) ychange = 1; else if(direction>2 && direction<6) ychange = -1;
				if(direction<4 && direction>0) xchange = 1; else if(direction>4) xchange = -1;
				y+=ychange;
				x+=xchange;
				std::cout << "Puppet '"<< name <<"': " << "I move forward to (" << x << "," << y << ")" << std::endl;
				return 50;
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
