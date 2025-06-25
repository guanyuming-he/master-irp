#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines utility macros and other helpers.
 * @author Guanyuming He
 */

#include <iostream>
#include <source_location>
#include <stdexcept>
#include <string>
extern "C" {
#include <lexbor/core/types.h>
}

#define NOT_IMPLEMENTED \
	throw std::runtime_error("Not implemented");

// lxb_char_t is unsigned char, so I call this ustring.
using ustring = std::basic_string<lxb_char_t>;

/**
 * This function must be called before all
 * to initialize core components.
 *
 * This must be called in the master thread before any more thread is forked.
 */
void global_init();

enum class log_levels : int 
{
	NO_LOG,
	VERBOSE_1,
	VERBOSE_2
};

constexpr log_levels log_level = log_levels::VERBOSE_1;

template <log_levels L = log_level>
void util_log(
	const std::string_view msg, 
	const std::source_location loc = std::source_location::current()
) {
	// for debug log, it's better to use endl to force flushing.
	if constexpr (L == log_levels::VERBOSE_1)
	{
		std::cout << msg << std::endl;
	}
	else if (L == log_levels::VERBOSE_2)
	{
		std::cout 
			<< loc.file_name() << '('
			<< loc.line() << ':'
			<< loc.column() << ") :\n"
			<< msg << std::endl;
	}
}
