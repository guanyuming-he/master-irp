/**
 * Unit tests for HTML parser and html class.
 * Copyright (C) 2025 Guanyuming He
 * The file is licensed under the GNU GPL v3.
 * 
 * Tests are written using Boost.Test
 * Tests were generated using Claude.ai 
 * and then modified by myself.
 */
#include <boost/test/tools/old/interface.hpp>
#define BOOST_TEST_MODULE HTMLParserTests
#include <boost/test/unit_test.hpp>

#include <cctype>

#include "../search/url2html.h"
#include "../search/webpage.h"
#include "../search/utility.h"

#include <chrono>
#include <map>
#include <string>
#include <vector>

namespace ch = std::chrono;


struct global_setup {
	global_setup() { global_init(); }
	~global_setup() = default;
};
BOOST_GLOBAL_FIXTURE(global_setup);

// Test fixture for parser tests
struct ParserFixture {
    ParserFixture() {
    }
    
    parser p;
};

// Test fixture for html tests
struct HtmlFixture {
    HtmlFixture() {
    }
    
    parser p;
    
    // Helper method to create html object from string
    html create_html(const std::string& html_content, 
                     const std::map<std::string, std::string>& headers = {}) {
		std::string text;
        auto* doc = p.parse(
            reinterpret_cast<const lxb_char_t*>(html_content.c_str()),
            html_content.size(),
		   	&text
        );
        return html(doc, std::move(headers), std::move(text));
    }
};

// Test data for various HTML documents
namespace test_data {
    const std::string basic_html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Test Page</title>
</head>
<body>
    <h1>Hello World</h1>
    <p>This is a test paragraph.</p>
</body>
</html>
)";

    const std::string html_with_links = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Links Test</title>
</head>
<body>
    <h1>Link Testing</h1>
    <a href="https://example.com">Example Link</a>
    <a href="/relative/path">Relative Link</a>
    <a href="mailto:test@example.com">Email Link</a>
    <a>Link without href</a>
    <p>Some text with <a href="https://github.com">GitHub</a> link.</p>
</body>
</html>
)";

	// Its links have whitespaces around.
    const std::string html_with_link_ws = 
"<!DOCTYPE html>                                                            "
"<html>                                                                     "
"<head>                                                                     "
"    <title>Links Test</title>                                              "
"</head>                                                                    "
"<body>                                                                     "
"    <h1>Link Testing</h1>                                                  "
"    <a href=\"https://example.com \">Example Link</a>                        "
"    <a href=\"/relative/path\t\n\">Relative Link</a>                         "
"    <a href=\"//example.com/abc\r\n\">Email Link</a>                  "
"    <a>Link without href</a>                                               "
"    <p>Some text with <a href=\" https://example.com/def\">GitHub</a> link.</p>    "
"</body>                                                                    "
"</html>                                                                    "
;
	// Some of its links are illformed
    const std::string html_with_illformed_links = 
"<!DOCTYPE html>                                                            "
"<html>                                                                     "
"<head>                                                                     "
"    <title>Links Test</title>                                              "
"</head>                                                                    "
"<body>                                                                     "
"    <h1>Link Testing</h1>                                                  "
"    <a href=\"123https:/example.com \">Example Link</a>                        "
"    <a href=\"/relative/path\t\n\">Relative Link</a>                         "
"    <a href=\"!*&^%$#@()\">Email Link</a>                  "
"    <a>Link without href</a>                                               "
"    <p>Some text with <a href=\" https://example.com/def\">GitHub</a> link.</p>    "
"</body>                                                                    "
"</html>                                                                    "
;

    const std::string html_with_text = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Text Extraction Test</title>
</head>
<body>
    <h1>Main Heading</h1>
    <p>First paragraph with some text.</p>
    <div>
        <h2>Subheading</h2>
        <p>Second paragraph in a div.</p>
        <ul>
            <li>List item 1</li>
            <li>List item 2</li>
        </ul>
    </div>
    <script>console.log('This should not appear in text');</script>
    <style>body { color: red; }</style>
