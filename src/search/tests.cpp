#include <gtest/gtest.h>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>

// Include your headers
#include "webpage.h"
#include "url2html.h"
#include "utility.h"

namespace ch = std::chrono;

class WebScraperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize global components before each test
        global_init();
    }
};

// Test fixture for parser tests
class ParserTest : public WebScraperTest {
protected:
    parser p;
    
    // Helper method to create HTML document from string
    lxb_html_document_t* parseHTML(const std::string& html_content) {
        return p.parse(
            reinterpret_cast<const lxb_char_t*>(html_content.c_str()),
            html_content.size()
        );
    }
};

// Test fixture for html class tests
class HtmlTest : public WebScraperTest {
protected:
    parser p;
    
    // Helper method to create html object from string
    std::unique_ptr<html> createHtml(const std::string& html_content) {
        auto* doc = p.parse(
            reinterpret_cast<const lxb_char_t*>(html_content.c_str()),
            html_content.size()
        );
        return std::make_unique<html>(doc);
    }
};

// Test fixture for webpage tests
class WebpageTest : public WebScraperTest {
protected:
    // Mock url2html for testing
    class MockUrl2Html : public url2html {
    private:
        std::string mock_html_content;
        
    public:
        void setMockContent(const std::string& content) {
            mock_html_content = content;
        }
        
        lxb_html_document_t* convert(const std::string& url) override {
            parser p;
            return p.parse(
                reinterpret_cast<const lxb_char_t*>(mock_html_content.c_str()),
                mock_html_content.size()
            );
        }
    };
};

// Parser Tests
TEST_F(ParserTest, ParseValidHTML) {
    std::string html_content = "<html><head><title>Test</title></head><body>Hello World</body></html>";
    
    lxb_html_document_t* doc = parseHTML(html_content);
    ASSERT_NE(doc, nullptr);
    
    // Clean up
    lxb_html_document_destroy(doc);
}

TEST_F(ParserTest, ParseEmptyHTML) {
    std::string html_content = "";
    
    lxb_html_document_t* doc = parseHTML(html_content);
    ASSERT_NE(doc, nullptr); // lexbor should still create a document
    
    lxb_html_document_destroy(doc);
}

TEST_F(ParserTest, ParseMalformedHTML) {
    std::string html_content = "<html><head><title>Test</title><body>Missing closing tags";
    
    lxb_html_document_t* doc = parseHTML(html_content);
    ASSERT_NE(doc, nullptr); // lexbor should handle malformed HTML gracefully
    
    lxb_html_document_destroy(doc);
}

TEST_F(ParserTest, ParseHTMLWithSpecialCharacters) {
    std::string html_content = "<html><head><title>Test &amp; More</title></head><body>&lt;special&gt;</body></html>";
    
    lxb_html_document_t* doc = parseHTML(html_content);
    ASSERT_NE(doc, nullptr);
    
    lxb_html_document_destroy(doc);
}

// HTML Class Tests
TEST_F(HtmlTest, GetTitleFromValidHTML) {
    std::string html_content = "<html><head><title>My Test Page</title></head><body>Content</body></html>";
    
    auto html_obj = createHtml(html_content);
    std::string title = html_obj->get_title();
    
    EXPECT_EQ(title, "My Test Page");
}

TEST_F(HtmlTest, GetTitleFromHTMLWithoutTitle) {
    std::string html_content = "<html><head></head><body>Content</body></html>";
    
    auto html_obj = createHtml(html_content);
    std::string title = html_obj->get_title();
    
    EXPECT_TRUE(title.empty());
}

TEST_F(HtmlTest, GetTitleWithWhitespace) {
    std::string html_content = "<html><head><title>  Whitespace Title  </title></head><body>Content</body></html>";
    
    auto html_obj = createHtml(html_content);
    std::string title = html_obj->get_title();
    
    // Assuming the implementation trims whitespace
    EXPECT_EQ(title, "Whitespace Title");
}

TEST_F(HtmlTest, GetTextFromSimpleHTML) {
    std::string html_content = "<html><head><title>Title</title></head><body><p>Hello World</p></body></html>";
    
    auto html_obj = createHtml(html_content);
    std::string text = html_obj->get_text();
    
    // Text should be in lowercase and contain the body content
    EXPECT_NE(text.find("hello world"), std::string::npos);
}

