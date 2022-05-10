CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

project(raytracer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenVDB REQUIRED)
find_package(SFML COMPONENTS graphics window system REQUIRED)
find_package(OpenEXR REQUIRED)
find_package(Imath CONFIG REQUIRED)
find_package(Eigen3 3.4 REQUIRED NO_MODULE)

set(CDIR ${CMAKE_CURRENT_SOURCE_DIR})
set(EXT_SRC ${CDIR}/ext/include/stb/stb_image.h)

file(
		GLOB_RECURSE
		SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/*
)
# set(SRC ${SRC})

add_executable(${PROJECT_NAME} ${EXT_SRC} ${SRC})
add_compile_definitions(TIXML_USE_STL)
target_include_directories(${PROJECT_NAME} PRIVATE ${CDIR}/src ${CDIR}/ext/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${IMATH_INCLUDE_PATH})
target_link_libraries(${PROJECT_NAME} PRIVATE OpenVDB::openvdb sfml-graphics OpenEXR::OpenEXR Imath::Imath Eigen3::Eigen)

if (WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE sfml-main)
endif (WIN32)