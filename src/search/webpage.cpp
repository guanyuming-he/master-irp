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

webpage::webpage(const std::string& url, url2html& convertor):
	html_tree(convertor.convert(url)),
	url(url),
	title(html_tree->get_title()),
	date(html_tree->get_date())
{

}



