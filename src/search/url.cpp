/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements the url class that 
 * represents a url
 *
 * @author Guanyuming He
 */

#include "url.h"
#include <cmath>
#include <cstddef>
#include <curl/urlapi.h>
#include <optional>
#include <stdexcept>
#include <string>
extern "C" {
#include <curl/curl.h>
}


curl_str::~curl_str()
{
	// doc says it does nothing if str == nullptr.
	curl_free(str);
}

curl_str& curl_str::operator=(curl_str&& right) noexcept
{
	curl_free(str);

	str = right.str;
	right.str = nullptr;

	return *this;
}

url::url(const char* str):
	handle(curl_url())
{
	if (!handle)
		throw std::runtime_error("Can't create CURLU.");

	auto rc = curl_url_set(
		handle,
	   	CURLUPART_URL, str, 
		0
	);

	if (CURLUE_OK != rc)
		throw std::runtime_error("Can't set CURLUPART_URL.");

	strip_query_frag();
}

url::url(
	const char* host, const char* res, 
	const char* scheme
): handle(curl_url()) 
{
	if (!handle)
		throw std::runtime_error("Can't create CURLU.");

	auto rc = curl_url_set(
		handle,
	   	CURLUPART_SCHEME, scheme, 
		0
	);
	if (CURLUE_OK != rc)
		throw std::runtime_error("Can't set CURLUPART_SCHEME.");
	rc = curl_url_set(
		handle,
	   	CURLUPART_HOST, host, 
		0
	);
	if (CURLUE_OK != rc)
		throw std::runtime_error("Can't set CURLUPART_HOST.");
	rc = curl_url_set(
		handle,
	   	CURLUPART_PATH, res, 
		0
	);
	if (CURLUE_OK != rc)
		throw std::runtime_error("Can't set CURLUPART_PATH.");

	strip_query_frag();
}

url::~url()
{
	if(handle)
		curl_url_cleanup(handle);
}

url::url(const url& other)
	: handle(
		other.handle ? curl_url_dup(other.handle) : nullptr
	)
{

}

url::url(url&& other) noexcept
	: handle(other.handle)
{
	other.handle = nullptr;
}

url& url::operator=(const url& right)
{
	if (handle)
		curl_url_cleanup(handle);
	
	handle = right.handle ?
		curl_url_dup(right.handle) : nullptr;
	return *this;
}

url& url::operator=(url&& right) noexcept
{
	if (handle)
		curl_url_cleanup(handle);
	
	handle = right.handle;
	right.handle = nullptr;
	return *this;
}

std::string url::get_essential() const 
{
	curl_str path;

	std::string authority{get_authority()};
	curl_url_get(
		handle, 
		CURLUPART_PATH, &path.str,
		0
	);
				  
	if (path)
		authority.append(path.str);

	return authority;
}

curl_str url::get_full() const 
{
	curl_str ret;

	curl_url_get(
		handle, 
		CURLUPART_URL, &ret.str, 
		0
	);

	return ret;
}

std::string url::get_authority() const 
{
	curl_str 
		user{get_user()}, passwd{get_passwd()}, 
		host{get_host()}, port{get_port()};

	std::string ret;
	// a good value to avoid too many small allocations.
	ret.reserve(32);
	if (user)
	{
		ret.append(user.str);
		if (passwd) 
		{
			ret += ":";
			ret.append(passwd);
		}
		ret += "@";
	}
	if (host)
	{
		ret.append(host.str);
	}
	if (port)
	{
		ret += ":";
		ret.append(port.str);
	}

	return ret;
}

curl_str url::get_scheme() const 
{
	curl_str ret;
	curl_url_get(
		handle, CURLUPART_SCHEME, &ret.str,
		0
	);
	return ret;
}
curl_str url::get_user() const 
{
	curl_str ret;
	curl_url_get(
		handle, CURLUPART_USER, &ret.str,
		0
	);
	return ret;
}
curl_str url::get_passwd() const 
{
	curl_str ret;
	curl_url_get(
		handle, CURLUPART_PASSWORD, &ret.str,
		0
	);
	return ret;
}
curl_str url::get_host() const 
{
	curl_str ret;
	curl_url_get(
		handle, CURLUPART_HOST, &ret.str,
		0
	);
	return ret;
}
curl_str url::get_port() const 
{
	curl_str ret;
	curl_url_get(
		handle, CURLUPART_PORT, &ret.str,
		0
	);
	return ret;
}
curl_str url::get_path() const 
{
	curl_str ret;
	curl_url_get(
		handle, CURLUPART_PATH, &ret.str,
		0
	);
	return ret;
}

void url::strip_query_frag()
{
	// When unsetting, don't use "".
	// Use nullptr instead.
	curl_url_set(
		handle,
		CURLUPART_QUERY, nullptr,
		0
	);
	curl_url_set(
		handle,
		CURLUPART_FRAGMENT, nullptr,
		0
	);
}

url url::url_resolution(
	const class url& base, const std::string &relative
) {
	CURLUcode rc;
	
	// First, check if base is absolute.
	// According to RFC 3986, section 3
	// and section 4.2,
	// I just check if it has a nonempty scheme.
	curl_str scheme_base {base.get_scheme()};
	if (!scheme_base)
		throw std::logic_error("base is not absolute.");

	// Next, check if relative is absolute.
	// If so, then I just return it.
	std::optional<url> test_scheme;
	try 
	{
		test_scheme.emplace(relative);
	}
	catch (const std::runtime_error& e)
	{
		// if relative has no scheme,
		// an exception will be thrown.
		// do nothing here.
	}
	if (test_scheme) // relative is absolute, as construction was successful.
		return test_scheme.value();
	

	// Here's a simplified version of RFC 3986 section 5.2 algorithm.
	// In particular, query and fragments are ignored.
	auto* ret = curl_url();
	// 1. use base.scheme.
	curl_url_set(
		ret, CURLUPART_SCHEME, scheme_base.str, 0 
	);

	// 2. use base.authority if rel does not have it.
	curl_str authority_rel, path_base;
	rc = curl_url_get(
		ret, CURLUPART_HOST, &authority_rel.str, 0
	);
	if (CURLUE_NO_HOST == rc)
	{
		// use base authority.
		curl_url_set(
			ret, CURLUPART_HOST,
			base.get_host().str,
			0
		);
		curl_url_set(
			ret, CURLUPART_PORT,
			base.get_port().str,
			0
		);
		curl_url_set(
			ret, CURLUPART_USER,
			base.get_user().str,
			0
		);
	}

	// finally, handle path.
	std::string final_path;
	// if relative path starts with '/', then it's an absolute path. just use
	// it.
	if (relative[0] == '/')
		final_path = std::move(relative);
	else // otherwise, concat them. 
	{
		path_base = base.get_path();
		// RFC 3986: if path_base is empty,
		// then return "/" + relative.
		// Otherwise, remove the last segment of path_base,
		// and concat them.
		if (!path_base || std::string(path_base).empty())
		{
			final_path += '/';
			final_path += relative;
		}
		else 
		{
			final_path += path_base.str; 
			auto last_slash = final_path.find_last_of('/');
			if (std::string::npos != last_slash)
				final_path.erase(last_slash+1);
			final_path += relative;
		}

	}
	
	// first, we set path back.
	curl_url_set(
		ret, CURLUPART_PATH, final_path.c_str(), 0 
	);
	// then, retrieve the url, and return a new CURLU*.
	// Doing this will cause curl to normalize the path.
	curl_str raw_url;
	curl_url_get(
		ret, CURLUPART_URL, &raw_url.str, 0
	);
	return url(raw_url.str);

}
