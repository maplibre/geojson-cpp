cmake_minimum_required(VERSION 3.10)

project(geojson-cpp)

set(CMAKE_CXX_STANDARD 20)

add_library(
  geojson-cpp
  STATIC
  src/maplibre/geojson.cpp
)

include_directories(include)

include(FetchContent)

FetchContent_Declare(
  geometry-hpp
  GIT_REPOSITORY https://github.com/maplibre/geometry.hpp
  GIT_TAG origin/main
  EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(geometry-hpp)

include_directories(SYSTEM ${geometry-hpp_SOURCE_DIR}/include)

FetchContent_Declare(
  MapboxVariant
  GIT_REPOSITORY https://github.com/mapbox/variant.git
  GIT_TAG origin/master
)
FetchContent_MakeAvailable(MapboxVariant)

include_directories(SYSTEM ${mapboxvariant_SOURCE_DIR}/include)

FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG v1.1.0
  SYSTEM
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(rapidjson)
include_directories(SYSTEM ${rapidjson_SOURCE_DIR}/include)


add_executable(
  test
  test/test.cpp
)

target_link_libraries(
  test
  geojson-cpp
)

add_executable(
  test_value
  test/test_value.cpp
)

target_link_libraries(
  test_value
  geojson-cpp
)
