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


webpage::webpage(
	const std::string& url,
	const std::string& title,
	const ch::year_month_day& date
):
	html_tree(std::nullopt),
	url(url), title(title), date(date)
{}

std::vector<class url> webpage::get_urls() const
{
	if (!html_tree)
		return {};
	
	// we need to turn each relative one into an absolute one.
	auto raw_urls = html_tree->get_urls();

	std::vector<class url> ret;
	ret.reserve(raw_urls.size());

	for (const auto& raw : raw_urls)
	{
		ret.emplace_back(url::url_resolution(
			this->url, raw
		));
	}

	return ret;
}
