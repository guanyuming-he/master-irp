#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the scraper class, which transfers the resource identified
 * by a URL into a std::string.
 *
 * @author Guanyuming He
 */

extern "C" {
#include <curl/curl.h>
}

#include <map>
#include <string>

#include <boost/url.hpp>
namespace urls = boost::urls;

/*
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
	 * Transfers the web resource specified by the URL which is returned as a
	 * string.
	 * Clearly, since the content is returned, the function blocks.
	 *
	 * @param url the url of the website. I cannot use a url_view here,
	 * for I need a c_str() to pass to libcurl.
	 * @param headers are the HTTP headers that the caller is interested in.
	 * Every key in the header will be filled with the corresponding value. For
	 * example, "date:". If the protocol is not HTTP/S, then not used.
	 * @returns the resource content, in string, of the url. If the transfer fails,
	 * then the content will be empty.
	 */
	std::string transfer(
		const urls::url& url,
		std::map<std::string, std::string>& headers
	) const;

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
	// It may seem better to declare it as a local var in transfer, which
	// indeed is better now, when transfer is blocking. However, I make it a
	// member variable so that it is easily extensible when in the future I
	// decide to make transfer async.
	mutable std::string buffer;

	CURL* handle;

};

