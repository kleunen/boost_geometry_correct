#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>

#include "correct.hpp"
#include "data/CLC2006_180927.wkt.cpp"

#include <iostream>
#include <fstream>
#include <chrono>
#include <random>

template <typename Mp>
void map(std::string const& name, Mp const& mp, Mp const& result)
{
  using point = typename boost::geometry::point_type<Mp>::type;

  std::string const filename = name + ".svg";
  std::ofstream svg(filename.c_str());

  boost::geometry::svg_mapper<point> mapper(svg, 400, 400);

  mapper.add(mp);
  mapper.add(result);

  // Original, light yellow
  mapper.map(mp, "fill-opacity:0.3;fill:rgb(255,255,192);stroke:rgb(255,255,0);" "stroke-width:1");
  // Fixed, blue on top
  mapper.map(result, "fill-opacity:0.6;fill:rgb(0,0,255);stroke:rgb(0,0,128);" "stroke-width:2");
}

template <typename Mp>
void measure_performance(std::string const& name, Mp const& mp, char option, int count)
{
  auto const start = std::chrono::steady_clock::now();

  const double remove_spike_threshold = 1E-12;
  for (int i = 0; i < count; i++)
  {
    Mp result;
    switch(option)
    {
 //     case 'd' : boost::geometry::dissolve(mp, result); break;
      case 'c' : geometry::correct(mp, result, remove_spike_threshold); break;
      case 'o' : geometry::correct_odd_even(mp, result, remove_spike_threshold); break;
    }
  }
  auto const end = std::chrono::steady_clock::now();
  auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  std::cout << name << " " << option << " time: " << ms / 1000.0 << std::endl;
}

void compare_fix_methods(std::string const& name, std::string const& wkt, int count = 0)
{
  using point = boost::geometry::model::d2::point_xy<double>;
  using polygon = boost::geometry::model::polygon<point>;
  using multi_polygon = boost::geometry::model::multi_polygon<polygon>;

  multi_polygon mp, dissolved, corrected, oddeven;

  boost::geometry::read_wkt(wkt, mp);

 // boost::geometry::dissolve(mp, dissolved);

  double const remove_spike_threshold = 1E-12;
  geometry::correct(mp, corrected, remove_spike_threshold);
  geometry::correct_odd_even(mp, oddeven, remove_spike_threshold);

  auto valid_str = [](bool v) { return v ? "valid" : "invalid"; };
  std::cout << name << " areas " << boost::geometry::area(mp)

 //           << " d " << boost::geometry::area(dissolved)
   //         << " " << valid_str(boost::geometry::is_valid(dissolved))

            << " c " << boost::geometry::area(corrected)
            << " " << valid_str(boost::geometry::is_valid(corrected))

            << " o " << boost::geometry::area(oddeven)
            << " " << valid_str(boost::geometry::is_valid(oddeven))
            << std::endl;

 //  map(name + "_d", mp, dissolved);
   map(name + "_c", mp, corrected);
   map(name + "_o", mp, oddeven);

   if (count > 0)
   {
     measure_performance(name, mp, 'd', count);
     measure_performance(name, mp, 'c', count);
     measure_performance(name, mp, 'o', count);
   }
   std::cout << std::endl;
}

// Generates a random polygon
std::string get_random(long long seed)
{
  std::mt19937 engine(seed);
  auto real_rand = std::bind(std::uniform_real_distribution<double>(0,1), engine);
  std::string first;
  std::string result = "MULTIPOLYGON(((";
  for (int i = 0; i < 100; i++)
  {
    std::ostringstream out;
    out << real_rand() << " " << real_rand();
    if (i > 0) { result += ","; } else { first = out.str(); }
    result += out.str();
  }
  // Close and finish it
  result += "," + first + ")))";

  return result;
}

int main()
{
  compare_fix_methods("pentagram", "MULTIPOLYGON(((5 0, 2.5 9, 9.5 3.5, 0.5 3.5, 7.5 9, 5 0)))", 10000);
  compare_fix_methods("complex", "MULTIPOLYGON(((55 10, 141 237, 249 23, 21 171, 252 169, 24 89, 266 73, 55 10)))", 0000);
  compare_fix_methods("multiple", "MULTIPOLYGON(((0 0, 10 0, 0 10, 10 10, 0 0, 5 0, 5 10, 0 10, 0 5, 10 5, 10 0, 0 0)))", 10000);
  compare_fix_methods("overlapping", "MULTIPOLYGON (((10 70, 90 70, 90 50, 30 50, 30 30, 50 30, 50 90, 70 90, 70 10, 10 10, 10 70)))", 10000);

  compare_fix_methods("random42", get_random(42), 10);
  compare_fix_methods("random123", get_random(123), 10);
  compare_fix_methods("large", wkt_CLC2006_180927, 10);

  return 0;
}


