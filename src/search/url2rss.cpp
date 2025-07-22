/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements the rss class.
 *
 * @author Guanyuming He
 */

#include "date_util.h"
#include "url2rss.h"

std::optional<webpage> 
rss::webpage_from_rss_item(const pugi::xml_node& item) const
{
	std::string title = item.child("title").text().as_string();

	std::string link_str = item.child("link").text().as_string();
	// Link is necessary.
	if (link_str.empty())
		return std::nullopt;
	auto link = process_link(link_str);
	if (!link)
		return std::nullopt;

	// Get date (pubDate for RSS 2.0)
	std::string date_str = item.child("pubDate").text().as_string();
	auto parsed_date = try_parse_date_str(date_str);

	// Fail gracefully instead of catastraphically.
	try 
	{
		return webpage(
			*link, title, 
			parsed_date.value_or(ch::floor<ch::days>(ch::system_clock::now()))
		);
	} 
	catch (const std::runtime_error& e)
	{
		// Something failed in webpage constructor
		return std::nullopt;
	}
}

std::optional<webpage> 
rss::webpage_from_atom_item(const pugi::xml_node& item) const
{
	// Get title
	std::string title = item.child("title").text().as_string();
	
	// Get link - Atom uses <link href="..."/>
	std::string link_str;
	auto link_node = item.child("link");
	if (link_node)
		link_str = link_node.attribute("href").as_string();
	// Link is necessary.
	if (link_str.empty())
		return std::nullopt;
	auto link = process_link(link_str);
	if (!link)
		return std::nullopt;
	
	// Get date (updated for Atom)
	std::string date_str = item.child("updated").text().as_string();
	auto parsed_date = try_parse_date_str(date_str);
	
	try 
	{
		return webpage(
			*link, title, 
			parsed_date.value_or(ch::floor<ch::days>(ch::system_clock::now()))
		);
	} 
	catch (const std::runtime_error&) 
	{
		// Something failed in webpage constructor
		return std::nullopt;
	}
}

std::optional<urls::url> 
rss::process_link(std::string_view link) const
{
	// Although an empty string is a valid RFC 3986 URI.
	// it doesn't fit here, as that would count as a relative URI pointing to
	// the RSS feed URI itself, which makes no sense.
	if (link.empty())
		return std::nullopt;

	std::optional<urls::url> dst;

	try 
	{
		dst.emplace(link);
	}	
	catch (const std::system_error& e)
	{
		return std::nullopt;
	}

	urls::url ret;
	auto res = urls::resolve(
		this->url, *dst,
		ret
	);
	if (res.has_error())
		return std::nullopt;

	// url.resolve does it in-place in dst.
	return ret;
}

rss::rss(rss&& other) noexcept : 
	doc(std::move(other.doc)),
	url(other.url) // To maintain its immutability, I simply copy it.
{}

std::vector<webpage> rss::read_webpages() const 
{
	std::vector<webpage> webpages;
	
	// Try RSS 2.0 format first
	auto rss_root = doc.child("rss");
	if (rss_root) 
	{
		auto channel = rss_root.child("channel");
		if (channel) 
		{
			for (auto item : channel.children("item")) 
			{
				auto wp = webpage_from_rss_item(item);
				if (wp.has_value())
					webpages.push_back(std::move(wp.value()));
			}
		}
	}
	
	// Try Atom format
	auto feed_root = doc.child("feed");
	if (feed_root) 
	{
		// Check if it's Atom namespace
		auto xmlns = feed_root.attribute("xmlns");
		if (
			xmlns && 
			( 
			 	std::string(xmlns.as_string()).contains("atom") ||
			 	std::string(xmlns.as_string()).contains("Atom")
			) 
		)
		{
			for (auto entry : feed_root.children("entry")) 
			{
				auto wp = webpage_from_atom_item(entry);
				if (wp.has_value()) 
					webpages.push_back(std::move(wp.value()));
			}
		}
	}
	
	// Return their accumulation. In reality, only one of them
	// will be accessed, because a RSS XML cannot have both a rss and a feed
	// root, normally.
	// If none of them is present, then an empty vector is returned.
	return webpages;	
}
