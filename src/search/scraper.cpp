/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements the scraper class.
 *
 * @author Guanyuming He
 */

#include "scraper.h"

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
) const
{
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