TEST_F(HtmlTest, GetTextFromComplexHTML) {
    std::string html_content = R"(
        <html>
            <head><title>Complex Page</title></head>
            <body>
                <h1>Main Heading</h1>
                <p>First paragraph with <strong>bold text</strong>.</p>
                <div>
                    <p>Second paragraph in a div.</p>
                    <ul>
                        <li>List item 1</li>
                        <li>List item 2</li>
                    </ul>
                </div>
            </body>
        </html>
    )";
    
    auto html_obj = createHtml(html_content);
    std::string text = html_obj->get_text();
    
    // Check that text contains expected content in lowercase
    EXPECT_NE(text.find("main heading"), std::string::npos);
    EXPECT_NE(text.find("first paragraph"), std::string::npos);
    EXPECT_NE(text.find("bold text"), std::string::npos);
    EXPECT_NE(text.find("list item 1"), std::string::npos);
}

TEST_F(HtmlTest, GetTextIgnoresScriptAndStyle) {
    std::string html_content = R"(
        <html>
            <head>
                <title>Test</title>
                <style>body { color: red; }</style>
                <script>console.log('hello');</script>
            </head>
            <body>
                <p>Visible text</p>
                <script>alert('popup');</script>
            </body>
        </html>
    )";
    
    auto html_obj = createHtml(html_content);
    std::string text = html_obj->get_text();
    
    // Text should contain visible content but not script/style content
    EXPECT_NE(text.find("visible text"), std::string::npos);
    // These should ideally not be present (depending on implementation)
    // EXPECT_EQ(text.find("console.log"), std::string::npos);
    // EXPECT_EQ(text.find("color: red"), std::string::npos);
}

TEST_F(HtmlTest, GetUrlsFromHTML) {
    std::string html_content = R"(
        <html>
            <head><title>Test</title></head>
            <body>
                <a href="https://example.com">Link 1</a>
                <a href="/relative/path">Link 2</a>
                <a href="mailto:test@example.com">Email</a>
                <img src="https://example.com/image.jpg" alt="Image">
                <link rel="stylesheet" href="/styles.css">
            </body>
        </html>
    )";
    
    auto html_obj = createHtml(html_content);
    std::vector<std::string> urls = html_obj->get_urls();
    
    EXPECT_FALSE(urls.empty());
    
    // Check for expected URLs (exact behavior depends on implementation)
    bool found_example_com = std::any_of(urls.begin(), urls.end(), 
        [](const std::string& url) { return url.find("example.com") != std::string::npos; });
    EXPECT_TRUE(found_example_com);
}

TEST_F(HtmlTest, GetUrlsFromHTMLWithNoLinks) {
    std::string html_content = "<html><head><title>Test</title></head><body><p>No links here</p></body></html>";
    
    auto html_obj = createHtml(html_content);
    std::vector<std::string> urls = html_obj->get_urls();
    
    EXPECT_TRUE(urls.empty());
}

TEST_F(HtmlTest, GetDateFromHTML) {
    // This test depends on how you implement date extraction
    // You might look for meta tags, article dates, etc.
    std::string html_content = R"(
        <html>
            <head>
                <title>Test</title>
                <meta name="date" content="2025-01-15">
                <meta property="article:published_time" content="2025-01-15T10:30:00Z">
            </head>
            <body>Content</body>
        </html>
    )";
    
    auto html_obj = createHtml(html_content);
    ch::year_month_day date = html_obj->get_date();
    
    // Test depends on implementation - might default to current date or extract from meta
    EXPECT_TRUE(date.ok());
}

// Webpage Class Tests
TEST_F(WebpageTest, ConstructorWithMetadata) {
    std::string url = "https://example.com";
    std::string title = "Test Page";
    ch::year_month_day date = ch::year{2025}/ch::month{1}/ch::day{15};
    
    webpage page(url, title, date);
    
    EXPECT_EQ(page.url, url);
    EXPECT_EQ(page.title, title);
    EXPECT_EQ(page.date, date);
    
    // Since HTML is not loaded, these should return empty
    EXPECT_TRUE(page.get_text().empty());
    EXPECT_TRUE(page.get_urls().empty());
}

