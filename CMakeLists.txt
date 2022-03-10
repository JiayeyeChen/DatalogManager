cmake_minimum_required(VERSION 3.0.2)
project(Datalog_Manager)

find_package(Boost REQUIRED COMPONENTS system thread regex)

include_directories(
  Inc
  ${Boost_INCLUDE_DIR}
)

set(SRC
  main.cpp
  datalog.cpp
  crc32_mpeg.cpp
)

set(LIBS
  ${Boost_LIBRARIES}
)

add_executable(datalog ${SRC})
target_link_libraries(datalog ${LIBS})