# Dissolve for boost geometry

This project provides a header-only library for dissolving or solving common problems in polygons for boost geometry. The problems this library can remove are: 
- self-intersection
- overlapping inners
- inners outside of the polygon outer

The main purpose of this library is to provide removing self-intersection from polygons using the boost geometry library. For this, a general approach is used. Meaning, for every polygon it can remove self-intersection, possibly generating multiple output polygons

# Example 1
![example 1 input](images/example_1_input.png)
![example 1 output](images/example_1_output.png)

# Example 2
![example 2 input](images/example_2_input.png)
![example 2 output](images/example_2_output.png)


# License

