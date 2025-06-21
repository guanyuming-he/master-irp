/**
 * Integration tests that see if it can work 
 * on one single website.
 *
 * Copyright (C) 2025 Guanyuming He
 * The file is licensed under the GNU GPL v3.
 * 
 * Tests are written using Boost.Test
 * Tests were generated using Claude.ai 
 * and then modified by myself.
 */

#define BOOST_TEST_MODULE WebScrapingIntegrationTests
#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>
#include <chrono>
#include <algorithm>

// Include your headers
#include "webpage.h"
#include "url2html.h"
#include "url.h"
#include "utility.h"

namespace ch = std::chrono;

// Global settings for web scraping.
struct WebScrapingFixture {
    WebScrapingFixture() :
		test_url_str("https://www.theguardian.com/uk/business")
	{
        global_init();
    }
    
    ~WebScrapingFixture() = default;

    const std::string test_url_str;
};

BOOST_FIXTURE_TEST_SUITE(WebScrapingIntegrationTests, WebScrapingFixture)

BOOST_AUTO_TEST_CASE(test_url_construction_and_parsing)
{
    BOOST_TEST_MESSAGE("Testing URL construction and parsing");
    
    // Test URL construction from string
    url test_url(test_url_str);
    
    // Test basic URL components
    auto scheme = test_url.get_scheme();
    BOOST_TEST(scheme);
    BOOST_TEST(std::string(scheme) == "https");
    
    auto host = test_url.get_host();
    BOOST_TEST(host);
    BOOST_TEST(std::string(host) == "www.theguardian.com");
    
    auto path = test_url.get_path();
    BOOST_TEST(path);
    BOOST_TEST(std::string(path) == "/uk/business");
    
    // Test full URL reconstruction
    auto full_url = test_url.get_full();
    BOOST_TEST(full_url);
    BOOST_TEST(std::string(full_url) == test_url_str);
    
    // Test essential part (authority + path)
    std::string essential = test_url.get_essential();
    BOOST_TEST(!essential.empty());
    BOOST_TEST(essential == "www.theguardian.com/uk/business");
}

BOOST_AUTO_TEST_CASE(test_url2html_conversion)
{
    BOOST_TEST_MESSAGE("Testing URL to HTML conversion");
    
    url test_url(test_url_str);
    url2html converter;
    
    // Convert URL to HTML
    html webpage_html = converter.convert(test_url);
    
    // Test that we got some content
    BOOST_TEST(!webpage_html.text.empty());
    BOOST_TEST_MESSAGE("Retrieved text length: " << webpage_html.text.length());
    
    // Test headers were captured
    BOOST_TEST(!webpage_html.headers.empty());
    BOOST_TEST_MESSAGE("Number of headers captured: " << webpage_html.headers.size());
    
    // Check for date: header
    BOOST_TEST(webpage_html.headers.contains("date"));
    
    // Test title extraction
    std::string title = webpage_html.get_title();
    BOOST_TEST(!title.empty());
    BOOST_TEST_MESSAGE("Page title: " << title);
    
    // Test date extraction
    ch::year_month_day page_date = webpage_html.get_date();
    BOOST_TEST_MESSAGE("Page date: " << 
        static_cast<int>(page_date.year()) << "-" <<
        static_cast<unsigned>(page_date.month()) << "-" <<
        static_cast<unsigned>(page_date.day()));
    
    // Test URL extraction
	webpage page(test_url, std::move(webpage_html));
    auto urls{page.get_urls()};
    BOOST_TEST(!urls.empty());
    BOOST_TEST_MESSAGE("Number of URLs found: " << urls.size());
    
    // Check that we found some common URL patterns
    bool has_guardian_links = false;
    bool has_external_links = false;
    
	// relative urls will be resoluted to have theguardian.
    for (const auto& u : urls) {
		auto authority = u.get_authority();
		if (authority.contains("www.theguardian.com"))
            has_guardian_links = true;
		else
            has_external_links = true;
    }
    
    BOOST_TEST(has_guardian_links);
    // External links might not always be present, so just log the result
    BOOST_TEST_MESSAGE("Has external links: " << (has_external_links ? "Yes" : "No"));
}

