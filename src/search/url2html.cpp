/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the routines declared in url2html.h
 *
 * @author Guanyuming He
 */

#include "url2html.h"

#include <cctype>
#include <lexbor/html/tokenizer.h>
#include <ranges>

extern "C" {
#include <lexbor/core/base.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/tokenizer.h>
#include <lexbor/html/interfaces/document.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/dom/dom.h>
#include <curl/curl.h>
#include <curl/easy.h>
}

#include <stdexcept>
#include <string>
#include <iomanip>
#include <chrono>



html::~html()
{
	lxb_html_document_destroy(handle);
}


parser::parser() :
	handle(lxb_html_parser_create())
{
	auto status = lxb_html_parser_init(handle);
	if (LXB_STATUS_OK != status)
	{
		throw std::runtime_error("Can't create a HTML parser.");
	}

	my_ctx.ori_callback = handle->tkz->callback_token_done;
	my_ctx.ori_ctx = handle->tkz->callback_token_ctx;
}

parser::~parser()
{
	lxb_html_parser_destroy(handle);
}

lxb_html_document_t* parser::parse(
	const lxb_char_t* buf, size_t size,
	std::string* all_text
) {
	if (!buf)
		return nullptr;

	if (nullptr != all_text)
	{
		all_text->clear();
		all_text->reserve(32*1024);
		my_ctx.new_ctx = all_text;
		lxb_html_tokenizer_callback_token_done_set(
			handle->tkz, &token_callback, &my_ctx
		);
	}
	else 
	{
		lxb_html_tokenizer_callback_token_done_set(
			handle->tkz, 
			my_ctx.ori_callback, my_ctx.ori_ctx
		);
	}

	auto* doc = lxb_html_parse(
		handle, buf, size
	);
	if (!doc)
		throw std::runtime_error("Can't parse HTML.");

	// the parser needs to be reset after each use.
	lxb_html_parser_clean(handle);

	return doc;
}
	
lxb_html_token_t* parser::token_callback(
	lxb_html_tokenizer_t *tkz, 
	lxb_html_token_t *token, void *ctx
) {
	const tkz_ctx& big_ctx = 
		*static_cast<tkz_ctx*>(ctx);

	// Only process the text tokens, and only
	// process when str is not nullptr.
    if (token->tag_id == LXB_TAG__TEXT) 
	{
		auto& str = *(big_ctx.new_ctx);
		// Don't have to lowercase everything, as Xapian 
		// can handle character cases.
		str.reserve(
			str.size() + (token->text_end - token->text_start)
		);
		str.append(token->text_start, token->text_end);
    }

	// then, call original callback
	return big_ctx.ori_callback(
		tkz, token, big_ctx.ori_ctx
	);

}

void scraper::global_init()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

scraper::scraper():
	buffer(),
	handle(curl_easy_init())
{
	if (!handle)
		throw std::runtime_error("Can't create curl handle.");

	// once set, these options will not be changed.

	// turn off progress meter to slightly increase perf
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
	// use a real agent to increase response chance.
	curl_easy_setopt(handle, CURLOPT_USERAGENT, 
		"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML,"
		" like Gecko) Chrome/124.0.0.0 Safari/537.36"
	);
	// follow redirections
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
	// limit max redirs just to be safe.
	curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 50L);
	// prefer HTTP over TLS.
	curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, 
		(long)CURL_HTTP_VERSION_2TLS
	);
	// keep alive may help, since I am going to reuse the same handle
	// across different urls with the same domain.
	curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE, 1L);

	// the writedata function
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &writeback);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

}

scraper::~scraper()
{
	curl_easy_cleanup(handle);
}
 
