cmake_minimum_required(VERSION 3.20)
project(
	search_eng
	LANGUAGES CXX C
)
enable_testing()
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)

############# Export compile_commands.json for clangd ########
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_library(search_eng
	search/utility.cpp
	search/webpage.cpp
	search/url2html.cpp
	# deprecated.
	# search/url.cpp
	search/index.cpp
	search/indexer.cpp
	search/searcher.cpp
	# This file comes from external library https://github.com/amosnier/sha-2
	sha-2/sha-256.c
)

################## Tools ########################
add_executable(indexer
	search/tools/indexer_main.cpp
)
target_link_libraries(indexer PRIVATE search_eng)
add_executable(updater
	search/tools/updater_main.cpp
)
target_link_libraries(updater PRIVATE search_eng)

add_executable(searcher
	search/tools/searcher_main.cpp
)
target_link_libraries(searcher PRIVATE search_eng)

add_executable(rm_doc
	search/tools/rm_doc.cpp
)
target_link_libraries(rm_doc PRIVATE search_eng)

add_executable(upd_doc
	search/tools/upd_doc.cpp
)
target_link_libraries(rm_doc PRIVATE search_eng)
############## External libs ###############

# Python 3 C API
# Use my anaconda's Python3.
set(CONDA_PREFIX "/home/anony/anaconda3/envs/daily")
set(Python3_ROOT_DIR ${CONDA_PREFIX})
set(Python3_EXECUTABLE "${CONDA_PREFIX}/bin/python3")
set(Python3_INCLUDE_DIRS "${CONDA_PREFIX}/include/python3.13")
set(Python3_LIBRARY_DIRS "${CONDA_PREFIX}/lib")
set(Python3_LIBRARIES 
	"${CONDA_PREFIX}/lib/libpython3.so"
	"${CONDA_PREFIX}/lib/libpython3.13.so.1.0"
) 
# Do not use this FindPython3, which isn't compatible with Anaconda
#set(Python3_FIND_STRATEGY LOCATION)
#set(Python3_FIND_VIRTUALENV ONLY)
#find_package(
#	Python3 3.13 REQUIRED
#	COMPONENTS Development
#)
target_include_directories(
	search_eng
	PRIVATE ${Python3_INCLUDE_DIRS}
)
target_link_directories(
	search_eng
	PRIVATE ${Python3_LIBRARY_DIRS}
)
target_link_libraries(
	search_eng
	PRIVATE ${Python3_LIBRARIES}
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

target_include_directories(searcher PRIVATE ${XAPIAN_INCLUDE_DIRS})
target_link_libraries(searcher PRIVATE ${XAPIAN_LIBRARIES})

############### TESTS ####################

# Common test settings
set(testsList 
	test_parser_html 
	# deprecated
	# test_url 
	test_onewebpage 
	test_index
	test_date_parsing
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

add_executable(test_bug3 tests/test_bug3.cpp)
target_link_libraries(test_bug3 PRIVATE search_eng)
target_link_libraries(test_bug3 PRIVATE ${XAPIAN_LIBRARIES})

################# Debug options ################
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(
		-Wall -Wextra -ggdb
	)
endif()