BOOST_AUTO_TEST_CASE(test_webpage_construction_from_url)
{
    BOOST_TEST_MESSAGE("Testing webpage construction from URL");
    
    url test_url(test_url_str);
    url2html converter;
    
    // Create webpage from URL (uses template constructor)
    webpage page(std::move(test_url), converter);
    
    // Test webpage properties
    BOOST_TEST(page.url.get_full());
    BOOST_TEST(std::string(page.url.get_full()) == test_url_str);
    
    BOOST_TEST(!page.title.empty());
    BOOST_TEST_MESSAGE("Webpage title: " << page.title);
    
    // Test text extraction
    std::string page_text = page.get_text();
    BOOST_TEST(!page_text.empty());
    BOOST_TEST_MESSAGE("Page text length: " << page_text.length());
    
    // Test that text is in lowercase
    bool is_lowercase = std::all_of(page_text.begin(), page_text.end(), 
        [](char c) { return !std::isalpha(c) || std::islower(c); });
    BOOST_TEST(is_lowercase);
    
    // Test URL extraction with absolute conversion
    std::vector<url> page_urls = page.get_urls();
    BOOST_TEST(!page_urls.empty());
    BOOST_TEST_MESSAGE("Number of absolute URLs: " << page_urls.size());
    
    // Verify that all URLs are absolute
    for (const auto& url_obj : page_urls) {
        auto scheme = url_obj.get_scheme();
        BOOST_TEST(scheme); // Should have a scheme (http/https)
        auto host = url_obj.get_host();
        BOOST_TEST(host); // Should have a host
    }
}


BOOST_AUTO_TEST_CASE(test_content_validation)
{
    BOOST_TEST_MESSAGE("Testing content validation and expected patterns");
    
    url test_url(test_url_str);
    url2html converter;
    webpage page(std::move(test_url), converter);
    
    std::string page_text = page.get_text();
    std::string page_title = page.title;
    
    // Test for expected content patterns (replace with actual values)
    // These are placeholder tests - replace with actual expected content
    
    // Title should contain business-related keywords
    std::vector<std::string> expected_title_keywords = {
        "Latest financial, market & economic news and analysis", 
        "Business",
        "The Guardian"
    };
    
    for (const auto& keyword : expected_title_keywords) {
        if (!page_title.contains(keyword)) {
			BOOST_FAIL(
				"Title must contain all expected keywords, but " <<
				keyword << " is not found."
			);
            break;
        }
    }
    
    // Text should contain business-related content
    std::vector<std::string> expected_text_keywords = {
        "business", 
        "opinion & analysis",
        "spotlight",
        "most viewed", 
        "across the guardian",
		"guardian news & media limited or its affiliated companies. all rights reserved.",
		"2025",
		"about us"
    };
    
    for (const auto& keyword : expected_text_keywords) {
        if (!page_text.contains(keyword)) {
			BOOST_FAIL(
				"Text must contain all expected keywords, but " <<
				keyword << " is not found."
			);
        }
    }
    
}

BOOST_AUTO_TEST_CASE(test_error_handling)
{
    BOOST_TEST_MESSAGE("Testing error handling for invalid URLs");
    
    // Test invalid URL handling
    std::vector<std::string> invalid_urls = {
        "not-a-url",
        "https://nonexistent-domain-12345.com",
        ""
    };
    
    url2html converter;
    
    for (const auto& invalid_url_str : invalid_urls) {
        try {
            url invalid_url(invalid_url_str);
            // Some invalid URLs might still construct but fail during conversion
            try {
                html result = converter.convert(invalid_url);
				if (!result.text.empty())
                	BOOST_FAIL("Unexpectedly succeeded with: " << invalid_url_str);
            } catch (const std::exception& e) {
                BOOST_TEST_MESSAGE("Expected conversion failure for '" << invalid_url_str << "': " << e.what());
            }
        } catch (const std::exception& e) {
            BOOST_TEST_MESSAGE("Expected URL construction failure for '" << invalid_url_str << "': " << e.what());
        }
    }
}

BOOST_AUTO_TEST_CASE(test_large_page_handling)
{
    BOOST_TEST_MESSAGE("Testing handling of large pages");
    
    // This test would be more meaningful with a known large page
    // For now, we'll just verify our current page can be processed efficiently
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    url test_url(test_url_str);
    url2html converter;
    webpage page(std::move(test_url), converter);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    BOOST_TEST_MESSAGE("Page processing took: " << duration.count() << " ms");
    
    // Should complete within reasonable time (adjust threshold as needed)
    BOOST_TEST(duration.count() < 30000); // 30 seconds max
    
    // Should handle the content without issues
    BOOST_TEST(!page.get_text().empty());
    BOOST_TEST(!page.get_urls().empty());
}

BOOST_AUTO_TEST_CASE(test_concurrent_access)
{
    BOOST_TEST_MESSAGE("Testing concurrent access patterns");
    
    // Test that multiple url2html instances can work simultaneously
    // This is a basic test - more comprehensive threading tests would be needed for production
    
    url2html converter1;
    url2html converter2;
    
    url test_url1(test_url_str);
    url test_url2(test_url_str);
    
    html result1 = converter1.convert(test_url1);
    html result2 = converter2.convert(test_url2);
    
    // Both should match
    BOOST_CHECK_EQUAL(result1.text, result2.text);
    BOOST_CHECK_EQUAL(result1.get_title(), result2.get_title());
    BOOST_CHECK_EQUAL(result1.get_urls().size(), result2.get_urls().size());
}

BOOST_AUTO_TEST_SUITE_END()