</body>
</html>
)";

    const std::string empty_html = R"(
<!DOCTYPE html>
<html>
<head></head>
<body></body>
</html>
)";

    const std::string malformed_html = R"(
<html>
<head>
<title>Malformed HTML
<body>
<p>Missing closing tags
<div>
<a href="test.com">Link
</html>
)";

    const std::string html_special_chars = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Special Characters: &lt;&gt;&amp;&quot;</title>
</head>
<body>
    <h1>Testing &lt;special&gt; characters</h1>
    <p>Ampersand: &amp; Quote: &quot; Less than: &lt;</p>
    <a href="https://example.com?param=value&amp;other=test">Complex URL</a>
</body>
</html>
)";

    const std::string html_unicode = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Unicode Test: 测试页面</title>
    <meta charset="UTF-8">
</head>
<body>
    <h1>Unicode Content</h1>
    <p>Chinese: 你好世界</p>
    <p>Japanese: こんにちは</p>
    <p>Emoji: 😀🌟🎉</p>
    <a href="https://example.com/测试">Unicode URL</a>
</body>
</html>
)";
}

BOOST_FIXTURE_TEST_SUITE(ParserTests, ParserFixture)

BOOST_AUTO_TEST_CASE(parse_basic_html) {
    BOOST_REQUIRE_NO_THROW({
        auto* doc = p.parse(
            reinterpret_cast<const lxb_char_t*>(test_data::basic_html.c_str()),
            test_data::basic_html.size(), nullptr
        );
        BOOST_CHECK(doc != nullptr);
        lxb_html_document_destroy(doc);
    });
}

BOOST_AUTO_TEST_CASE(parse_empty_html) {
    BOOST_REQUIRE_NO_THROW({
        auto* doc = p.parse(
            reinterpret_cast<const lxb_char_t*>(test_data::empty_html.c_str()),
            test_data::empty_html.size(), nullptr
        );
        BOOST_CHECK(doc != nullptr);
        lxb_html_document_destroy(doc);
    });
}

BOOST_AUTO_TEST_CASE(parse_malformed_html) {
    // Parser should handle malformed HTML gracefully
    BOOST_REQUIRE_NO_THROW({
        auto* doc = p.parse(
            reinterpret_cast<const lxb_char_t*>(test_data::malformed_html.c_str()),
            test_data::malformed_html.size(), nullptr
        );
        BOOST_CHECK(doc != nullptr);
        lxb_html_document_destroy(doc);
    });
}

BOOST_AUTO_TEST_CASE(parse_unicode_html) {
    BOOST_REQUIRE_NO_THROW({
        auto* doc = p.parse(
            reinterpret_cast<const lxb_char_t*>(test_data::html_unicode.c_str()),
            test_data::html_unicode.size(), nullptr
        );
        BOOST_CHECK(doc != nullptr);
        lxb_html_document_destroy(doc);
    });
}

BOOST_AUTO_TEST_CASE(parse_null_input) {
    BOOST_CHECK_EQUAL(
        p.parse(nullptr, 0, nullptr),
		nullptr
    );
}

