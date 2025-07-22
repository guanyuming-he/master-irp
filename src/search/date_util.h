#pragma once
/**
 * Copyright (C) 2025 Guanyuming He
 * The file is licensed under the GNU GPL v3.
 *
 * This file defines the utility functions for date handling only.
 * It's different from utility.h where the functions are needed by most if not
 * all.
 * 
 * @author Guanyuming He
 */

#include <optional>
#include <chrono>

namespace ch = std::chrono;

/**
 * Try to parse a str that may indicate a valid date.
 * Supports many different formats.
 * @returns a valid year_month_day iff parsing is succesful
 */
std::optional<ch::year_month_day> try_parse_date_str(
	std::string_view str
);

void date_global_init();
