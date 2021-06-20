#define BOOST_GEOMETRY_NO_ROBUSTNESS
#include <iostream>
#include "correct.hpp"

#include <boost/format.hpp>

#include <random>
#include <chrono>
#include <fstream>

#include "data/CLC2006_180927.wkt.cpp"

namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point;
typedef bg::model::polygon<point> polygon;
typedef bg::model::multi_polygon<polygon> multi_polygon;

enum CombineType {
	CombineNonZeroWinding,
	CombineOddEven
};

void write_svg(std::string const& name, multi_polygon const &input, multi_polygon const& result)
{
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

void write_svg(std::string const& name, polygon const &input, multi_polygon const& result)
{
	multi_polygon mp;
	mp.push_back(input);
	write_svg(name, mp, result);
}


template <typename T>
void generate_from_string(std::string const &name, std::string const &input, CombineType type)
{
	T poly;
	boost::geometry::read_wkt(input, poly);

	// Minimum area for sub polygon
	double remove_spike_threshold = 1E-12;

	multi_polygon result;
	if(type == CombineNonZeroWinding)
		geometry::correct(poly, result, remove_spike_threshold);
	else if(type == CombineOddEven)
		geometry::correct_odd_even(poly, result, remove_spike_threshold);

	if(boost::geometry::is_valid(result))
		std::cout << name << " is valid" << std::endl;
	else
		std::cout << name << " is not valid" << std::endl;
	write_svg(name, poly, result);
}

template <typename T = polygon>
void generate_from_string(std::string const &name, std::string const &input)
{
	generate_from_string<T>(name + "_nzw", input, CombineNonZeroWinding);
	generate_from_string<T>(name + "_oe", input, CombineOddEven);
}

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

	// https://3d.bk.tudelft.nl/hledoux/blog/your-polygons-the-same/
			
	// Bowtie polygon
	correct_from_string("POLYGON((0 0, 0 10, 10 0, 10 10, 0 0))");

	// Square with wrong orientation
	correct_from_string("POLYGON((0 0, 0 10, 10 10, 10 0, 0 0))");

	// Inner ring with one edge sharing part of an edge of the outer ring
	correct_from_string("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(5 2,5 7,10 7, 10 2, 5 2))");

	// Dangling edge
	correct_from_string("POLYGON((0 0, 10 0, 15 5, 10 0, 10 10, 0 10, 0 0))");

	// Outer ring not closed
	correct_from_string("POLYGON((0 0, 10 0, 10 10, 0 10))");

	// Two adjacent inner rings
	correct_from_string("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 8, 3 8, 3 1, 1 1), (3 1, 3 8, 5 8, 5 1, 3 1))");

	// Polygon with inner ring inside inner ring
	correct_from_string("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0), (2 8, 5 8, 5 2, 2 2, 2 8), (3 3, 4 3, 3 4, 3 3))");
}

void data_test_cases()
{
	// https://github.com/hugoledoux/BIGpolygons
	polygon poly;
	boost::geometry::read_wkt(wkt_CLC2006_180927, poly);

	std::cout << "Outer: " << poly.outer().size() << ", inners: " << poly.inners().size() << std::endl;

	// Minimum area for sub polygon
	//double remove_spike_threshold = 1E-12;
	double remove_spike_threshold = 0;

	if(boost::geometry::is_valid(poly))
		std::cout << "Input is valid" << std::endl;
	else	
		std::cout << "Input is not valid" << std::endl;

	auto start_time =std::chrono::high_resolution_clock::now();

	multi_polygon result;
	geometry::correct(poly, result, remove_spike_threshold);

	auto stop_time = std::chrono::high_resolution_clock::now();

	std::cout << "Result polygons:  " << result.size() << ", outer: " << result[0].outer().size() << ", inners: " << result[0].inners().size() << std::endl;
	std::cout << std::chrono::duration_cast< std::chrono::milliseconds >(stop_time - start_time).count() << " ms " << std::endl;

	std::string message;
	if(boost::geometry::is_valid(result, message))
		std::cout << "Result is valid" << std::endl;
	else	
		std::cout << "Result is not valid: " << message << std::endl;
}

