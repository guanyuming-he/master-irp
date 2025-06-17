/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements the class that represents a webpage
 * and related helpers.
 *
 * @author Guanyuming He
 */

#include "webpage.h"
#include "url2html.h"
#include <chrono>
#include <lexbor/dom/interfaces/element.h>
#include <stdexcept>

extern "C" {
#include <lexbor/html/interfaces/document.h>
#include <lexbor/dom/dom.h>
}

html::~html()
{
	lxb_html_document_destroy(handle);
}

webpage::webpage(
	const std::string& url,
	const std::string& title,
	const ch::year_month_day& date
):
	url(url), title(title), date(date)
{}

webpage::webpage(const std::string& url, url2html& convertor):
	html_tree(convertor.convert(url)),
	url(url),
	title(html_tree->get_title()),
	date(html_tree->get_date())
{

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
    if (!a_tags)
        throw std::runtime_error("Could not make lexbor collection.");

	auto status = lxb_dom_elements_by_tag_name(
		lxb_dom_interface_element(handle->body),
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
