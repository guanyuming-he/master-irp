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

#include <boost/url/url_view.hpp>
#include <type_traits>
extern "C" {
#include <curl/curl.h>
#include <lexbor/core/types.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/interface.h>
}

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <chrono>
namespace ch = std::chrono;

#include <boost/url.hpp>
namespace urls = boost::urls;

/**
 * The struct contains the HTML tree of a webpage.
 */
struct html final 
{
public:
	// html content of no webpage is not accepted.
	html() = delete;

	// no copy. only move
	html(const html&) = delete;
	html(html&& other) noexcept:
		handle(other.handle), 
		headers(std::move(other.headers)), text(std::move(other.text)),
		date(std::move(other.date))
	{
		other.handle = nullptr;
	}

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
		M&& headers, S&& text,
		std::optional<ch::year_month_day>&& date = std::nullopt
	):
		handle(handle), headers(std::forward<M>(headers)),
		text(std::forward<S>(text)),
		date(std::move(date))
	{}
	~html();

public:
	// @returns the title of the webpage or empty if it doesn't have a title
	std::string get_title() const;

	/**
	 * Try to get the date from the Header.
	 * Previously, I also try to parse the HTML myself, a task
	 * which turned out to be too tedious to do well.
	 * As such, I stop doing that here, but instead seek
	 * to use Python's htmldate to do the work.
	 * Therefore, a url2html class will first try that,
	 * and only if it doesn't work, does that class fallback to calling
	 * this method.
	 */
	ch::year_month_day get_date();

	/**
	 * This is for recursive scraping.
	 * Note that each is the text *as-is* in a href,
	 * even if the href is nonsense or is an invalid url.
	 * It is my class webpage's responsibility to filter out 
	 * incorrect ones and turn all into valid urls.
	 *
	 * @returns a list of all urls referred to by the HTML document.
	 */
	std::vector<std::string> get_urls() const;

private:
	lxb_html_document_t* handle;
	// May be passed through ctor.
	// If not, calculated in get_date();
	std::optional<ch::year_month_day> date;

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
	// In lowercase.
	const std::string text;

	/**
	 * No harm in making it public; advantage in doing so:
	 * easy to test.
	 * 
	 * Try to parse a str that may indicate a valid date.
	 * @returns a valid year_month_day iff parsing is succesful
	 */
	static std::optional<ch::year_month_day> try_parse_date_str(
		std::string_view str
	);

	/**
	 * No harm in making it public; advantage in doing so:
	 * easy to test.
	 * 
	 * Try to parse the Date: in headers.
	 * @returns a valid year_month_day iff parsing is succesful
	 */
	std::optional<ch::year_month_day> 
	try_parse_header_date() const;

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
	 * @param url the url of the website. I cannot use a url_view here,
	 * for I need a c_str() to pass to libcurl.
	 * @param headers every key in the header will be filled with the
	 * corresponding value. For example, 
	 * @returns the HTML content, in string, of the url. If the transfer fails,
	 * then the content will be empty.
	 */
	std::string transfer(
		const urls::url& url,
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

// Forward decl of PyObject.
struct _object;
typedef struct _object PyObject;

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
	/**
	 * Converts a url to html.
	 * @param url a syntatically valid url. I cannot use a url_view here
	 * as I need its c_str().
	 */
	html convert(const urls::url& url);

private:
	scraper s;
	parser p;

public:
	/**
	 * Made public for testing.
	 *
	 * Try to use Python's htmldate to get a date out of the HTML.
	 * Unfortunately, as that uses a different internal rep of 
	 * a HTML document, I will have to parse each document twice, I guess.
	 *
	 * @param html_content returned by the scraper.
	 * @param u htmldate may scan the url for date info
	 */
	static std::optional<ch::year_month_day> 
	date_outof_html(
		const std::string& html_content,
		const urls::url& u
	);

	static void global_init();
	static void global_uninit();

private:
	static constexpr const char* module_name = "htmldate";
	static PyObject* htmldate_module;
	static PyObject* find_date_func;
};

std::string url_get_essential(urls::url_view u);
