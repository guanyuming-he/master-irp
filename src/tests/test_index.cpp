/**
 * Unit tests for the index class.
 *
 * Copyright (C) 2025 Guanyuming He
 * The file is licensed under the GNU GPL v3.
 * 
 * This time, Claude failed to understand many of my interfaces.
 * As a result, only a small part of the tests are written by Claude.
 * Instead, most are written by myself.
 *
 */
#include <boost/test/tools/old/interface.hpp>
#define BOOST_TEST_MODULE IndexTests

#include <boost/test/unit_test.hpp>

#include <xapian.h>
#include <filesystem>
#include <optional>
#include <fstream>

#include "../search/index.h"
#include "../search/webpage.h"
#include "../search/url2html.h"

namespace fs = std::filesystem;
namespace xp = Xapian;

// Test fixture for disk-based index tests
struct DiskIndexFixture 
{
    fs::path temp_dir;
    fs::path db_path;

    DiskIndexFixture() {
        // Create temporary directory
        temp_dir = fs::temp_directory_path() / "index_test";
        fs::create_directories(temp_dir);

		// A db is a directory!
        db_path = temp_dir / "test_db";
    }

    ~DiskIndexFixture() {
        // rm -rf temp_dir
        if (fs::exists(temp_dir)) {
            fs::remove_all(temp_dir);
        }
    }

};

// Mock HTML and webpage creation helpers
class MockHtml {
public:
	static std::string create_html_string(
		const std::string& title, const
		std::string& body_text
	) {
        return R"(<!DOCTYPE html>
			<html>
			<head>
				<title>)" + title + R"(</title>
				<meta name="date" content="2025-01-15">
			</head>
			<body>
				<h1>)" + title + R"(</h1>
				<p>)" + body_text + R"(</p>
			</body>
			</html>)";
    }

    static html create_html_object(const std::string& title, const std::string& body_text) {
        std::string html_content = create_html_string(title, body_text);

        // Create a mock parser to parse the HTML
        parser p;
        std::string all_text;
        lxb_html_document_t* doc = p.parse(
            reinterpret_cast<const lxb_char_t*>(html_content.c_str()),
            html_content.length(),
            &all_text
        );

        // Create mock headers
        std::map<std::string, std::string> headers;
        headers["date"] = "Mon, 15 Jan 2025 12:00:00 GMT";
        headers["content-type"] = "text/html";

        return html(doc, std::move(headers), std::move(all_text));
    }
};

webpage create_mock_webpage(
		const std::string& url_str, const std::string&
		title, const std::string& content = ""
) {
	urls::url page_url(url_str);
    html html_obj = MockHtml::create_html_object(title, content);

    return webpage(std::move(page_url), std::move(html_obj));
}

BOOST_AUTO_TEST_SUITE(IndexConstructionTests)

BOOST_AUTO_TEST_CASE(test_index_ctor_no_dir)
{
	auto dbp = fs::temp_directory_path() / "nonexistent";
	if (fs::exists(dbp))
		fs::remove_all(dbp);

	class index i(dbp);

	// ctor should succeed without exceptions.
	
	// num_docs should be 0.
	BOOST_CHECK_EQUAL(i.num_documents(), 0);
}

BOOST_AUTO_TEST_CASE(test_index_ctor_no_parent_dir)
{
	auto dbp = fs::temp_directory_path() / "nonexistent";
	if (fs::exists(dbp))
		fs::remove_all(dbp);

	BOOST_CHECK_THROW(
		class index i(dbp / "level2"),
		xp::RuntimeError
	);
}

BOOST_AUTO_TEST_CASE(test_index_ctor_has_dir_empty)
{
	auto dbp = fs::temp_directory_path() / "test_db1";
	// overwrite to ensure it's empty.
	{
		xp::WritableDatabase db(dbp.string(), xp::DB_CREATE_OR_OVERWRITE);
		db.close();
	}

	class index i(dbp);

	// num_docs should be 0.
	BOOST_CHECK_EQUAL(i.num_documents(), 0);
}

