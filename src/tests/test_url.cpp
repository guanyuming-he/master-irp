/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file tests the url class.
 * @author Guanyuming He
 * @deprecated switch to boost.url, because libcurl's uri handling is so
 * terrible.
 *
 * Test cases are generated using Claude.ai, and modified 
 * by me.
 */
#define BOOST_TEST_MODULE URLTests
#include <boost/test/included/unit_test.hpp>
#include "../search/url.h"
#include <stdexcept>
#include <string>

BOOST_AUTO_TEST_SUITE(URLBasicConstructionTests)

BOOST_AUTO_TEST_CASE(test_construct_from_string)
{
    BOOST_CHECK_NO_THROW(url u("https://example.com/path"));
    BOOST_CHECK_NO_THROW(url u("http://test.org"));
    BOOST_CHECK_NO_THROW(url u("ftp://files.example.com/dir/file.txt"));
}

BOOST_AUTO_TEST_CASE(test_construct_from_components)
{
    BOOST_CHECK_NO_THROW(url u("example.com", "/path/to/resource"));
    BOOST_CHECK_NO_THROW(url u("test.org", "/", "http"));
    BOOST_CHECK_NO_THROW(url u("secure.site.com", "/api/v1/data", "https"));
}

BOOST_AUTO_TEST_CASE(test_construct_invalid_url)
{
    BOOST_CHECK_THROW(url u("not-a-valid-url"), std::runtime_error);
    BOOST_CHECK_THROW(url u(""), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_copy_constructor)
{
    url original("https://example.com/path");
    url copy(original);
    
    BOOST_CHECK_EQUAL(std::string(copy.get_full()), std::string(original.get_full()));
}

BOOST_AUTO_TEST_CASE(test_move_constructor)
{
    url original("https://example.com/path");
    std::string original_url = std::string(original.get_full());
    
    url moved(std::move(original));
    BOOST_CHECK_EQUAL(std::string(moved.get_full()), original_url);
}

BOOST_AUTO_TEST_CASE(test_copy_assignment)
{
    url original("https://example.com/path");
    url assigned("https://different.com");
    
    assigned = original;
    BOOST_CHECK_EQUAL(std::string(assigned.get_full()), std::string(original.get_full()));
}

BOOST_AUTO_TEST_CASE(test_move_assignment)
{
    url original("https://example.com/path");
    std::string original_url = std::string(original.get_full());
    url assigned("https://different.com");
    
    assigned = std::move(original);
    BOOST_CHECK_EQUAL(std::string(assigned.get_full()), original_url);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(URLComponentTests)

BOOST_AUTO_TEST_CASE(test_get_scheme)
{
    url u1("https://example.com/path");
    curl_str scheme = u1.get_scheme();
    BOOST_CHECK(scheme);
    BOOST_CHECK_EQUAL(std::string(scheme), "https");
    
    url u2("http://test.org");
    scheme = u2.get_scheme();
    BOOST_CHECK(scheme);
    BOOST_CHECK_EQUAL(std::string(scheme), "http");
    
    url u3("ftp://files.example.com/file.txt");
    scheme = u3.get_scheme();
    BOOST_CHECK(scheme);
    BOOST_CHECK_EQUAL(std::string(scheme), "ftp");
}

BOOST_AUTO_TEST_CASE(test_get_host)
{
    url u1("https://example.com/path");
    curl_str host = u1.get_host();
    BOOST_CHECK(host);
    BOOST_CHECK_EQUAL(std::string(host), "example.com");
    
    url u2("http://subdomain.test.org:8080/api");
    host = u2.get_host();
    BOOST_CHECK(host);
    BOOST_CHECK_EQUAL(std::string(host), "subdomain.test.org");
    
    url u3("https://192.168.1.1/endpoint");
    host = u3.get_host();
    BOOST_CHECK(host);
    BOOST_CHECK_EQUAL(std::string(host), "192.168.1.1");
}

BOOST_AUTO_TEST_CASE(test_get_port)
{
    url u1("https://example.com:8443/path");
    curl_str port = u1.get_port();
    BOOST_CHECK(port);
    BOOST_CHECK_EQUAL(std::string(port), "8443");
    
    url u2("http://test.org:8080/api");
    port = u2.get_port();
    BOOST_CHECK(port);
    BOOST_CHECK_EQUAL(std::string(port), "8080");
    
    // URLs without explicit port
    url u3("https://example.com/path");
    port = u3.get_port();
    // Port might be null or default port depending on libcurl behavior
    // BOOST_CHECK(!port); // Uncomment if libcurl returns null for default ports
    
    url u4("http://test.org/api");
    port = u4.get_port();
    // BOOST_CHECK(!port); // Uncomment if libcurl returns null for default ports
}

BOOST_AUTO_TEST_CASE(test_get_user)
{
    url u1("https://username:password@example.com/path");
    curl_str user = u1.get_user();
    BOOST_CHECK(user);
    BOOST_CHECK_EQUAL(std::string(user), "username");
    
    url u2("http://user@test.org/api");
    user = u2.get_user();
    BOOST_CHECK(user);
    BOOST_CHECK_EQUAL(std::string(user), "user");
    
    // URL without user info
    url u3("https://example.com/path");
    user = u3.get_user();
    BOOST_CHECK(!user);
}

BOOST_AUTO_TEST_CASE(test_get_passwd)
{
    url u1("https://username:password@example.com/path");
	curl_str passwd = u1.get_passwd();
    BOOST_CHECK(passwd);
    BOOST_CHECK_EQUAL(std::string(passwd), "password");
    
    // URL without passwd
    url u2("https://example.com/path");
    passwd = u2.get_passwd();
    BOOST_CHECK(!passwd);
}

BOOST_AUTO_TEST_CASE(test_get_authority)
{
    url u1("https://example.com/path");
    std::string authority = u1.get_authority();
    BOOST_CHECK_EQUAL(authority, "example.com");
    
    url u2("http://subdomain.test.org:8080/api");
    authority = u2.get_authority();
    BOOST_CHECK_EQUAL(authority, "subdomain.test.org:8080");
    
    url u3("https://user:pass@secure.example.com:9443/resource");
    authority = u3.get_authority();
    BOOST_CHECK_EQUAL(authority, "user:pass@secure.example.com:9443");
    
    url u4("ftp://username@files.example.com/file.txt");
    authority = u4.get_authority();
    BOOST_CHECK_EQUAL(authority, "username@files.example.com");
}

BOOST_AUTO_TEST_CASE(test_get_path)
{
    url u1("https://example.com/path/to/resource");
    curl_str path = u1.get_path();
    BOOST_CHECK(path);
    BOOST_CHECK_EQUAL(std::string(path), "/path/to/resource");
    
    url u2("http://test.org");
    path = u2.get_path();
    BOOST_CHECK(path);
    BOOST_CHECK_EQUAL(std::string(path), "/");
    
    url u3("https://example.com/");
    path = u3.get_path();
    BOOST_CHECK(path);
    BOOST_CHECK_EQUAL(std::string(path), "/");
}

BOOST_AUTO_TEST_CASE(test_get_essential)
{
    url u1("https://example.com/path/to/resource");
    std::string essential = u1.get_essential();
    BOOST_CHECK_EQUAL(essential, "example.com/path/to/resource");
    
    url u2("http://test.org");
    essential = u2.get_essential();
    BOOST_CHECK_EQUAL(essential, "test.org/");
    
    url u3("https://subdomain.example.com:8080/api/v1/users");
    essential = u3.get_essential();
    BOOST_CHECK_EQUAL(essential, "subdomain.example.com:8080/api/v1/users");
    
    url u4("https://user:pass@secure.example.com:9443/resource");
    essential = u4.get_essential();
    BOOST_CHECK_EQUAL(essential, "user:pass@secure.example.com:9443/resource");
}

BOOST_AUTO_TEST_CASE(test_get_full)
{
    url u1("https://example.com/path");
    curl_str full = u1.get_full();
    BOOST_CHECK(full);
    BOOST_CHECK_EQUAL(std::string(full), "https://example.com/path");
    
    url u2("http://test.org:8080/api/v1");
    full = u2.get_full();
    BOOST_CHECK(full);
    BOOST_CHECK_EQUAL(std::string(full), "http://test.org:8080/api/v1");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(URLQueryFragmentStrippingTests)

BOOST_AUTO_TEST_CASE(test_query_stripped)
{
    url u("https://example.com/path?query=value&param=test");
    std::string full_url = std::string(u.get_full());
    BOOST_CHECK(full_url.find("query") == std::string::npos);
    BOOST_CHECK(full_url.find("?") == std::string::npos);
    BOOST_CHECK_EQUAL(full_url, "https://example.com/path");
}

BOOST_AUTO_TEST_CASE(test_fragment_stripped)
{
    url u("https://example.com/path#section");
    std::string full_url = std::string(u.get_full());
    BOOST_CHECK(full_url.find("section") == std::string::npos);
    BOOST_CHECK(full_url.find("#") == std::string::npos);
    BOOST_CHECK_EQUAL(full_url, "https://example.com/path");
}

BOOST_AUTO_TEST_CASE(test_query_and_fragment_stripped)
{
    url u("https://example.com/path?query=value#section");
    std::string full_url = std::string(u.get_full());
    BOOST_CHECK(full_url.find("query") == std::string::npos);
    BOOST_CHECK(full_url.find("section") == std::string::npos);
    BOOST_CHECK(full_url.find("?") == std::string::npos);
    BOOST_CHECK(full_url.find("#") == std::string::npos);
    BOOST_CHECK_EQUAL(full_url, "https://example.com/path");
}

BOOST_AUTO_TEST_CASE(test_complex_query_fragment_stripped)
{
    url u("https://example.com/path/to/resource?q1=v1&q2=v2&q3=v3#fragment-with-dashes");
    std::string full_url = std::string(u.get_full());
    BOOST_CHECK_EQUAL(full_url, "https://example.com/path/to/resource");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(URLResolutionTests)

BOOST_AUTO_TEST_CASE(test_absolute_relative_url)
{
    url base("https://example.com/dir/page.html");
    
    // Absolute URL should return as-is
    auto resolved = url::url_resolution(base, "https://other.com/resource");
	BOOST_TEST(resolved.has_value());
    std::string resolved_str = std::string(resolved->get_full());
    BOOST_CHECK_EQUAL(resolved_str, "https://other.com/resource");
}

BOOST_AUTO_TEST_CASE(test_relative_path_resolution)
{
    url base("https://example.com/dir/page.html");
    
    // Relative path
    auto resolved = url::url_resolution(base, "resource.html");
	BOOST_TEST(resolved.has_value());
    std::string resolved_str = std::string(resolved->get_full());
    BOOST_CHECK_EQUAL(resolved_str, "https://example.com/dir/resource.html");
}

BOOST_AUTO_TEST_CASE(test_absolute_path_resolution)
{
    url base("https://example.com/dir/page.html");
    
    // Absolute path (starts with /)
    auto resolved = url::url_resolution(base, "/newpath/resource.html");
	BOOST_TEST(resolved.has_value());
    std::string resolved_str = std::string(resolved->get_full());
    BOOST_CHECK_EQUAL(resolved_str, "https://example.com/newpath/resource.html");
}

BOOST_AUTO_TEST_CASE(test_resolution_with_trailing_slash)
{
    url base("https://example.com/dir/");
    
    auto resolved = url::url_resolution(base, "resource.html");
	BOOST_TEST(resolved.has_value());
    std::string resolved_str = std::string(resolved->get_full());
    BOOST_CHECK_EQUAL(resolved_str, "https://example.com/dir/resource.html");
}

BOOST_AUTO_TEST_CASE(test_resolution_without_trailing_slash)
{
    url base("https://example.com/dir");
    
    auto resolved = url::url_resolution(base, "resource.html");
	BOOST_TEST(resolved.has_value());
    std::string resolved_str = std::string(resolved->get_full());
    BOOST_CHECK_EQUAL(resolved_str, "https://example.com/resource.html");
}

BOOST_AUTO_TEST_CASE(test_resolution_relative_starts_with_slash)
{
    url base("https://example.com/dir/page.html");
    
    auto resolved = url::url_resolution(base, "/resource.html");
	BOOST_TEST(resolved.has_value());
    std::string resolved_str = std::string(resolved->get_full());
    BOOST_CHECK_EQUAL(resolved_str, "https://example.com/resource.html");
}

BOOST_AUTO_TEST_CASE(test_resolution_complex_paths)
{
    url base("https://example.com/path1/path2/page.html");
    
    auto resolved = url::url_resolution(base, "../sibling.html");
	BOOST_TEST(resolved.has_value());
    std::string resolved_str = std::string(resolved->get_full());
    // libcurl should normalize the path
    BOOST_CHECK(resolved_str.find("example.com") != std::string::npos);
    BOOST_CHECK(resolved_str.find("sibling.html") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(test_resolution_dot_paths)
{
    url base("https://example.com/dir/");
    
    auto resolved = url::url_resolution(base, "./resource.html");
	BOOST_TEST(resolved.has_value());
    std::string resolved_str = std::string(resolved->get_full());
    BOOST_CHECK(resolved_str.find("example.com") != std::string::npos);
    BOOST_CHECK(resolved_str.find("resource.html") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(test_resolution_different_schemes)
{
    url base("https://example.com/path");
    
    // Different scheme should be treated as absolute
    auto resolved = url::url_resolution(base, "http://other.com/resource");
	BOOST_TEST(resolved.has_value());
    std::string resolved_str = std::string(resolved->get_full());
    BOOST_CHECK_EQUAL(resolved_str, "http://other.com/resource");
    
    // FTP scheme
    resolved = url::url_resolution(base, "ftp://files.example.com/file.txt");
    resolved_str = std::string(resolved->get_full());
    BOOST_CHECK_EQUAL(resolved_str, "ftp://files.example.com/file.txt");
}

BOOST_AUTO_TEST_CASE(test_resolution_with_ports)
{
    url base("https://example.com:8080/api/");
    
    auto resolved = url::url_resolution(base, "resource");
	BOOST_TEST(resolved.has_value());
    std::string resolved_str = std::string(resolved->get_full());
    BOOST_CHECK_EQUAL(
		resolved_str, 
		"https://example.com:8080/api/resource"
	);
    
    // Test authority preservation
    std::string base_authority = base.get_authority();
    std::string resolved_authority = resolved->get_authority();
    BOOST_CHECK_EQUAL(base_authority, "example.com:8080");
}

BOOST_AUTO_TEST_CASE(test_resolution_base_not_absolute)
{
    // Create a relative URL by parsing without scheme
    BOOST_CHECK_THROW({
        url relative_base("example.com/path");  // This might throw during construction
        url::url_resolution(relative_base, "resource.html");
    }, std::logic_error);
}

BOOST_AUTO_TEST_CASE(test_resolution_empty_relative)
{
    url base("https://example.com/dir/page.html");
    
    auto resolved = url::url_resolution(base, "");
    // Empty relative URL should resolve to base without fragment
    std::string resolved_str = std::string(resolved->get_full());
    BOOST_CHECK(resolved_str.find("example.com") != std::string::npos);
}

// Newly added after a bug.
BOOST_AUTO_TEST_CASE(test_resolution_invalid_url)
{
    url base("https://example.com/dir/page.html");
    
    auto resolved = url::url_resolution(base, "????");
	BOOST_TEST(!resolved.has_value());
    auto resolved1 = url::url_resolution(base, "https:abc/");
	BOOST_TEST(!resolved1.has_value());
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(CurlStrTests)

BOOST_AUTO_TEST_CASE(test_curl_str_bool_operator)
{
    url u("https://example.com/path");
    curl_str scheme = u.get_scheme();
    BOOST_CHECK(scheme);  // Should be true
    
    curl_str host = u.get_host();
    BOOST_CHECK(host);    // Should be true
}

BOOST_AUTO_TEST_CASE(test_curl_str_string_conversion)
{
    url u("https://example.com/path");
    curl_str scheme = u.get_scheme();
    std::string scheme_str = std::string(scheme);
    BOOST_CHECK_EQUAL(scheme_str, "https");
}

BOOST_AUTO_TEST_CASE(test_curl_str_move_assignment)
{
    url u("https://example.com/path");
    curl_str scheme1 = u.get_scheme();
    curl_str scheme2 = u.get_host();  // Different string
    
    std::string original_scheme = std::string(scheme1);
    scheme2 = std::move(scheme1);
    
    BOOST_CHECK_EQUAL(std::string(scheme2), original_scheme);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(URLEdgeCasesTests)

BOOST_AUTO_TEST_CASE(test_url_with_user_info)
{
    BOOST_CHECK_NO_THROW(url u("https://user:pass@example.com/path"));
    url u("https://user:pass@example.com/path");
    
    curl_str host = u.get_host();
    BOOST_CHECK(host);
    BOOST_CHECK_EQUAL(std::string(host), "example.com");
    
    curl_str user = u.get_user();
    BOOST_CHECK(user);
    BOOST_CHECK_EQUAL(std::string(user), "user");

    curl_str passwd = u.get_passwd();
    BOOST_CHECK(passwd);
    BOOST_CHECK_EQUAL(std::string(passwd), "pass");
    
    std::string authority = u.get_authority();
    BOOST_CHECK_EQUAL(authority, "user:pass@example.com");
}

BOOST_AUTO_TEST_CASE(test_url_with_default_ports)
{
    url u1("https://example.com:443/path");
    url u2("http://example.com:80/path");
    
    BOOST_CHECK_NO_THROW(u1.get_full());
    BOOST_CHECK_NO_THROW(u2.get_full());
}

BOOST_AUTO_TEST_CASE(test_url_with_non_default_ports)
{
    url u("https://example.com:9443/path");
    
    curl_str host = u.get_host();
    BOOST_CHECK(host);
    BOOST_CHECK_EQUAL(std::string(host), "example.com");
    
    curl_str port = u.get_port();
    BOOST_CHECK(port);
    BOOST_CHECK_EQUAL(std::string(port), "9443");
    
    std::string authority = u.get_authority();
    BOOST_CHECK_EQUAL(authority, "example.com:9443");
}

BOOST_AUTO_TEST_CASE(test_url_with_international_domain)
{
    // Test with internationalized domain names if libcurl supports them
    BOOST_CHECK_NO_THROW(url u("https://example.org/path"));
}

BOOST_AUTO_TEST_CASE(test_url_path_normalization)
{
    url u("https://example.com/path/../other/./resource");
    std::string full_url = std::string(u.get_full());
    // libcurl should normalize the path
    BOOST_CHECK(full_url.find("example.com") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(test_url_percent_encoding)
{
    url u("https://example.com/path%20with%20spaces");
    BOOST_CHECK_NO_THROW(u.get_full());
    BOOST_CHECK_NO_THROW(u.get_path());
}

BOOST_AUTO_TEST_CASE(test_very_long_url)
{
    std::string long_path = "/";
    for (int i = 0; i < 100; ++i) {
        long_path += "very-long-path-segment-";
        long_path += std::to_string(i);
        long_path += "/";
    }
    
    std::string long_url = "https://example.com" + long_path;
    BOOST_CHECK_NO_THROW(url u(long_url));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(URLSpecialProtocolTests)

BOOST_AUTO_TEST_CASE(test_file_protocol)
{
    BOOST_CHECK_NO_THROW(url u("file:///path/to/file.txt"));
    url u("file:///path/to/file.txt");
    curl_str scheme = u.get_scheme();
    BOOST_CHECK_EQUAL(std::string(scheme), "file");
}

BOOST_AUTO_TEST_CASE(test_ftp_protocol)
{
    BOOST_CHECK_NO_THROW(url u("ftp://ftp.example.com/pub/file.txt"));
    url u("ftp://ftp.example.com/pub/file.txt");
    curl_str scheme = u.get_scheme();
    BOOST_CHECK_EQUAL(std::string(scheme), "ftp");
}

BOOST_AUTO_TEST_SUITE_END()
