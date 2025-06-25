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

#include <algorithm>
#include <boost/system/result.hpp>
#include <cctype>

webpage::webpage(
	const std::string_view url,
	const std::string& title,
	const ch::year_month_day& date
):
	html_tree(std::nullopt),
	url(
		boost::urls::url(url).remove_fragment().remove_query()
	), 
	title(title), date(date)
{}

std::vector<urls::url> webpage::get_urls() const
{
	if (!html_tree)
		return {};
	
	// we need to turn each relative one into an absolute one.
	auto raw_urls = html_tree->get_urls();

	std::vector<urls::url> ret;
	ret.reserve(raw_urls.size());

	for (auto& raw : raw_urls)
	{
		auto normalized_end = std::remove_if(
			raw.begin(), raw.end(),
			[](unsigned char x) { return std::isspace(x); }
		);

		urls::url dest(this->url);
		boost::system::result<void> res;
		try {
			res = dest.resolve(
				urls::url_view{boost::core::string_view{
					std::string_view(raw.begin(), normalized_end)		
				}}
			);
		}
		catch (...) {
			// Just skip this one if cannot resolve.
			continue;
		}
		// Only add if the resolution was successful.
		if (!res.has_error())
			ret.emplace_back(std::move(dest));
	}

	return ret;
}