void jts_test_cases() 
{
	// https://github.com/locationtech/jts/blob/master/modules/tests/src/test/resources/testxml/misc/TestInvalidA.xml
	// JTS test cases
	
	// Polygon - Exverted shell, point touch
	correct_from_string("POLYGON ((10 90, 50 70, 90 90, 90 10, 50 70, 10 10, 10 90))");

	// Polygon - Exverted shell, point-line touch
	correct_from_string("POLYGON ((10 90, 90 90, 90 10, 50 90, 10 10, 10 90))");

	// Polygon - Exverted shell, line touch
	correct_from_string("POLYGON ((10 90, 90 90, 90 10, 60 90, 40 90, 10 10, 10 90))");

	// Polygon - Inverted shell, point touch
	correct_from_string("POLYGON ((10 90, 50 90, 30 50, 70 50, 50 90, 90 90, 90 10, 10 10, 10 90))");

	// Polygon - Inverted shell, point-line touch
	correct_from_string("POLYGON ((10 90, 40 90, 40 30, 80 30, 40 70, 90 70, 90 10, 10 10, 10 90))");

	// Polygon - Inverted shell, line touch, exterior
	correct_from_string("POLYGON ((10 90, 70 90, 40 90, 40 60, 70 60, 70 90, 90 90, 90 10, 10 10, 10 90))");

	// Polygon - Inverted shell, line touch, interior
	correct_from_string("POLYGON ((10 90, 50 90, 50 70, 30 40, 70 40, 50 70, 50 90, 90 90, 90 10, 10 10, 10 90))");

	// Polygon/Hole - Exverted hole, point touch
	correct_from_string("POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (20 80, 50 60, 80 80, 80 20, 50 60, 20 20, 20 80))");

	// Polygon/Hole - Exverted hole, point-line touch
	correct_from_string("POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (20 80, 80 80, 80 20, 50 80, 20 20, 20 80))");

	// Polygon/Hole - Exverted hole, line touch
	correct_from_string("POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (80 80, 60 50, 40 50, 20 80, 20 20, 40 50, 60 50, 80 20, 80 80))");

	// Polygon/Hole - Inverted hole, point touch
	correct_from_string("POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (20 80, 50 80, 30 50, 70 50, 50 80, 80 80, 80 20, 20 20, 20 80))");

	// Polygon/Hole - Inverted hole, point-line touch
	correct_from_string("POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (20 80, 40 80, 40 40, 70 40, 40 70, 80 70, 80 20, 20 20, 20 80))");

	// Polygon/Hole - Inverted hole, line touch, interior
	correct_from_string("POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (20 80, 60 80, 40 80, 40 40, 60 40, 60 80, 80 80, 80 20, 20 20, 20 80))");

	// Polygon/Hole - Inverted hole, line touch, exterior 
	correct_from_string("POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (50 80, 50 60, 40 40, 60 40, 50 60, 50 80, 80 30, 20 30, 50 80))");

	// Polygon/Hole - Exverted shell, point touch; exverted hole, point touch 
	correct_from_string("POLYGON ((10 10, 10 90, 50 50, 90 90, 90 10, 50 50, 10 10), (80 60, 50 50, 20 60, 20 40, 50 50, 80 40, 80 60))");

	// JTS document
	// https://docs.google.com/document/d/19YEQS0goSpZlwaYivS6ZpxJ5gRth2gdG6aY2XVd5fcc/edit
	// 
	correct_from_string("POLYGON ((10 70, 90 70, 90 50, 30 50, 30 30, 50 30, 50 90, 70 90, 70 10, 10 10, 10 70))");

	correct_from_string("POLYGON ((10 50, 80 50, 80 70, 40 70, 40 30, 30 30, 30 80, 90 80, 90 40, 20 40, 20 20, 50 20, 50 90, 60 90, 60 10, 10 10, 10 50))");
}

