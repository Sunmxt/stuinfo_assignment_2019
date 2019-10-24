cmake_minimum_required(VERSION 3.15)
project(stuinfo_assignment_2019)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(sqlite3)
find_package(glog)

set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(stuinfo main.cpp student.cpp)
add_library(sorm STATIC sorm/indexer.cpp sorm/mapper.cpp sorm/model.cpp)
target_link_libraries(stuinfo ${SQLite3_LIBRARIES} glog::glog sorm)