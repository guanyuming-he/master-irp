cmake_minimum_required(VERSION 3.20)
project(search_eng)

set(CMAKE_CXX_STANDARD 20)


add_library(search_eng
	search/utility.cpp
	search/webpage.cpp
	search/url2html.cpp
	search/url.cpp
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


target_compile_options(
	search_eng
	PRIVATE -Wall -Wextra -ggdb
)

############### TESTS ####################
add_executable(test_parser_html
	search/test_parser_html.cpp
)
add_executable(test_url 
	search/test_url.cpp
)
# This test needs lexbor.
target_link_libraries(test_parser_html
	PRIVATE lexbor
)

# Common test settings
set(testsList 
	test_parser_html test_url
)
find_package(Boost 1.81 COMPONENTS unit_test_framework REQUIRED)
foreach (test IN LISTS testsList)
	target_link_libraries(${test}
		PRIVATE search_eng
	)
	# The test uses boost.test
	target_link_libraries(${test}
		PRIVATE Boost::unit_test_framework
	)
	target_compile_options(
		${test}
		PRIVATE -Wall -Wextra -ggdb
	)
endforeach()

