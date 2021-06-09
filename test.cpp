#include "dissolve.hpp"

#include <iostream>
#include <boost/format.hpp>

#include <random>

namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point;
typedef bg::model::polygon<point> polygon;
typedef bg::model::multi_polygon<polygon> multi_polygon;

int main()
{
	std::default_random_engine generator;
  	std::uniform_real_distribution<double> distribution(0.0,1.0);

	for(std::size_t run = 0; run < 100000; ++run) {
		polygon poly;
		for(std::size_t i = 0; i < (unsigned int)(5 + distribution(generator) * 20); ++i) {
			poly.outer().push_back( { distribution(generator), distribution(generator) } );
		}
		poly.outer().push_back( poly.outer().front() );

		double remove_spike_threshold = 1E-12;

		multi_polygon result;
		dissolve::dissolve(poly, result, remove_spike_threshold);

		if(!boost::geometry::is_valid(result)) {
			if(boost::geometry::is_valid(poly))
				std::cout << "Input polygon is valid" << std::endl;
			else
				std::cout << "Input polygon is not valid" << std::endl;
			std::cout << boost::geometry::wkt(poly) << std::endl;

			std::cout << "Output polygon is not valid" << std::endl;
		}

		if(run % 500 == 0) std::cout << run << std::endl;
	}

	std::cout << "Done" << std::endl;
	return 0;
}

