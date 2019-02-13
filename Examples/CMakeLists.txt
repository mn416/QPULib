cmake_minimum_required(VERSION 3.1)
project(QPULib_Examples CXX)
set(CMAKE_CXX_STANDARD 14)

macro(add_executable _name)
	_add_executable(${ARGV})
	if(TARGET ${_name})
		# link libqpu statically because we don't know if it has been installed yet
		target_link_libraries(${_name} libqpu.a)

		# ensure that the qpu library is built first, no matter if static or not
		add_dependencies(${_name} qpu.so)
		add_dependencies(${_name} qpu.a)
	endif()
endmacro()

STRING(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Lib)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/../lib)

add_executable(AutoTest ${CMAKE_CURRENT_SOURCE_DIR}/AutoTest.cpp)
add_executable(DMA ${CMAKE_CURRENT_SOURCE_DIR}/DMA.cpp)
add_executable(GCD ${CMAKE_CURRENT_SOURCE_DIR}/GCD.cpp)
add_executable(HeatMap ${CMAKE_CURRENT_SOURCE_DIR}/HeatMap.cpp)
add_executable(HeatMapScalar ${CMAKE_CURRENT_SOURCE_DIR}/HeatMapScalar.cpp)
add_executable(Hello ${CMAKE_CURRENT_SOURCE_DIR}/Hello.cpp)
add_executable(ID ${CMAKE_CURRENT_SOURCE_DIR}/ID.cpp)
add_executable(Mandelbrot ${CMAKE_CURRENT_SOURCE_DIR}/Mandelbrot.cpp)
add_executable(MultiTri ${CMAKE_CURRENT_SOURCE_DIR}/MultiTri.cpp)
add_executable(OET ${CMAKE_CURRENT_SOURCE_DIR}/OET.cpp)
add_executable(Print ${CMAKE_CURRENT_SOURCE_DIR}/Print.cpp)
add_executable(ReqRecv ${CMAKE_CURRENT_SOURCE_DIR}/ReqRecv.cpp)
add_executable(Rot3D ${CMAKE_CURRENT_SOURCE_DIR}/Rot3D.cpp)
add_executable(Sort ${CMAKE_CURRENT_SOURCE_DIR}/Sort.cpp)
add_executable(Tri ${CMAKE_CURRENT_SOURCE_DIR}/Tri.cpp)
add_executable(TriFloat ${CMAKE_CURRENT_SOURCE_DIR}/TriFloat.cpp)
