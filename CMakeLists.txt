cmake_minimum_required(VERSION 3.0)

project(boost_geometry_dissolve)

FIND_PACKAGE( Boost 1.65 REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
ADD_EXECUTABLE(example example.cpp)
