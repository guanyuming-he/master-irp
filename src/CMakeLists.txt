cmake_minimum_required(VERSION 3.20)
project(search_eng)

set(CMAKE_CXX_STANDARD 20)

# Define the main source files
add_executable(search_eng
	search/test.cpp
)

# curl has some problem with find_package.
# Instead, use curl-config, which is recommended by its website.
execute_process(
  COMMAND curl-config --cflags
  OUTPUT_VARIABLE curl_cflags
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
  COMMAND curl-config --libs
  OUTPUT_VARIABLE curl_lflags
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_options(
	search_eng
	PRIVATE ${curl_cflags}
)
target_link_libraries(
	search_eng
	PRIVATE ${curl_lflags}
)

find_package(xapian REQUIRED)
target_include_directories(search_eng PRIVATE ${XAPIAN_INCLUDE_DIRS})
target_link_libraries(search_eng PRIVATE ${XAPIAN_LIBRARIES})

# lexbor doesn't support find_project either.
# Just make sure I installed it.
target_link_libraries(search_eng PRIVATE lexbor)
