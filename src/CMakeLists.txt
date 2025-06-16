cmake_minimum_required(VERSION 3.20)
project(search_eng)

set(CMAKE_CXX_STANDARD 20)

# Define the main source files
add_executable(search_eng
	search/main.cpp
)

find_package(curl REQUIRED)
target_include_directories(search_eng PRIVATE ${CURL_INCLUDE_DIR})
target_link_libraries(search_eng PRIVATE ${CURL_LIBRARIES})

find_package(xapian REQUIRED)
target_include_directories(search_eng PRIVATE ${XAPIAN_INCLUDE_DIRS})
target_link_libraries(search_eng PRIVATE ${XAPIAN_LIBRARIES})

find_package(lexbor REQUIRED)
target_include_directories(search_eng PRIVATE ${LEXBOR_INCLUDE_DIRS})
target_link_libraries(search_eng PRIVATE ${LEXBOR_LIBRARIES})


