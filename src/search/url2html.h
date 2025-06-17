#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file declares the routines that are used to turn a url into a HTML tree.
 * They are divided into two classes,
 * a parser and a scraper.
 * The scraper gets the HTML from the URL, handling network logic.
 * The parser parses the HTML into a tree.
 * 
 * Together, they are put in a url2html class.
 *
 * @author Guanyuming He
 */

extern "C" {
#include <curl/curl.h>
#include <lexbor/core/types.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/interface.h>
}

#include <string>
#include <map>

/**
 * This struct encapsulates the HTML parser.
 */
struct parser final
{
public:
	parser();
	~parser();

public:
	lxb_html_document_t* parse(
		const lxb_char_t* buf, size_t size
	);

private:
	lxb_html_parser_t * const handle;
};

/**
 * Encapsulates a curl handle that does the scraping.
 *
 * According to libcurl doc:
 * https://everything.curl.dev/transfers/easyhandle.html#reuse
 * "Easy handles are meant and designed to be reused."
 * As such, a handle is created and reused for many url transfers
 * within a thread.
 */
class scraper final 
{
public:
	// Inits libcurl.
	static void global_init();

public:
	scraper();
	~scraper();

public:
	/**
	 * Transfers the HTML document for url.
	 * Clearly, since the content is returned, the function blocks.
	 *
	 * @param url the url of the website
	 * @param headers every key in the header will be filled with the
	 * corresponding value. For example, 
	 * @returns the HTML content, in string, of the url.
	 */
	std::string transfer(
		const std::string& url,
		std::map<std::string, std::string>& headers
	);

private:
	// the CURL write callback.
	// My logic is: scraper maintains internel state
	// to know which callback calls are for one url.
	// These calls all accumulate data into one byte string.
	// Its signature is the same as in
	// https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
	static inline size_t writeback(
		char* ptr, size_t size, size_t nmemb, void* userdata
	) {
		return static_cast<scraper*>(userdata)->
			int_writeback(ptr, size, nmemb);
	}
	// the internal callback that actually does the job.
	size_t int_writeback(
		char* ptr, size_t size, size_t nmemb
	);

private:
	// This is the interal state I talked about.
	// Before every url transfer, it's cleared. Once a url
	// transfer finishes, it contains the accumulated data.
	std::string buffer;

	CURL* handle;

};


/**
 * Using a parser and a scraper,
 * turns a url into a 
 * lxb_html_document_t*
 */
class url2html
{
public:
	url2html():
		s(), p()
	{}

public:
	lxb_html_document_t* convert(const std::string& url);

private:
	scraper s;
	parser p;
};
