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
#include <chrono>
namespace ch = std::chrono;

#include "utility.h"

// lexbor
extern "C" {
#include <lexbor/html/parser.h>
#include <lexbor/html/interface.h>
}

/**
 * This struct encapsulates the HTML parser.
 * 
 * More precisely, it only encapsulates its creation and destruction.
 * Everything else is exposed.
 */
struct parser final
{
public:
	parser();
	~parser();

	lxb_html_parser_t * const handle;
};

/**
 * The struct contains the HTML content of a webpage.
 */
struct html final 
{
public:
	// html content of no webpage is not accepted.
	html() = delete;
	/**
	 * Parses the content of a HTML
	 * @param content the byte-string content of a HTML document.
	 */
	html(const ustring& content, parser& pser);
	~html();

public:
	// These two are for webpage metadata
	std::string get_title() const;
	std::string get_date() const;

	/**
	 * This is for index building.
	 * @returns all the text of the HTML document, in lowercase.
	 */
	std::string get_text() const;

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
	explicit webpage(const std::string& url);

public:
	// @returns the text in lowercase or "" if not loaded.
	inline std::string get_text() const 
	{ return html_tree ? html_tree->get_text() : ""; }

private: // declare this first as it needs to be inited first.
	std::optional<html> html_tree;
public: // since they are immutable, no need to go private.
	const std::string url;
	const std::string title;
	const ch::year_month_day date;

};