BOOST_AUTO_TEST_CASE(test_index_ctor_has_dir_nonempty)
{
	auto dbp = fs::temp_directory_path() / "test_db1";
	xp::docid id1, id2;
	// overwrite to ensure it has intended content.
	{
		xp::WritableDatabase db(dbp.string(), xp::DB_CREATE_OR_OVERWRITE);
		
		xp::Document doc1, doc2;
		doc1.set_data("abc"); doc1.add_boolean_term("Q1");
		doc2.set_data("def"); doc2.add_boolean_term("Q2");
		id1 = db.add_document(doc1); 
		id2 = db.add_document(doc2);

		db.close();
	}

	class index i(dbp);

	// num_docs should be 2.
	BOOST_CHECK_EQUAL(i.num_documents(), 2);
	// should have the added docs.
	std::optional<xp::Document> doc1 = i.get_document(id1);
	std::optional<xp::Document> doc2 = i.get_document(id2);
	BOOST_TEST(doc1.has_value());
	BOOST_TEST(doc2.has_value());
	BOOST_CHECK_EQUAL(doc1->get_data(), "abc");
	BOOST_CHECK_EQUAL(doc2->get_data(), "def");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE(IndexAddDocTests, DiskIndexFixture)

BOOST_AUTO_TEST_CASE(add_to_empty)
{
	class index i(db_path);
	
	std::string url_str{"https://test-add-to-empty/abc"};
	std::string title_str{"Test add to empty"};
	auto pg1 = create_mock_webpage(
		url_str, 
		title_str,
		"Here is some content"
	);
	i.add_document(pg1);

	BOOST_CHECK_EQUAL(i.num_documents(), 1);
	auto doc = i.get_document(pg1);
	BOOST_TEST(doc.has_value());

	// doc's data will contain the url and the title.
	auto data = doc->get_data();
	BOOST_TEST(data.contains(url_str));
	BOOST_TEST(data.contains(title_str));
}

BOOST_AUTO_TEST_CASE(add_to_nonempty)
{
	xp::docid id1, id2;
	// overwrite to ensure it has intended content.
	{
		xp::WritableDatabase db(db_path, xp::DB_CREATE_OR_OVERWRITE);
		
		xp::Document doc1, doc2;
		doc1.set_data("abc"); doc1.add_boolean_term("Q1");
		doc2.set_data("def"); doc2.add_boolean_term("Q2");
		id1 = db.add_document(doc1); 
		id2 = db.add_document(doc2);

		db.close();
	}

	class index i(db_path);
	
	std::string url_str{"https://test-add-to-non-empty/abc"};
	std::string title_str{"Test add to non-empty"};
	auto pg1 = create_mock_webpage(
		url_str, 
		title_str,
		"Here is some content"
	);
	i.add_document(pg1);

	BOOST_CHECK_EQUAL(i.num_documents(), 3);
	auto doc = i.get_document(pg1);
	BOOST_TEST(doc.has_value());

	// doc's data will contain the url and the title.
	auto data = doc->get_data();
	BOOST_TEST(data.contains(url_str));
	BOOST_TEST(data.contains(title_str));
}

BOOST_AUTO_TEST_CASE(add_existing)
{
	class index i(db_path);
	
	std::string url_str{"https://test-add-to-non-empty/abc"};
	std::string title_str{"Test add to existing"};
	auto pg1 = create_mock_webpage(
		url_str, 
		title_str,
		"Here is some content"
	);
	auto pg2 = create_mock_webpage(
		url_str, 
		title_str,
		"Here is some content 2"
	);
	// the two adds should result in the same.
	i.add_document(pg1);
	i.add_document(pg2);

	BOOST_CHECK_EQUAL(i.num_documents(), 1);
	auto doc = i.get_document(pg1);
	BOOST_TEST(doc.has_value());

	// doc's data will contain the url and the title.
	auto data = doc->get_data();
	BOOST_TEST(data.contains(url_str));
	BOOST_TEST(data.contains(title_str));
}

BOOST_AUTO_TEST_CASE(add_multiple)
{
	class index i(db_path);
	
	std::string url_str1{"https://test-add-to-non-empty/abc"};
	std::string title_str1{"Abc"};
	std::string url_str2{"https://test-add-to-non-empty/def"};
	std::string title_str2{"Def"};

	auto pg1 = create_mock_webpage(
		url_str1, 
		title_str1,
		"Here is some content"
	);
	auto pg2 = create_mock_webpage(
		url_str2, 
		title_str2,
		"Here is some content 2"
	);
	// the two adds should result in the same.
	i.add_document(pg1);
	i.add_document(pg2);

	BOOST_CHECK_EQUAL(i.num_documents(), 2);
	auto doc1 = i.get_document(pg1);
	auto doc2 = i.get_document(pg2);
	BOOST_TEST(doc1.has_value());
	BOOST_TEST(doc2.has_value());

	// doc's data will contain the url and the title.
	auto data1 = doc1->get_data();
	BOOST_TEST(data1.contains(url_str1));
	BOOST_TEST(data1.contains(title_str1));
	auto data2 = doc2->get_data();
	BOOST_TEST(data2.contains(url_str2));
	BOOST_TEST(data2.contains(title_str2));
}

BOOST_AUTO_TEST_SUITE_END()
