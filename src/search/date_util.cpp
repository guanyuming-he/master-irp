/**
 * Copyright (C) 2025 Guanyuming He
 * The file is licensed under the GNU GPL v3.
 *
 * The file implements the functions in date_util.h.
 */

#include "date_util.h"

// Use C's strptime directly.
#include <locale.h>
#include <ctime>

#include <regex>

std::optional<ch::year_month_day> try_parse_date_str(
	std::string_view str
) {
	/**
	 * Try to match the string with any of the these,
	 * where %t means {0,1}*<white-space>
	 */
	static const std::string formats[] = {
		"%Y-%m-%d",			// 2025-02-01
		"%m/%d/%Y",			// 01/02/2025
		// I can't put more variants of - or / here,
		// as %Y doesn't force reading 4 digits, but instead may read just 2 digits.

		// %a and %b are either abbriviated or full.
		"%b %d %Y",			// Feb 1 2025
		"%b %d, %Y",		// Feb 1, 2025
		"%d %b %Y",			// 1 Feb 2025
		"%d %b, %Y",		// 1 Feb, 2025
		"%a %d %b %Y",		// Sat 1 Feb 2025	
		"%a, %d %b %Y",		// Sat, 1 Feb 2025	
		"%a %b %d %Y",		// Sat Feb 1 2025	
		"%a, %b %d %Y",		// Sat, Feb 1 2025	
		"%a, %b %d, %Y",	// Sat, Feb 1, 2025	
	};
	// regex of matching ordinal
	// 1st, 2nd, ...
	// Because a day of a month can be expressed by at most 2 digits,
	// it's 
	static const std::string ord_pattern(
		R"(([\d]{0,1})(st|nd|rd|th))"
	);
	static const std::regex ord_regex(ord_pattern);
	static const std::string conseq_spaces_pattern(
		"[\\s]+"
	);
	static const std::regex  conseq_spaces_regex(
		conseq_spaces_pattern
	);
	
	// Preprocessing:
	// 1. remove head and trailing spaces, also
	// turn all other consequtive space sequences into one space.
	while (
		!str.empty() && std::isspace(str.front())
	)
		str.remove_prefix(1);
	while (
		!str.empty() && std::isspace(str.back())
	)
		str.remove_suffix(1);
	std::string space_trimmed_str;
	std::regex_replace(
		std::back_inserter(space_trimmed_str),
		str.begin(), str.end(),
		conseq_spaces_regex, " "
	);

	// 2. remove ordinal suffixes like "23rd", "1st"
	std::string proced_str;
	std::regex_replace(
		std::back_inserter(proced_str),
		space_trimmed_str.begin(), space_trimmed_str.end(),
		ord_regex, "$1"
	);
	
	for (const auto& fmt : formats) {
		std::tm t{};
		// returns NULL on failure.
		// I use strptime, since std::get_time() won't work.
		// Also, ch::parse isn't implemented in g++ 12.
		auto res = strptime(proced_str.c_str(), fmt.c_str(), &t);
		if (res) {
			// this one matches.
			return ch::year_month_day(
				ch::year(t.tm_year + 1900),
				ch::month(t.tm_mon + 1),
				ch::day(t.tm_mday)
			);
		}
	}
	
	// Could not parse in any way listed in formats.
	return std::nullopt;
}

void date_global_init()
{
	setlocale(LC_TIME, "en_US.UTF-8");
}
