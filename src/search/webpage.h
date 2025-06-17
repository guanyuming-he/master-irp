#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the class that represents a webpage
 * and related helpers.
 *
 * @author Guanyuming He
 */

#include <optional>
#include <string>
#include <vector>
#include <chrono>
namespace ch = std::chrono;

extern "C" {
#include <lexbor/html/interfaces/document.h>
}

#include "utility.h"

class url2html;

/**
 * The struct contains the HTML tree of a webpage.
 */
struct html final 
{
public:
	// html content of no webpage is not accepted.
	html() = delete;
	explicit html(lxb_html_document_t* const handle):
		handle(handle)
	{}
	~html();

public:
	// These two are for webpage metadata
	std::string get_title() const;
	ch::year_month_day get_date() const;

	/**
	 * This is for index building.
	 * @returns all the text of the HTML document, in lowercase.
	 */
	std::string get_text() const;
	/**
	 * This is for recursive scraping.
	 * @returns a list of all urls referred to by the HTML document.
	 */
	std::vector<std::string> get_urls() const;

private:
	lxb_html_document_t* const handle;

};

/**
 * The class represents a webpage.
 * It has its url, title, date, and other metadata.
 *
 * Optionally, it loads the whole HTML of the page 
 */
class webpage final 
{
public:
	// Two different constructors.
	// 1. loads from file, which only has metadata. 
	webpage(
		const std::string& url,
		const std::string& title,
		const ch::year_month_day& date
	);

	// 2. loads from url, reads the HTML, and calculates the metadata.
	webpage(const std::string& url, url2html& convertor);

public:
	// @returns the text in lowercase or "" if not loaded.
	inline std::string get_text() const 
	{ return html_tree ? html_tree->get_text() : ""; }
	
	// @returns a vector of urls in the HTML, or {} if not loaded.
	inline std::vector<std::string> get_urls() const 
	{ return html_tree ? html_tree->get_urls() : std::vector<std::string>{}; }

private: // declare this first as it needs to be inited first.
	// for now, I don't know if it should be const.
	// I leave it as not to open the possibility of parsing
	// the HTML later.
	std::optional<html> html_tree;

public: // since they are immutable, no need to go private.
	const std::string url;
	const std::string title;
	const ch::year_month_day date;

};
