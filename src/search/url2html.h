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

#include <type_traits>
extern "C" {
#include <curl/curl.h>
#include <lexbor/core/types.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/interface.h>
}

#include <string>
#include <vector>
#include <map>
#include <chrono>
namespace ch = std::chrono;


/**
 * The struct contains the HTML tree of a webpage.
 */
struct html final 
{
public:
	// html content of no webpage is not accepted.
	html() = delete;

	// forward the map, supporting both copy and move.
	template <typename M, typename S>
	requires std::is_same_v<
		std::remove_cvref_t<M>, 
		std::map<std::string, std::string>
	> && std::is_same_v<
		std::remove_cvref_t<S>, 
		std::string
	>
	html(
		lxb_html_document_t* const handle,
		M&& headers, S&& text
	):
		handle(handle), headers(std::forward<M>(headers)),
		text(std::forward<S>(text))
	{}
	~html();

public:
	// @returns the title of the webpage or empty if it doesn't have a title
	std::string get_title() const;

	// @returns the date of the page or today if it doesn't have a date.
	ch::year_month_day get_date() const;

	/**
	 * This is for recursive scraping.
	 * @returns a list of all urls referred to by the HTML document.
	 */
	std::vector<std::string> get_urls() const;

private:
	lxb_html_document_t* const handle;

public: // no need to be private since they are immutable.
	// HTTP response headers are in key: val format.
	// E.g. Connection: keep-alive
	// However, I don't record all headers here, only
	// those that I need, like Date: xxx
	// Which headers are collected is controlled by the specific parameter
	// passed in parser.
	// Note that, although the HTTP standard says the header name is
	// case-insensitive, in this map all names are lowercase.
	// E.g. date
	// keep-alive
	const std::map<std::string, std::string> headers;
	// all the text in the HTML document, concated together.
	const std::string text;

};


/**
 * This struct encapsulates the HTML parser.
 */
struct parser final
{
public:
	parser();
	~parser();

public:
	/**
	 * Parses HTML. Optionally put all text in all_text.
	 *
	 * @param buf points to HTML data.
	 * @param size num of bytes in HTML data.
	 * @param all_text buffer to contain all the text in HTML, or nullptr if
	 * not used.
	 *
	 * @returns parsed document.
	 */
	lxb_html_document_t* parse(
		const lxb_char_t* buf, size_t size,
		std::string* all_text
	);

private:
	lxb_html_parser_t * const handle;
	
	/**
	 * the most convenient way of getting all text of a HTML
	 * is during the parsing, hooking into the tokenizer.
	 * This callback does that.
	 * @param ctx here points to a tkz_ctx.
	 */
	static lxb_html_token_t *
	token_callback(
		lxb_html_tokenizer_t *tkz, 
		lxb_html_token_t *token, void *ctx
	);
	struct tkz_ctx
	{
		decltype(&token_callback) ori_callback;
		void* ori_ctx;
		std::string* new_ctx;
	};
	// store it throughout my life time
	// so that its addr won't expire.
	tkz_ctx my_ctx;
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
	html convert(const std::string& url);

private:
	scraper s;
	parser p;
};
