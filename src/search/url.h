#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the url class that 
 * represents a url
 *
 * @author Guanyuming He
 */

#include <string>

// forward decl of CURLU;
struct Curl_URL;
typedef struct Curl_URL CURLU;

// strings returned by libcurl needs to be freed
// by curl_free().
struct curl_str 
{
public:
	curl_str() : str(nullptr) {}
	curl_str(char* str) : str(str) {}

	// can't copy. libcurl does not provide functions to do that.
	curl_str(const curl_str&) = delete;
	curl_str(curl_str&& other) noexcept:
		str(other.str)
	{ other.str = nullptr; }
	// can't copy. libcurl does not provide functions to do that.
	curl_str& operator=(const curl_str&) = delete;
	curl_str& operator=(curl_str&&) noexcept;

	~curl_str();

public:
	operator bool() const { return nullptr != str; }
	// Please use operator bool() to check before using this.
	// Like the habit of the standard library,
	// I don't perform checks here.
	operator std::string() const 
	{ return std::string(str); }

public:
	char* str;
};

/**
 * Strictly speaking, curl handles uris, the set of which is a 
 * superset of that of urls.
 *
 * URIs are defined in RFC 3986, see 
 * https://datatracker.ietf.org/doc/html/rfc3986.
 * For historical reasons, people still use URLs to refer to URIs.
 *
 * Important: I discard all query and fragment (as defined in RFC 3986)
 * in the url. That is, I only care about the scheme, authority,
 * and the path.
 */
class url final 
{
public:
	// the handle must be valid, except in a moved-from obj.
	url() = delete;
	~url();

	// I intend the class to be stored,
	// so I need to complete its copy & move functions.
	url(const url&);
	url(url&&) noexcept;
	url& operator=(const url&);
	url& operator=(url&&) noexcept;

	url(CURLU* handle):
		handle(handle) {}
	url(const char* str);
	url(const std::string& str) :
		url(str.c_str()) {}
	// some uris are relative within a host.
	// we need to handle that.
	url(
		const char* host, const char* res, 
		const char* scheme = "https"
	);

public:
	// @returns the handle
	CURLU* get_handle() 
	{ return handle; }
	// @returns a const handle
	const CURLU* get_handle() const 
	{ return (const CURLU*)handle; }

	// @returns the essential part, i.e., 
	// authority + path, of the url.
	// If that is unavailable, then "" is returned.
	std::string get_essential() const;
	// @returns authority, which is 
	// [ userinfo "@" ] host [ ":" port ]
	// where userinfo can be user:passwd
	std::string get_authority() const;

	// @returns the full url.
	curl_str get_full() const;
	// @returns the scheme, or null curl_str if does not exist.
	curl_str get_scheme() const;
	// @returns the user, or null curl_str if does not exist.
	curl_str get_user() const;
	// @returns the passwd, or null curl_str if does not exist.
	curl_str get_passwd() const;
	// @returns the host, or null curl_str if does not exist.
	curl_str get_host() const;
	// @returns the port, or null curl_str if does not exist.
	curl_str get_port() const;

	// @returns the path, or null curl_str if does not exist.
	curl_str get_path() const;
	// query and fragment are stripped,
	// so no getter is provided for them.

private:
	CURLU* handle;

	/**
	 * As I said in the class comment,
	 * I will discard the query and fragment part of a URL.
	 * This function does that.
	 */
	void strip_query_frag();

public:
	/**
	 * Given a base url in the absolute form,
	 * and a url that may or may not be relative,
	 * decide if it is, and if it is, run the uri resolution algorithm as
	 * defined in RFC 3986
	 * https://datatracker.ietf.org/doc/html/rfc3986#section-5
	 *
	 * @param base base url. Must be absolute. (Since I strip the fragment
	 * part, every full URI is absolute.)
	 * @param relative relative url found in a resource of base.
	 * May or may not be relative.
	 * @returns the param "relative" in its absolute form
	 * @throws std::logic_error if base is not in absolute form.
	 */
	static url
	url_resolution(
		const class url& base,
		const std::string& relative
	);

};
