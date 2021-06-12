#include "make_valid.hpp"

#include <iostream>
#include <boost/format.hpp>

namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point;
typedef bg::model::polygon<point> polygon;
typedef bg::model::multi_polygon<polygon> multi_polygon;

void correct_from_string(std::string const &input)
{
	std::cout << "Dissolve polygon: " << input << std::endl;

	polygon poly;
	boost::geometry::read_wkt(input, poly);

	if(boost::geometry::is_valid(poly))
		std::cout << "Input polygon is valid" << std::endl;
	else
		std::cout << "Input polygon is not valid" << std::endl;

	// Minimum area for sub polygon
	double remove_spike_threshold = 1E-12;

	multi_polygon result;
	geometry::correct(poly, result, remove_spike_threshold);

	std::cout << "Total area: " << boost::geometry::area(result) << std::endl;
	std::cout << boost::geometry::wkt(result) << std::endl;

	for(auto const &poly: result) {
		std::cout << "Polygon area: " << boost::geometry::area(poly) << std::endl;
		std::cout << boost::geometry::wkt(poly) << std::endl;
	}

	if(boost::geometry::is_valid(result))
		std::cout << "Output polygon is valid" << std::endl;
	else
		std::cout << "Output polygon is not valid" << std::endl;

	std::cout << std::endl;
}

int main()
{
	// Dissolve pentagram
	correct_from_string("POLYGON((5 0, 2.5 9, 9.5 3.5, 0.5 3.5, 7.5 9, 5 0))");

	// Dissolve mote complex example
	correct_from_string("POLYGON((55 10, 141 237, 249 23, 21 171, 252 169, 24 89, 266 73, 55 10))");

	// Multiple intersections
	correct_from_string("POLYGON((0 0, 10 0, 0 10, 10 10, 0 0, 5 0, 5 10, 0 10, 0 5, 10 5, 10 0, 0 0))");
}
