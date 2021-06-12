#include "correct.hpp"

#include <iostream>
#include <boost/format.hpp>

#include <random>

namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point;
typedef bg::model::polygon<point> polygon;
typedef bg::model::multi_polygon<polygon> multi_polygon;

void random_test() 
{
	std::default_random_engine generator;
  	std::uniform_real_distribution<double> distribution(0.0,1.0);

	for(std::size_t run = 0; run < 1000; ++run) {
		polygon poly;
		for(std::size_t i = 0; i < (unsigned int)(5 + distribution(generator) * 20); ++i) {
			poly.outer().push_back( { distribution(generator), distribution(generator) } );
		}
		poly.outer().push_back( poly.outer().front() );

		double remove_spike_threshold = 1E-12;

		multi_polygon result;
		geometry::correct(poly, result, remove_spike_threshold);

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
}

template<typename T = polygon>
void correct_from_string(std::string const &input)
{
	std::cout << "Correct polygon: " << input << std::endl;

	T poly;
	boost::geometry::read_wkt(input, poly);

	if(boost::geometry::is_valid(poly))
		std::cout << "Input polygon is valid" << std::endl;
	else
		std::cout << "Input polygon is not valid" << std::endl;

	// Minimum area for sub polygon
	double remove_spike_threshold = 1E-12;

	multi_polygon result;
	geometry::correct(poly, result, remove_spike_threshold);

	std::cout << boost::geometry::wkt(result) << std::endl;

	std::string message;
	if(boost::geometry::is_valid(result, message))
		std::cout << "Output polygon is valid" << std::endl;
	else
		std::cout << "Output polygon is not valid: " << message << std::endl;

	std::cout << std::endl;
}

void test_cases()
{

	// Invalid OGC geometries
	// https://stackoverflow.com/questions/49902090/dataset-of-invalid-geometries-in-boostgeometry
	//
	// Hole Outside Shell
	correct_from_string("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0), (15 15, 15 20, 20 20, 20 15, 15 15))");

	// Nested holes
	correct_from_string("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0), (2 2, 2 8, 8 8, 8 2, 2 2), (3 3, 3 7, 7 7, 7 3, 3 3))");

	// Disconnected Interior
	correct_from_string("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0), (5 0, 10 5, 5 10, 0 5, 5 0))");
	//
	
	//Self-Intersection 
	correct_from_string("POLYGON((0 0, 10 10, 0 10, 10 0, 0 0))");

	// Ring Self-Intersection
	correct_from_string("POLYGON((5 0, 10 0, 10 10, 0 10, 0 0, 5 0, 3 3, 5 6, 7 3, 5 0))");

	// Nested shells
	correct_from_string<multi_polygon>("MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0)),(( 2 2, 8 2, 8 8, 2 8, 2 2)))");

	// Duplicated rings
	correct_from_string<multi_polygon>("MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0)),((0 0, 10 0, 10 10, 0 10, 0 0)))");

	// Too few points
	correct_from_string("POLYGON((2 2, 8 2))");

	// Ring not closed
	correct_from_string("POLYGON((0 0, 0 10, 10 10, 10 0))");

	// Invalid coordinate
	correct_from_string("POLYGON((NaN 3, 3 4, 4 4, 4 3, 3 3))");
}

int main()
{
	test_cases();
	random_test();
	return 0;
}

