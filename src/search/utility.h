#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines utility macros and other helpers.
 * @author Guanyuming He
 */

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
