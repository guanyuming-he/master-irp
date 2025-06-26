cmake_minimum_required(VERSION 3.20)
project(
	search_eng
	LANGUAGES CXX C
)
enable_testing()
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)

add_library(search_eng
	search/utility.cpp
	search/webpage.cpp
	search/url2html.cpp
	# deprecated.
	# search/url.cpp
	search/index.cpp
	search/indexer.cpp
	# This file comes from external library https://github.com/amosnier/sha-2
	sha-2/sha-256.c
)

add_executable(indexer
	search/indexer_main.cpp
)
target_link_libraries(indexer PRIVATE search_eng)

############## External libs ###############

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

# Xapian
find_package(xapian REQUIRED)
target_include_directories(search_eng PRIVATE ${XAPIAN_INCLUDE_DIRS})
target_link_libraries(search_eng PRIVATE ${XAPIAN_LIBRARIES})

# lexbor doesn't support find_project either.
# Just make sure I installed it.
target_link_libraries(search_eng PRIVATE lexbor)

find_package(Boost 
	1.81 
	REQUIRED
	COMPONENTS unit_test_framework url
)
target_link_libraries(search_eng PUBLIC Boost::url)

############### TESTS ####################

# Common test settings
set(testsList 
	test_parser_html 
	# deprecated
	# test_url 
	test_onewebpage 
	test_index
)
foreach (test IN LISTS testsList)
	add_executable(${test} tests/${test}.cpp)

	target_link_libraries(${test}
		PRIVATE search_eng
	)
	# The tests use boost.test
	target_link_libraries(${test}
		PRIVATE Boost::unit_test_framework
	)
	add_test(
		NAME run${test}
		COMMAND ${test}
	)
endforeach()

# Specific test settings.
# This test needs lexbor.
target_link_libraries(test_parser_html PRIVATE lexbor)
# This test needs xapian and lexbor
target_link_libraries(test_index PRIVATE lexbor)
target_include_directories(test_index PRIVATE ${XAPIAN_INCLUDE_DIRS})
target_link_libraries(test_index PRIVATE ${XAPIAN_LIBRARIES})

################# Debug options ################
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(
		-Wall -Wextra -ggdb
	)
endif()
