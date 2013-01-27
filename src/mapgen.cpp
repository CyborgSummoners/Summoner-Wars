#include "mapgen.hpp"
#include <iostream>
#include <queue>
#include <vector>
#include <limits>

void sum::Mapgen::print_map(Terrain* map, size_t width, size_t height, std::ostream& out) {
	for(size_t i=0; i<height; ++i) {
		for(size_t k=0; k<width; ++k) {
			out << ( map[i*width + k] == wall? '#' : '.' );
		}
		out << std::endl;
	}
}


void sum::Mapgen::Arena::generate(Terrain*& map, size_t width, size_t height) const {
	for(size_t k=0; k<width; ++k) {
		map[k] = wall;
	}
	for(size_t i=1; i<height-1; ++i) {
		map[i*width] = map[i*width+width-1] = wall;
		for(size_t k=1; k<width-1; ++k) {
			map[i*width + k] = floor;
		}
	}
	for(size_t k=0; k<width; ++k) {
		map[(height-1)*width+k] = wall;
	}
}

namespace sum {
namespace Mapgen {
namespace {

	struct node {
		size_t x;
		size_t y;
		node() : x(0), y(0) {}
		node(size_t x, size_t y) : x(x), y(y) {}
	};

	void floodfill(Terrain* map, int* areas, size_t width, size_t height, int area, size_t node_x, size_t node_y) {
		std::queue<node> q;
		q.push( node(node_x, node_y) );
		node n;
		while(!q.empty()) {
			n = q.front();
			q.pop();

			if( map[ n.y*width + n.x ] != wall && areas[ n.y*width + n.x ] == 0 ) {
				areas[ n.y*width + n.x ] = area;
				if(n.x>0)        { q.push( node(n.x-1, n.y) ); }  	// west
				if(n.x<width-1)  { q.push( node(n.x+1, n.y) ); } 	// east
				if(n.y>0)        { q.push( node(n.x, n.y-1) ); } 	// north
				if(n.y<height-1) { q.push( node(n.x, n.y+1) ); } 	// south
			}
		}
	}
}
}
}


void sum::Mapgen::Caves::generate(Terrain*& map, size_t width, size_t height) const {
	Terrain* map_new = new Terrain[width*height];
	Terrain* t = 0;

	// initialization: like arena
	for(size_t k=0; k<width; ++k) {
		map[k] = map_new[k] = map[(height-1)*width+k] = map_new[(height-1)*width+k] = wall;
	}
	for(size_t i=1; i<height-1; ++i) {
		map[i*width] = map[i*width+width-1] = map_new[i*width] = map_new[i*width+width-1] = wall;
		for(size_t k=1; k<width-1; ++k) {
			if(sf::Randomizer::Random(1,100) < 40) map[i*width + k] = wall;
			else map[i*width + k] = floor;

			map_new[i*width + k] = floor;
		}
	}

	for(unsigned short cycle = 0; cycle < cycles; ++cycle) {
		// for each node
		for(size_t i=1; i<height-1; ++i) {
			for(size_t k=1; k<width-1; ++k) {
				// if the area around the node contains > 5 walls (including itself),
				size_t wall_num = 0;
				for(int ii=-1; ii<=1; ++ii) {
					for(int kk=-1; kk<=1; ++kk) {
						if(map[(i+ii)*width + k + kk] == wall) wall_num++;
					}
				}

				// it will live:
				if(wall_num >= 5) map_new[i*width + k] = wall;
				else map_new[i*width + k] = floor;
			}
		}

		t = map;
		map = map_new;
		map_new = t;
	}

	delete[] map_new;

	int* areas = new int[width*height];
	for(size_t k=0; k<width*height; ++k) {
		if(map[k] == wall) areas[k] = -1;
		else areas[k] = 0;
	}
	int area_num = 0;

	for(size_t i=0; i<height; ++i) {
		for(size_t k=0; k<width; ++k) {
			if(areas[i*width + k] == 0) {
				++area_num;
				floodfill(map, areas, width, height, area_num, k, i);
			}
		}
	}

	if(area_num < 2) {
		delete[] areas;
		return;
	}

	// handling disjoint areas
	// naively: always connects segments to segment #1. though it guarantees connectedness all the same, it may look ugly.
	int* distmap = new int[height*width];
	int* prntmap = new int[height*width];
	int src_area = 1;

	int x = -1;
	int y = -1;
	for(size_t k=0; k<width*height; ++k) {
		distmap[k] = std::numeric_limits<int>::max();
		prntmap[k] = -1;
		if(areas[k] == src_area) {
			if(x < 0) {
				x = k % width;
				y = k / width;
			}
		}
	}

	std::queue<node> q;
	q.push( node(x, y) );
	node n;

	while(!q.empty()) {
		n = q.front();
		q.pop();

		if(areas[n.y*width + n.x] == src_area) {
			distmap[n.y*width + n.x] = 0;
		}

		if(n.x>0        && distmap[n.y*width + n.x-1]  > distmap[n.y*width + n.x]+1) { distmap[n.y*width + n.x-1]  = distmap[n.y*width + n.x]+1; prntmap[n.y*width + n.x-1] = n.y*width+n.x;   q.push( node(n.x-1, n.y) ); }  // west
		if(n.x<width-1  && distmap[n.y*width + n.x+1]  > distmap[n.y*width + n.x]+1) { distmap[n.y*width + n.x+1]  = distmap[n.y*width + n.x]+1; prntmap[n.y*width + n.x+1] = n.y*width+n.x;   q.push( node(n.x+1, n.y) ); }  // east
		if(n.y>0        && distmap[(n.y-1)*width + n.x]> distmap[n.y*width + n.x]+1) { distmap[(n.y-1)*width + n.x]= distmap[n.y*width + n.x]+1; prntmap[(n.y-1)*width + n.x] = n.y*width+n.x; q.push( node(n.x, n.y-1) ); }  // north
		if(n.y<height-1 && distmap[(n.y+1)*width + n.x]> distmap[n.y*width + n.x]+1) { distmap[(n.y+1)*width + n.x]= distmap[n.y*width + n.x]+1; prntmap[(n.y+1)*width + n.x] = n.y*width+n.x; q.push( node(n.x, n.y+1) ); }  // south
	}

	for(int ar=src_area+1; ar<=area_num; ++ar) {	//this isn't very clever.
		int min_dist = std::numeric_limits<int>::max();
		int min_dist_tile;

		for(size_t k=0; k<width*height; ++k) {
			if(areas[k] == ar && distmap[k]<min_dist) {
				min_dist_tile = k;
				min_dist = distmap[k];
			}
		}

		int p = min_dist_tile;
		while(areas[p] != 1) {
			map[p] = floor;
			p = prntmap[p];
		}
	}

	//std::cout << "zZz @^^. zZz " << std::endl;
	delete[] distmap;
	delete[] prntmap;
	delete[] areas;
}