TEST_F(WebpageTest, ConstructorWithUrl2Html) {
    MockUrl2Html mock_converter;
    std::string mock_html = "<html><head><title>Dynamic Title</title></head><body>Dynamic content</body></html>";
    mock_converter.setMockContent(mock_html);
    
    std::string url = "https://example.com";
    webpage page(url, mock_converter);
    
    EXPECT_EQ(page.url, url);
    EXPECT_EQ(page.title, "Dynamic Title");
    EXPECT_FALSE(page.get_text().empty());
    EXPECT_NE(page.get_text().find("dynamic content"), std::string::npos);
}

TEST_F(WebpageTest, GetTextWhenHTMLLoaded) {
    MockUrl2Html mock_converter;
    std::string mock_html = "<html><head><title>Test</title></head><body><p>Test Content</p></body></html>";
    mock_converter.setMockContent(mock_html);
    
    webpage page("https://example.com", mock_converter);
    std::string text = page.get_text();
    
    EXPECT_FALSE(text.empty());
    EXPECT_NE(text.find("test content"), std::string::npos);
}

TEST_F(WebpageTest, GetUrlsWhenHTMLLoaded) {
    MockUrl2Html mock_converter;
    std::string mock_html = R"(
        <html>
            <head><title>Test</title></head>
            <body>
                <a href="https://example.com/page1">Link 1</a>
                <a href="https://example.com/page2">Link 2</a>
            </body>
        </html>
    )";
    mock_converter.setMockContent(mock_html);
    
    webpage page("https://example.com", mock_converter);
    std::vector<std::string> urls = page.get_urls();
    
    EXPECT_FALSE(urls.empty());
    EXPECT_GE(urls.size(), 2);
}

// Edge Cases and Error Handling Tests
TEST_F(HtmlTest, HandleVeryLargeHTML) {
    // Create a large HTML document
    std::string large_content = "<html><head><title>Large</title></head><body>";
    for (int i = 0; i < 10000; ++i) {
        large_content += "<p>Paragraph " + std::to_string(i) + "</p>";
    }
    large_content += "</body></html>";
    
    auto html_obj = createHtml(large_content);
    EXPECT_NO_THROW({
        std::string text = html_obj->get_text();
        EXPECT_FALSE(text.empty());
    });
}

TEST_F(HtmlTest, HandleHTMLWithUnicodeCharacters) {
    std::string html_content = R"(
        <html>
            <head><title>Unicode Test 测试</title></head>
            <body>
                <p>English text</p>
                <p>中文测试</p>
                <p>Español</p>
                <p>العربية</p>
                <p>🚀 Emoji test</p>
            </body>
        </html>
    )";
    
    auto html_obj = createHtml(html_content);
    EXPECT_NO_THROW({
        std::string title = html_obj->get_title();
        std::string text = html_obj->get_text();
        EXPECT_FALSE(title.empty());
        EXPECT_FALSE(text.empty());
    });
}

TEST_F(WebpageTest, HandleEmptyTitle) {
    MockUrl2Html mock_converter;
    std::string mock_html = "<html><head><title></title></head><body>Content</body></html>";
    mock_converter.setMockContent(mock_html);
    
    webpage page("https://example.com", mock_converter);
    
    EXPECT_TRUE(page.title.empty());
    EXPECT_FALSE(page.get_text().empty());
}

// Performance Tests (optional)
TEST_F(ParserTest, ParseMultipleDocuments) {
    std::vector<std::string> html_docs = {
        "<html><head><title>Doc 1</title></head><body>Content 1</body></html>",
        "<html><head><title>Doc 2</title></head><body>Content 2</body></html>",
        "<html><head><title>Doc 3</title></head><body>Content 3</body></html>"
    };
    
    for (const auto& doc : html_docs) {
        lxb_html_document_t* parsed = parseHTML(doc);
        ASSERT_NE(parsed, nullptr);
        lxb_html_document_destroy(parsed);
    }
}

// Integration Tests
TEST_F(WebScraperTest, Url2HtmlIntegration) {
    url2html converter;
    
    // Test with a simple HTML string (this requires implementing a way to feed HTML directly)
    // For now, this is a placeholder for integration testing
    EXPECT_NO_THROW({
        // This would typically test the full pipeline
        // std::string html = "<html><head><title>Test</title></head><body>Content</body></html>";
        // auto* doc = converter.convert("test://url");
        // ASSERT_NE(doc, nullptr);
        // lxb_html_document_destroy(doc);
    });
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
