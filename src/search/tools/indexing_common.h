#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines common data and procedurals that are shared among programs
 * that index new documents into the database.
 *
 * In fact, as different programs that include this header are each built
 * independently of the others, it doesn't matter whether the variables have 
 * external or internal linkage --- they will appear in each program's data
 * independently anyway.
 *
 * But to clarify that fact, I think it is better to delcare them as static and
 * define them in the header, emphasizing the fact that in each program they
 * are independent. This makes my software source easier to understand.
 *
 * @author Guanyuming He
 */

#include "../indexer.h"
#include "../webpage.h"

#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>

/**
 * In general, the condition of recursing is more loose than that of indexing.
 * If a document is indexed, then naturally we would want to also recurse it 
 * to find related ones.
 *
 * @returns (b1, b2).
 * Recurse iff b1; index iff b2.
 */
using path_filter_func_t = 
std::pair<bool, bool>
(const std::string_view);

// At least three words in a 
// word-word-word-... pattern
// This is common it news article addresses.
static const std::string words_sep_dash_pattern("[A-Za-z](-[A-Za-z]+){2,}");
static const std::regex words_sep_dash_re(words_sep_dash_pattern);
static bool has_words_separated_by_dash(const std::string_view p)
{
	return std::regex_search(
		p.begin(), p.end(), 
		words_sep_dash_re
	);
}
/**
 * @returns true iff the path has date encoded in it.
 * 
 * The test cases for this function are done in regex testing websites
 * like regex101.com.
 * Test cases (must accept):
 * 2025-02-01
 * 2025-29-07
 * 08-12-2025
 * 12-09-2025
 * 2025/11/03
 * 2025/03/15
 * 11/20/2025
 * 30/01/2025
 * (must not accept)
 * -1/-2/2025
 * 1/1/1
 * 2021/2022/2023
 */
static const std::string date_in_path_pattern(
	R"((^|\/)\d{4}[-/]\d{1,2}[-/]\d{1,2}($|\/)|(^|\/)\d{1,2}[-/]\d{1,2}[-/]\d{4}($|\/))"
);
static const std::regex date_in_path_regex(date_in_path_pattern);
static bool has_dates(const std::string_view p)
{
	return std::regex_search(
		p.begin(), p.end(),
		date_in_path_regex
	);
}
/**
 * For my indexer, my url filter rule is:
 * for the host, executes a function which
 * returns (b_recurse, b_index), given input path of the url.
 */
