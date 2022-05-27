#include "correct.hpp"

#include <iostream>
#include <boost/format.hpp>
#include <fstream>
#include <chrono>

namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point;
typedef bg::model::polygon<point> polygon;
typedef bg::model::multi_polygon<polygon> multi_polygon;

enum CombineType {
	CombineNonZeroWinding,
	CombineOddEven
};

template <typename T>
void write_svg(std::string const& name, T const& input, T const& result)
{
  using point = typename boost::geometry::point_type<T>::type;

  std::string const filename = name + ".svg";
  std::ofstream svg(filename.c_str());

  boost::geometry::svg_mapper<point> mapper(svg, 400, 400);

  mapper.add(input);
  mapper.add(result);

  // Original, light yellow
  mapper.map(input, "fill-opacity:0.3;fill:rgb(255,255,192);stroke:rgb(255,255,0);" "stroke-width:1");
  // Fixed, blue on top
  mapper.map(result, "fill-opacity:0.6;fill:rgb(0,0,255);stroke:rgb(0,0,128);" "stroke-width:2");
}

void correct_from_string(std::string const &name, std::string const &input, CombineType type = CombineNonZeroWinding)
{
	std::cout << "Dissolve polygon: " << input << std::endl;

	multi_polygon poly;
	boost::geometry::read_wkt(input, poly);

	if(boost::geometry::is_valid(poly))
		std::cout << "Input polygon is valid" << std::endl;
	else
		std::cout << "Input polygon is not valid" << std::endl;

	// Minimum area for sub polygon
	double remove_spike_threshold = 1E-12;

	multi_polygon result;
	if(type == CombineNonZeroWinding)
		geometry::correct(poly, result, remove_spike_threshold);
	else if(type == CombineOddEven)
		geometry::correct_odd_even(poly, result, remove_spike_threshold);

	write_svg(name, poly, result);

	std::cout << "Total area: " << boost::geometry::area(result) << std::endl;
	std::cout << boost::geometry::wkt(result) << std::endl;

	for(auto const &poly: result) {
		std::cout << "Polygon area: " << boost::geometry::area(poly) << std::endl;
		std::cout << boost::geometry::wkt(poly) << std::endl;
	}

	std::string message;
	if(boost::geometry::is_valid(result, message))
		std::cout << "Output polygon is valid " << std::endl;
	else
		std::cout << "Output polygon is not valid " << message << std::endl;

	std::cout << std::endl;
}

void benchmark(CombineType type)
{
	// Benchmark overlapping approach
	multi_polygon poly;
	boost::geometry::read_wkt("MULTIPOLYGON (((10 50, 80 50, 80 70, 40 70, 40 30, 30 30, 30 80, 90 80, 90 40, 20 40, 20 20, 50 20, 50 90, 60 90, 60 10, 10 10, 10 50)))", poly);

  	auto const start = std::chrono::steady_clock::now();

	constexpr std::size_t count = 10000;
	for(std::size_t i = 0; i < count; ++i) {
		// Minimum area for sub polygon
		double remove_spike_threshold = 1E-12;

		multi_polygon result;
		if(type == CombineNonZeroWinding)
			geometry::correct(poly, result, remove_spike_threshold);
		else if(type == CombineOddEven)
			geometry::correct_odd_even(poly, result, remove_spike_threshold);
	}

  	auto const end = std::chrono::steady_clock::now();
  	auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	if(type == CombineNonZeroWinding)
		std::cout << "time: ";
	else if(type == CombineOddEven)
		std::cout << "time o/e: ";

  	std::cout << (double)ms / count << std::endl;
}

int main()
{
	//correct_from_string("multiple", "MULTIPOLYGON (((100 100, 100 300, 200 300, 200 130, 130 130, 130 250, 200 250, 200 160, 150 160, 150 220, 240 220, 240 160, 200 160, 200 250, 270 250, 270 130, 200 130, 200 300, 300 300, 300 100, 100 100)))", CombineOddEven);

	// Dissolve pentagram
	correct_from_string("pentagram", "MULTIPOLYGON (((5 0, 2.5 9, 9.5 3.5, 0.5 3.5, 7.5 9, 5 0)))", CombineNonZeroWinding);
	correct_from_string("pentagram_o_e", "MULTIPOLYGON (((5 0, 2.5 9, 9.5 3.5, 0.5 3.5, 7.5 9, 5 0)))", CombineOddEven);

	// Dissolve mote complex example
	correct_from_string("complex", "MULTIPOLYGON (((55 10, 141 237, 249 23, 21 171, 252 169, 24 89, 266 73, 55 10)))");

	// Multiple intersections
	correct_from_string("multiple", "MULTIPOLYGON (((0 0, 10 0, 0 10, 10 10, 0 0, 5 0, 5 10, 0 10, 0 5, 10 5, 10 0, 0 0)))");

	// Overlapping region
	std::cout << "Overlapping region using non-zero winding: " << std::endl;
	correct_from_string("overlapping", "MULTIPOLYGON (((10 70, 90 70, 90 50, 30 50, 30 30, 50 30, 50 90, 70 90, 70 10, 10 10, 10 70)))");

	// Overlapping region (odd-even)
	std::cout << "Overlapping region using odd-even: " << std::endl;
	correct_from_string("overlapping_o_e", "MULTIPOLYGON (((10 70, 90 70, 90 50, 30 50, 30 30, 50 30, 50 90, 70 90, 70 10, 10 10, 10 70)))", CombineOddEven);

	benchmark(CombineNonZeroWinding);
	benchmark(CombineOddEven);
}