int main()
{
	test_cases();
	data_test_cases();
	random_test();
	jts_test_cases(); 

	generate_from_string("ref_figure_8", "POLYGON ((10 90, 90 10, 90 90, 10 10, 10 90))");
	generate_from_string("ref_self_cross", "POLYGON ((10 70, 90 70, 90 50, 30 50, 30 30, 50 30, 50 90, 70 90, 70 10, 10 10, 10 70))");
	generate_from_string("ref_self_overlap_multiple", "POLYGON ((10 50, 80 50, 80 70, 40 70, 40 30, 30 30, 30 80, 90 80, 90 40, 20 40, 20 20, 50 20, 50 90, 60 90, 60 10, 10 10, 10 50))");
	generate_from_string("ref_self_overlap", "POLYGON ((10 90, 50 90, 50 30, 70 30, 70 50, 30 50, 30 70, 90 70, 90 10, 10 10, 10 90))");
	generate_from_string("ref_pos_and_neg_winding", "POLYGON ((10 90, 90 90, 90 10, 60 10, 40 30, 70 60, 60 80, 40 60, 60 60, 40 80, 30 60, 60 30, 40 10, 10 10, 10 90))");
	generate_from_string("ref_double_pos_and_single_neg", "POLYGON ((10 90, 70 90, 70 10, 20 10, 20 80, 60 80, 60 20, 30 20, 30 70, 51 70, 50 30, 90 30, 90 60, 10 60, 10 90))");
	generate_from_string("ref_multiple_times_with_pos", "POLYGON ((100 100, 100 300, 200 300, 200 130, 130 130, 130 250, 200 250, 200 160, 150 160, 150 220, 240 220, 240 160, 200 160, 200 250, 270 250, 270 130, 200 130, 200 300, 300 300, 300 100, 100 100))");
	generate_from_string("ref_spiral_with_pos", "POLYGON ((60 10, 10 10, 10 90, 90 90, 90 20, 20 20, 20 80, 80 80, 80 30, 30 30, 30 70, 70 70, 70 40, 40 40, 40 60, 60 60, 60 10))");
	generate_from_string("ref_spiral_with_neg", "POLYGON ((0 100, 100 100, 100 0, 0 0, 0 100), (60 10, 60 60, 40 60, 40 40, 70 40, 70 70, 30 70, 30 30, 80 30, 80 80, 20 80, 20 20, 90 20, 90 90, 10 90, 10 10, 60 10))");
	generate_from_string("ref_hole_self_intersect", "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (80 70, 20 70, 20 20, 70 20, 70 80, 50 80, 50 30, 30 30, 30 50, 80 50, 80 70))");
	generate_from_string("ref_hole_outside", "POLYGON ((10 90, 50 90, 50 10, 10 10, 10 90), (60 80, 90 80, 90 20, 60 20, 60 80))");
	generate_from_string("ref_hole_overlap", "POLYGON ((10 90, 60 90, 60 10, 10 10, 10 90), (30 70, 90 70, 90 30, 30 30, 30 70))");
	generate_from_string("ref_hole_equal", "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (10 90, 90 90, 90 10, 10 10, 10 90))");
	generate_from_string("ref_hole_covers", "POLYGON ((30 70, 70 70, 70 30, 30 30, 30 70), (10 90, 90 90, 90 10, 10 10, 10 90))");
	generate_from_string("ref_holes_overlap", "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (80 80, 80 30, 30 30, 30 80, 80 80), (20 20, 20 70, 70 70, 70 20, 20 20))");
	generate_from_string("ref_holes_cover_exactly", "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (50 50, 50 10, 10 10, 10 50, 50 50), (50 50, 90 50, 90 10, 50 10, 50 50), (50 50, 10 50, 10 90, 50 90, 50 50), (50 50, 50 90, 90 90, 90 50, 50 50))");
	generate_from_string("ref_holes_nested", "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (30 70, 70 70, 70 30, 30 30, 30 70), (20 80, 80 80, 80 20, 20 20, 20 80))");
	generate_from_string("ref_nest_polygons", "MULTIPOLYGON (((30 70, 70 70, 70 30, 30 30, 30 70)), ((10 90, 90 90, 90 10, 10 10, 10 90)))");
	return 0;
}