static const std::unordered_map<std::string, path_filter_func_t*> 
filtermap {
	{std::string("hbr.org"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.starts_with("/topic") ||
				p.starts_with("/the-latest");
			bool b2 = 
				// it has an odd structure of /yyyy/mm
				p.size() > 8 &&
				std::isdigit(p.at(1)) &&
				std::isdigit(p.at(2)) &&
				std::isdigit(p.at(6)) &&
				has_words_separated_by_dash(p);
			return {b1||b2, b2};
	}},
	{std::string("www.cnbc.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.starts_with("/business") ||
				p.starts_with("/investing") ||
				p.starts_with("/markets");
			bool b2 = 
				has_dates(p) &&
				has_words_separated_by_dash(p);
			return {b1||b2, b2};
	}},
	{std::string("www.ft.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.empty() ||
				p.starts_with("/companies") ||
				p.starts_with("/markets");
			bool b2 = 
				p.starts_with("/content");
			// here I don't use b1||b2 for the first because b2 is too general.
			return {b1, b2};
	}},
	{std::string("edition.cnn.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.empty() ||
				p.starts_with("/business");
			bool b2 = 
				has_dates(p) && 
				has_words_separated_by_dash(p) &&
				p.contains("/business/");
			return {b1||b2, b2};
	}},
	{std::string("www.economist.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.empty() ||
				p.starts_with("/topics");
			bool b2 = 
				has_dates(p) &&
				has_words_separated_by_dash(p);
			return {b1||b2, b2};
	}},
	{std::string("fortune.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.starts_with("/the-latest") ||
				p.starts_with("/section");
			bool b2 = 
				p.starts_with("/article") ||
				has_words_separated_by_dash(p);
			return {b1||b2, b2};
	}},
	{std::string("www.theguardian.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = 
				p.starts_with("/business") ||
				p.starts_with("/money") || 
				p.starts_with("/uk/business") ||
				p.starts_with("/uk/money");
			bool b2 = has_dates(p);
			return {b1||b2, b1&&b2};
	}},
	{std::string("www.theatlantic.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = p.starts_with("/economy"); 
			bool b2 = has_dates(p);
			return {b1, b1&&b2};
	}},
	{std::string("www.ibtimes.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = p.starts_with("/economy-markets");
			bool b2 = has_words_separated_by_dash(p);
			return {b1||b2, b2};
	}},
	{std::string("www.forbes.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = p.starts_with("/business");
			bool b2 = p.starts_with("/sites");
			return {b1||b2, b2};
	}},
	{std::string("www.nytimes.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = p.starts_with("/section");
			bool b2 = has_dates(p);
			bool b3 = 
				p.contains("business")
				|| p.contains("market");
			return {(b1||b2) && b3, b2&&b3};
	}},
	{std::string("www.inc.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = p.starts_with("/section");
			bool b2 = has_words_separated_by_dash(p);
		return {b1||b2, b2};
	}},
	{std::string("www.entrepreneur.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = p.starts_with("/business-news");
			bool b2 = has_words_separated_by_dash(p);
		return {b1||b2, b2};
	}},
	{std::string("www.foxbusiness.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = true;
			bool b2 = has_words_separated_by_dash(p);
		return {b1||b2, b2};
	}},
	// It needs me to enable JS
	{std::string("www.reuters.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.starts_with("/business") ||
				p.starts_with("/markets");
			bool b2 = 
				has_dates(p) &&
				has_words_separated_by_dash(p);
			return {b1||b2, b1&&b2};
	}},
	// Bloomberg blocked me
	{std::string("www.bloomberg.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = 
				p.empty() ||
				p.starts_with("/uk") ||
				p.starts_with("/economics") ||
				p.starts_with("/markets") || 
				p.starts_with("/deals");
			bool b2 = p.starts_with("news/articles");
			return {b1||b2, b2};
	}},
	// wsj blocks me if I don't enable JS and cookies.
	{std::string("www.wsj.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.empty() ||
				p.starts_with("/business") ||
				p.starts_with("/economy");
			bool b2 = 
				has_words_separated_by_dash(p);
			// here I don't use b1||b2 for the first because b2 is too general.
			return {b1||b2, b2};
	}},
	// Don't index this website, according to Sean.
	//{std::string("www.businessinsider.com"), 
	//	[](const std::string_view p) -> std::pair<bool,bool> {
	//		bool b1 = p.starts_with("/business");
	//		bool b2 = has_words_separated_by_dash(p);
	//		return {b1||b2, b2};
	//}},
};

static bool index_filter(urls::url& u)
{
	std::string key{u.encoded_host()};
	if (!filtermap.contains(key))
		return false;

	auto val {filtermap.at(key)};
	return val(u.encoded_path()).second;
}

static bool recurse_filter(urls::url& u)
{
	std::string key{u.encoded_host()};
	if (!filtermap.contains(key))
		return false;

	auto val {filtermap.at(key)};
	return val(u.encoded_path()).first;
}

static bool wp_index_filter(webpage& pg)
{
	// for now just return true if the year
	// is within last 2 years and the text is not empty.
	return 
		(int)pg.get_date().year() >= 2024 &&
		!pg.get_text().empty();
}

static bool wp_recurse_filter(webpage& pg)
{
	// recurse only if either its title or its text is not empty.
	return 
		!pg.get_text().empty() &&
		!pg.get_title().empty();
}

// Whether use this or load from file depends on the cmd args.
static indexer::uque_t start_queue;

