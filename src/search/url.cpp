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
	class url ret{relative};
	curl_str scheme_rel;
	rc = curl_url_get(
		ret.get_handle(), 
		CURLUPART_SCHEME, &scheme_rel.str, 0
	);
	if (CURLUE_OK == rc) // relative is absolute.
		return ret;
	
	// Now, relative is not absolute.
	// Perform the resolution algorithm in 
	// https://datatracker.ietf.org/doc/html/rfc3986#section-5.2
	
	// 1. use base.scheme 
	curl_url_set(
		ret.get_handle(), CURLUPART_SCHEME, scheme_base.str, 0
	);

	// 2. use base.authority if rel does not have it.
	curl_str authority_rel, path_rel, path_base;
	rc = curl_url_get(
		ret.get_handle(), CURLUPART_HOST, &authority_rel.str, 0
	);
	if (CURLUE_NO_HOST == rc)
	{
		// use base authority.
		curl_url_set(
			ret.get_handle(), CURLUPART_HOST,
			base.get_host().str,
			0
		);
	}

	// finally, concat base path with relative path.
	curl_url_get(
		ret.get_handle(), CURLUPART_PATH, &path_rel.str, 0
	);
	if (!path_rel) // doesn't have a path.
		return ret;

	path_base = base.get_path();
	// see if there's a '/' at the end of path_base
	// or at the start of path_rel.
	std::string concat_path(path_base);
	if (
		concat_path.back() != '/' &&
		path_rel.str[0] != '/'
	) {
		concat_path.push_back('/');
	}
	concat_path.append(path_rel.str);
	
	// first, we set path back.
	curl_url_set(
		ret.get_handle(), CURLUPART_PATH, concat_path.c_str(), 0 
	);
	// then, retrieve the url, and return a new CURLU*.
	// Doing this will cause curl to normalize the path.
	curl_str raw_url;
	curl_url_get(
		ret.get_handle(), CURLUPART_URL, &raw_url.str, 0
	);
	return url(raw_url.str);

}
