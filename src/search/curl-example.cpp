/**
 * My test ground for curl functions.
 */
#include <curl/urlapi.h>
#include <iostream>
extern "C" {
#include <curl/curl.h>
}

int main(int argc, char *argv[])
{
	std::string orig{"https://datatracker.ietf.org/doc/html/rfc9112?id=1#request.target"};

	std::cout << "Original url\n" <<
		orig << std::endl;

	CURLU* uri = curl_url();
	curl_url_set(
		uri,
		CURLUPART_URL, 
		orig.c_str(),
		0
	);

	char* host, *res, *query, *frag;

	CURLUcode rc;
	rc = curl_url_get(
		uri,
		CURLUPART_HOST, &host,
		0
	);
	rc = curl_url_get(
		uri, 
		CURLUPART_PATH, &res,
		0
	);
	rc = curl_url_get(
		uri, 
		CURLUPART_QUERY, &query,
		0
	);
	rc = curl_url_get(
		uri, 
		CURLUPART_FRAGMENT, &frag,
		0
	);

 
	std::string ret;
	auto len1 = std::char_traits<char>::length(host);
	auto len2 = std::char_traits<char>::length(res);
	ret.reserve(len1+len2);
	ret.append(host, len1);
	ret.append(res, len2);
	std::cout << "Without scheme\n";
	std::cout << host << res << query << frag << std::endl;
	std::cout << "Host and path only\n";
	std::cout << ret << std::endl;

	// see what if I strip it out of query and frag
	curl_url_set(
		uri, CURLUPART_FRAGMENT, nullptr, 0
	);
	curl_url_set(
		uri, CURLUPART_QUERY, nullptr, 0
	);
	char* stripped;
	curl_url_get(
		uri, CURLUPART_URL, &stripped, 0
	);
	std::cout << "Stripped url\n";
	std::cout << stripped << std::endl;

	// reassemble test
	CURLU* uri1 = curl_url();
	curl_url_set(
		uri1,
		CURLUPART_HOST, 
		host,
		0
	);
	curl_url_set(
		uri1,
		CURLUPART_PATH, 
		res,
		0
	);
	curl_url_set(
		uri1,
		CURLUPART_SCHEME, 
		"https",
		0
	);
	char* reassembled;
	curl_url_get(
		uri1,
		CURLUPART_URL,
		&reassembled,
		0
	);
	std::cout << "Reassembled with scheme, host, and path\n";
	std::cout << reassembled << std::endl;

	// does relative path gets calculated?
	auto* uri2 = curl_url();
	curl_url_set(
		uri2, CURLUPART_URL,
		"https://a/b/c/d/../../g",
		0
	);
	char* path2;
	curl_url_get(
		uri2, CURLUPART_PATH, &path2, 0
	);
	std::cout << "Will path be cleaned?\n" <<
		path2 << std::endl;

	curl_url_set(
		uri2, CURLUPART_PATH, "/b/c/d/../../g", 0
	);
	curl_url_get(
		uri2, CURLUPART_PATH, &path2, 0
	);
	std::cout << "Will path be cleaned after setting?\n" <<
		path2 << std::endl;

	
	return 0;
}