std::string scraper::transfer(
	const urls::url& url,
	std::map<std::string, std::string>& headers
) {
	buffer.clear();
	// reserves 64KB, an average size of HTML articles of pure text.
	// This avoids the first few reallocations
	// and also does not waste much if the website is small.
	buffer.reserve(64*1024u);

	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_perform(handle);

	// check the headers
	for (const auto& [k,v] : headers)
	{
		curl_header* header;
		auto h = curl_easy_header(
			handle, k.c_str(), 
			0, CURLH_HEADER,
			-1, &header
		);

		if (CURLHE_OK == h)
		{
			headers[k] = std::string(header->value);
		}
	}

	// buffer is no use to me. I relinquish its resource to you.
	return std::move(buffer);
}

size_t scraper::int_writeback(
	char* ptr, size_t size, size_t nmemb
) {
	size_t rel_size = size * nmemb;

	// this function does not need to know which url data it is transferring.
	// All it needs to do is to write to buffer,
	// which is automatically cleared on transferring a new url.
	//
	// This insert automatically handles reallocation, using
	// string's algorithm.
	buffer.append(ptr, rel_size);

	return rel_size;
}


html url2html::convert(
	const urls::url& url
) {
	// I only care about the date for now.
	std::map<std::string, std::string> headers {
		{ "date", "" }
	};

	std::string content{ s.transfer(url, headers) };	
	// curl returns char array, but lxb expect unsigned char array.
	// Anyway, if lxb only expected bytes, then it's fine.
	std::string text;
	auto* doc =  p.parse(
		reinterpret_cast<const lxb_char_t*>(content.c_str()),
	   	content.size(),
		&text
	);

	return html(
		doc, 
		std::move(headers), std::move(text)
	);

}

std::string html::get_title() const 
{
	size_t size;
    const lxb_char_t* title = lxb_html_document_title(
			handle, &size
	);
    if (!title) return "";

    return std::string(reinterpret_cast<const char*>(title), size);
}
 
ch::year_month_day html::get_date() const
{
	auto now = ch::system_clock::now();

	if (!headers.contains("date"))
	{
		// return today.
		return ch::year_month_day(
			ch::floor<ch::days>(now)
		);
	}

	// get the date from the header.
	std::istringstream is{ headers.at("date") };
	// The Date format uses en_US locale.
	// (which is the default one)
	is.imbue(std::locale());
	
	std::tm time;
	// don't waste time to parse the time after the year.
	is >> std::get_time(
		&time, "%a, %d %b %Y"
	);
	if (is.fail())
	{
		// The HTTP response header is corrupted.
		// But don't fail loudly.
		// Just return today.
		return ch::year_month_day(
			ch::floor<ch::days>(now)
		);
	}
	
	ch::year_month_day date{
		ch::year{time.tm_year+1900}, 
		ch::month{static_cast<unsigned>(time.tm_mon+1)}, 
		ch::day{static_cast<unsigned>(time.tm_mday)}
	};
	return date;
}

std::vector<std::string> html::get_urls() const 
{
    std::vector<std::string> urls;

	// No official doc for how to do this. 
	// The code is modified from the example
	// https://github.com/lexbor/lexbor/blob/master/examples/
	// lexbor/html/elements_by_tag_name.c

    auto* a_tags = lxb_dom_collection_make(
		&handle->dom_document, 128
	);

    if (!a_tags) {
        throw std::runtime_error("Could not make lexbor collection.");
	}

	auto status = lxb_dom_elements_by_tag_name(
		lxb_dom_interface_element(handle),
        a_tags, 
		(const lxb_char_t *)"a", 1
	);
	if (LXB_STATUS_OK != status)
		throw std::runtime_error("Can't get HTML a tags.");


	// for how to do the following, see 
	// https://github.com/lexbor/lexbor/blob/master/examples/
	// lexbor/html/element_attributes.c
    for (size_t i = 0; i < lxb_dom_collection_length(a_tags); ++i) 
	{
        auto* element = lxb_dom_collection_element(
			a_tags, i
		);
		size_t attr_len;
		auto* attr = lxb_dom_element_get_attribute(
			element, 
			(const lxb_char_t*)"href", 4, 
			&attr_len
		);
		if (!attr) // might not have href.
			continue;

        urls.emplace_back(
			(const char*)attr, attr_len
		);
    }

    return urls;
}