BOOST_AUTO_TEST_CASE(parse_empty_input) {
    BOOST_REQUIRE_NO_THROW({
        auto* doc = p.parse(
            reinterpret_cast<const lxb_char_t*>(""),
            0, nullptr
        );
        BOOST_CHECK(doc != nullptr);
        lxb_html_document_destroy(doc);
    });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(HtmlTests, HtmlFixture)

BOOST_AUTO_TEST_CASE(get_title_basic) {
    auto h = create_html(test_data::basic_html);
    BOOST_CHECK_EQUAL(h.get_title(), "Test Page");
}

BOOST_AUTO_TEST_CASE(get_title_empty) {
    auto h = create_html(test_data::empty_html);
    BOOST_CHECK_EQUAL(h.get_title(), "");
}

BOOST_AUTO_TEST_CASE(get_title_special_chars) {
    auto h = create_html(test_data::html_special_chars);
    std::string expected_title = "Special Characters: <>&\"";
    BOOST_CHECK_EQUAL(h.get_title(), expected_title);
}

BOOST_AUTO_TEST_CASE(get_title_unicode) {
    auto h = create_html(test_data::html_unicode);
    BOOST_CHECK_EQUAL(h.get_title(), "Unicode Test: 测试页面");
}

BOOST_AUTO_TEST_CASE(get_urls_basic) {
    auto h = create_html(test_data::html_with_links);
    auto urls = h.get_urls();
    
    BOOST_CHECK_EQUAL(urls.size(), 4); // 4 links with href attributes
    
    // Check specific URLs
    BOOST_CHECK(std::find(urls.begin(), urls.end(), "https://example.com") != urls.end());
    BOOST_CHECK(std::find(urls.begin(), urls.end(), "/relative/path") != urls.end());
    BOOST_CHECK(std::find(urls.begin(), urls.end(), "mailto:test@example.com") != urls.end());
    BOOST_CHECK(std::find(urls.begin(), urls.end(), "https://github.com") != urls.end());
}

BOOST_AUTO_TEST_CASE(get_urls_empty_document) {
    auto h = create_html(test_data::empty_html);
    auto urls = h.get_urls();
		
    BOOST_CHECK(urls.empty());
}

BOOST_AUTO_TEST_CASE(get_urls_no_links) {
    auto h = create_html(test_data::basic_html);
    auto urls = h.get_urls();
    BOOST_CHECK(urls.empty());
}

BOOST_AUTO_TEST_CASE(get_urls_special_chars) {
    auto h = create_html(test_data::html_special_chars);
    auto urls = h.get_urls();
    
    BOOST_CHECK_EQUAL(urls.size(), 1);
    BOOST_CHECK_EQUAL(urls[0], "https://example.com?param=value&other=test");
}

BOOST_AUTO_TEST_CASE(get_urls_unicode) {
    auto h = create_html(test_data::html_unicode);
    auto urls = h.get_urls();
    
    BOOST_CHECK_EQUAL(urls.size(), 1);
    BOOST_CHECK_EQUAL(urls[0], "https://example.com/测试");
}

// From a bug during running my indexer.
BOOST_AUTO_TEST_CASE(test_get_urls_space_around_url)
{
    auto h = create_html(test_data::html_with_link_ws);
	// Now, test if webpage can strip off the spaces and get the true urls.
	webpage pg(urls::url("https://example.com"), std::move(h));
	auto urls{pg.get_urls()};
	
	BOOST_CHECK_EQUAL(urls.size(), 4);
	BOOST_CHECK_EQUAL(
		std::string(urls[0].c_str()), 
		"https://example.com"
	);
	BOOST_CHECK_EQUAL(
		std::string(urls[1].c_str()), 
		"https://example.com/relative/path"
	);
	BOOST_CHECK_EQUAL(
		std::string(urls[2].c_str()), 
		"https://example.com/abc"
	);
	BOOST_CHECK_EQUAL(
		std::string(urls[3].c_str()), 
		"https://example.com/def"
	);
}

BOOST_AUTO_TEST_CASE(test_get_urls_illformed)
{
    auto h = create_html(test_data::html_with_illformed_links);
	// Now, test if webpage can strip off the spaces and get the true urls.
	webpage pg(urls::url("https://example.com"), std::move(h));
	auto urls{pg.get_urls()};
	
	// illformed links will be ignored
	BOOST_CHECK_EQUAL(urls.size(), 2);
	BOOST_CHECK_EQUAL(
		std::string(urls[0].c_str()), 
		"https://example.com/relative/path"
	);
	BOOST_CHECK_EQUAL(
		std::string(urls[1].c_str()), 
		"https://example.com/def"
	);
}

BOOST_AUTO_TEST_CASE(get_text_basic) {
    auto h = create_html(test_data::basic_html);
    std::string text = h.text;
    
    BOOST_CHECK(text.find("Hello World") != std::string::npos);
    BOOST_CHECK(text.find("This is a test paragraph.") != std::string::npos);
	// should not turn text into lower case
    BOOST_CHECK(text.find("hello world") == std::string::npos); 
}

BOOST_AUTO_TEST_CASE(get_text_complex) {
    auto h = create_html(test_data::html_with_text);
    std::string text = h.text;
    
    // Check that various text elements are extracted
    BOOST_CHECK(text.find("Main Heading") != std::string::npos);
    BOOST_CHECK(text.find("First paragraph with some text.") != std::string::npos);
    BOOST_CHECK(text.find("Subheading") != std::string::npos);
    BOOST_CHECK(text.find("List item 1") != std::string::npos);
    BOOST_CHECK(text.find("List item 2") != std::string::npos);
    
	// even code and style are included
    BOOST_CHECK(text.find("console.log") != std::string::npos);
    BOOST_CHECK(text.find("color: red") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(get_text_empty) {
    auto h = create_html(test_data::empty_html);
	// should be nothing more than whitespaces
	for (const auto c : h.text)
	{
		BOOST_CHECK(std::isspace(c));
	}
}

BOOST_AUTO_TEST_CASE(get_text_unicode) {
    auto h = create_html(test_data::html_unicode);
    std::string text = h.text;
    
    // Unicode text should be preserved
    BOOST_CHECK(text.find("你好世界") != std::string::npos);
    BOOST_CHECK(text.find("こんにちは") != std::string::npos);
    BOOST_CHECK(text.find("😀🌟🎉") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(get_date_valid_header) {
    std::map<std::string, std::string> headers = {
        {"date", "Wed, 21 Oct 2015 07:28 GMT"}
    };
    
    auto h = create_html(test_data::basic_html, headers);
    auto date = h.get_date();
    
    BOOST_CHECK_EQUAL(static_cast<int>(date.year()), 2015);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(date.month()), 10);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(date.day()), 21);
}

BOOST_AUTO_TEST_CASE(get_date_different_format) {
    std::map<std::string, std::string> headers = {
        {"date", "Fri, 01 Jan 2021 12:00 GMT"}
    };
    
    auto h = create_html(test_data::basic_html, headers);
    auto date = h.get_date();
    
    BOOST_CHECK_EQUAL(static_cast<int>(date.year()), 2021);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(date.month()), 1);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(date.day()), 1);
}

BOOST_AUTO_TEST_CASE(get_date_invalid_header) {
    std::map<std::string, std::string> headers = {
        {"date", "invalid date format"}
    };
    
    // This should return today on failure.
    BOOST_CHECK(
        create_html(test_data::basic_html, headers).get_date()
		 == ch::year_month_day{
			ch::floor<ch::days>(ch::system_clock::now())
		}
    );
}

BOOST_AUTO_TEST_CASE(get_date_missing_header) {
    std::map<std::string, std::string> headers; // Empty headers
    
    // This should return today
    BOOST_CHECK(
        create_html(test_data::basic_html, headers).get_date()
		 == ch::year_month_day{
			ch::floor<ch::days>(ch::system_clock::now())
		}
    );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(WebpageTests, HtmlFixture)

BOOST_AUTO_TEST_CASE(webpage_constructor_metadata) {
    std::string url = "https://example.com/test";
    std::string title = "Test Page";
    ch::year_month_day date = ch::year{2023} / ch::month{6} / ch::day{15};
    
    webpage wp(url, title, date);
    
    //BOOST_CHECK_EQUAL(wp.url, url);
    BOOST_CHECK_EQUAL(wp.title, title);
	BOOST_CHECK(wp.date == date);
    
    // Should not have HTML content
    BOOST_CHECK_EQUAL(wp.get_text(), "");
    BOOST_CHECK(wp.get_urls().empty());
}

BOOST_AUTO_TEST_CASE(webpage_get_text_no_html) {
    webpage wp("https://example.com", "Test", ch::year{2023}/ch::month{1}/ch::day{1});
    BOOST_CHECK_EQUAL(wp.get_text(), "");
}

BOOST_AUTO_TEST_CASE(webpage_get_urls_no_html) {
    webpage wp("https://example.com", "Test", ch::year{2023}/ch::month{1}/ch::day{1});
    auto urls = wp.get_urls();
    BOOST_CHECK(urls.empty());
}

BOOST_AUTO_TEST_SUITE_END()

// Additional test cases for edge cases and error conditions
BOOST_FIXTURE_TEST_SUITE(EdgeCaseTests, HtmlFixture)

BOOST_AUTO_TEST_CASE(very_large_html) {
    // Create a large HTML document
    std::string large_html = "<!DOCTYPE html><html><head><title>Large Document</title></head><body>";
    for (int i = 0; i < 1000; ++i) {
        large_html += "<p>This is paragraph " + std::to_string(i) + " with some content.</p>";
        if (i % 10 == 0) {
            large_html += "<a href=\"https://example.com/page" + std::to_string(i) + "\">Link " + std::to_string(i) + "</a>";
        }
    }
    large_html += "</body></html>";
    
    BOOST_REQUIRE_NO_THROW({
        auto h = create_html(large_html);
        BOOST_CHECK_EQUAL(h.get_title(), "Large Document");
        
        auto urls = h.get_urls();
        BOOST_CHECK_EQUAL(urls.size(), 100); // Every 10th iteration
        
        std::string text = h.text;
        BOOST_CHECK(!text.empty());
        BOOST_CHECK(text.find("This is paragraph 500") != std::string::npos);
    });
}

BOOST_AUTO_TEST_CASE(nested_elements) {
    std::string nested_html = R"(
<!DOCTYPE html>
<html>
<head><title>Nested Elements</title></head>
<body>
    <div>
        <div>
            <div>
                <p>Deeply nested <a href="inner.html">link</a></p>
                <div>
                    <span>More nesting with <a href="deep.html">another link</a></span>
                </div>
            </div>
        </div>
    </div>
</body>
</html>
)";
    
    auto h = create_html(nested_html);
    auto urls = h.get_urls();
    
    BOOST_CHECK_EQUAL(urls.size(), 2);
    BOOST_CHECK(std::find(urls.begin(), urls.end(), "inner.html") != urls.end());
    BOOST_CHECK(std::find(urls.begin(), urls.end(), "deep.html") != urls.end());
    
    std::string text = h.text;
    BOOST_CHECK(text.find("Deeply nested ") != std::string::npos);
    BOOST_CHECK(text.find("More nesting with ") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(html_with_comments) {
    std::string html_with_comments = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Comments Test</title>
    <!-- This is a comment -->
</head>
<body>
    <!-- Another comment -->
    <p>Visible text</p>
    <!-- <a href="commented-out.html">Commented link</a> -->
    <a href="real-link.html">Real link</a>
    <!-- End comment -->
</body>
</html>
)";
    
    auto h = create_html(html_with_comments);
    
    BOOST_CHECK_EQUAL(h.get_title(), "Comments Test");
    
    auto urls = h.get_urls();
    BOOST_CHECK_EQUAL(urls.size(), 1);
    BOOST_CHECK_EQUAL(urls[0], "real-link.html");
    
    std::string text = h.text;
    BOOST_CHECK(text.find("Visible text") != std::string::npos);
	// this is commented out.
    BOOST_CHECK(text.find("Commented link") == std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()

// Other tests
BOOST_FIXTURE_TEST_SUITE(OtherTests, HtmlFixture)


BOOST_AUTO_TEST_CASE(reuse_parser) {
    // Test that a single parser can be reused multiple times
    parser p;
    
    for (int i = 0; i < 10; ++i) {
        auto* doc = p.parse(
            reinterpret_cast<const lxb_char_t*>(test_data::basic_html.c_str()),
            test_data::basic_html.size(),
			nullptr
        );
        
        BOOST_CHECK(doc != nullptr);
		BOOST_CHECK(doc->body != nullptr);
		BOOST_CHECK(doc->head != nullptr);
        lxb_html_document_destroy(doc);
    }
}

BOOST_AUTO_TEST_SUITE_END()
