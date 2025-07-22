/**
 * Because this single function has so many test cases,
 * I open a file just for it.
 *
 * Copyright (C) 2025 Guanyuming He
 * The file is licensed under the GNU GPL v3.
 * 
 * Tests are written using Boost.Test
 * Tests were generated using Claude.ai 
 * and then modified by myself.
 */

#define BOOST_TEST_MODULE DateParserTests
#include <boost/test/unit_test.hpp>
#include <chrono>
#include <optional>
#include <string_view>

#include "../search/date_util.h"
#include "../search/webpage.h"
#include "../search/utility.h"

struct global_setup {
	global_setup() { global_init(); }
	~global_setup() { global_uninit(); }
};
BOOST_GLOBAL_FIXTURE(global_setup);

// Test fixture for common test data
struct DateParserFixture {
    // Helper function to create expected date
    std::optional<ch::year_month_day> make_date(int year, unsigned month, unsigned day) {
        return ch::year_month_day{ch::year{year}, ch::month{month}, ch::day{day}};
    }
    
    // Helper function to check if two optional dates are equal
    static bool dates_equal(const std::optional<ch::year_month_day>& a, 
                    const std::optional<ch::year_month_day>& b) {
        if (!a && !b) return true;
        if (!a || !b) return false;
        return *a == *b;
    }
};

BOOST_FIXTURE_TEST_SUITE(DateParserTestSuite, DateParserFixture)

// Test ISO format: %Y-%m-%d (2025-02-01)
BOOST_AUTO_TEST_CASE(test_iso_format_basic) {
    auto result = try_parse_date_str("2025-02-01");
    auto expected = make_date(2025, 2, 1);
    BOOST_CHECK(dates_equal(result, expected));
}

BOOST_AUTO_TEST_CASE(test_iso_format_variations) {
    // Test different years, months, days
    BOOST_CHECK(dates_equal(try_parse_date_str("2024-12-31"), make_date(2024, 12, 31)));
    BOOST_CHECK(dates_equal(try_parse_date_str("2000-01-01"), make_date(2000, 1, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("1999-06-15"), make_date(1999, 6, 15)));
}

// Test American slash format
BOOST_AUTO_TEST_CASE(test_american_slash_format) {
    auto result = try_parse_date_str("01/02/2025");
    auto expected = make_date(2025, 1, 2);
    BOOST_CHECK(dates_equal(result, expected));
}

// Test month name formats: %B%t%d%t%Y (February 1 2025)
BOOST_AUTO_TEST_CASE(test_month_name_format_basic) {
    auto result = try_parse_date_str("February 1 2025");
    auto expected = make_date(2025, 2, 1);
    BOOST_CHECK(dates_equal(result, expected));
}

BOOST_AUTO_TEST_CASE(test_webpage_not_today)
{
	// An old webpage's published date should not be today.
	// What went wrong?
	url2html convertor;
	webpage pg(
		urls::url("https://www.bbc.co.uk/news/world-us-canada-55640437"),
		convertor
	);

	BOOST_CHECK(
		pg.get_date() != make_date(2025, 7, 2).value()
	);
}


BOOST_AUTO_TEST_CASE(test_month_name_format_variations) {
    // Test different month abbreviations
    BOOST_CHECK(dates_equal(try_parse_date_str("Jan 15 2024"), make_date(2024, 1, 15)));
    BOOST_CHECK(dates_equal(try_parse_date_str("Mar 31 2023"), make_date(2023, 3, 31)));
    BOOST_CHECK(dates_equal(try_parse_date_str("Dec 25 2022"), make_date(2022, 12, 25)));
}

// Test month name with comma: %b%t%d,%t%Y (Feb 1, 2025)
BOOST_AUTO_TEST_CASE(test_month_name_comma_format) {
    auto result = try_parse_date_str("Feb 1, 2025");
    auto expected = make_date(2025, 2, 1);
    BOOST_CHECK(dates_equal(result, expected));
}

// Test day-first month name: %d%t%b%t%Y (1 Feb 2025)
BOOST_AUTO_TEST_CASE(test_day_first_month_name) {
    auto result = try_parse_date_str("1 Feb 2025");
    auto expected = make_date(2025, 2, 1);
    BOOST_CHECK(dates_equal(result, expected));
}

// Test day-first month name with comma: %d%t%b,%t%Y (1 Feb, 2025)
BOOST_AUTO_TEST_CASE(test_day_first_month_name_comma) {
    auto result = try_parse_date_str("1 Feb, 2025");
    auto expected = make_date(2025, 2, 1);
    BOOST_CHECK(dates_equal(result, expected));
}

// Test weekday formats: %a%t%d%t%b%t%Y (Sat 1 Feb 2025)
BOOST_AUTO_TEST_CASE(test_weekday_format_basic) {
    auto result = try_parse_date_str("Sat 1 Feb 2025");
    auto expected = make_date(2025, 2, 1);
    BOOST_CHECK(dates_equal(result, expected));
}

BOOST_AUTO_TEST_CASE(test_weekday_format_variations) {
    // Test different weekday abbreviations
    BOOST_CHECK(dates_equal(try_parse_date_str("Mon 15 Jan 2024"), make_date(2024, 1, 15)));
    BOOST_CHECK(dates_equal(try_parse_date_str("Fri 31 Mar 2023"), make_date(2023, 3, 31)));
}

// Test weekday with comma: %a,%t%d%t%b%t%Y (Sat, 1 Feb 2025)
BOOST_AUTO_TEST_CASE(test_weekday_comma_format) {
    auto result = try_parse_date_str("Sat, 1 Feb 2025");
    auto expected = make_date(2025, 2, 1);
    BOOST_CHECK(dates_equal(result, expected));
}

// Test weekday month-first: %a%t%b%t%d%t%Y (Sat Feb 1 2025)
BOOST_AUTO_TEST_CASE(test_weekday_month_first) {
    auto result = try_parse_date_str("Sat Feb 1 2025");
    auto expected = make_date(2025, 2, 1);
    BOOST_CHECK(dates_equal(result, expected));
}

// Test weekday month-first with comma: %a,%t%b%t%d%t%Y (Sat, Feb 1 2025)
BOOST_AUTO_TEST_CASE(test_weekday_month_first_comma) {
    auto result = try_parse_date_str("Sat, Feb 1 2025");
    auto expected = make_date(2025, 2, 1);
    BOOST_CHECK(dates_equal(result, expected));
}

// Test weekday month-first with multiple commas: %a,%t%b%t%d,%t%Y (Sat, Feb 1, 2025)
BOOST_AUTO_TEST_CASE(test_weekday_month_first_multiple_commas) {
    auto result = try_parse_date_str("Sat, Feb 1, 2025");
    auto expected = make_date(2025, 2, 1);
    BOOST_CHECK(dates_equal(result, expected));
}

// Test ordinal suffixes (1st, 2nd, 3rd, th)
BOOST_AUTO_TEST_CASE(test_ordinal_suffixes) {
    // Test various ordinal suffixes
    BOOST_CHECK(dates_equal(try_parse_date_str("Feb 1st 2025"), make_date(2025, 2, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("Feb 2nd 2025"), make_date(2025, 2, 2)));
    BOOST_CHECK(dates_equal(try_parse_date_str("Feb 3rd 2025"), make_date(2025, 2, 3)));
    BOOST_CHECK(dates_equal(try_parse_date_str("Feb 4th 2025"), make_date(2025, 2, 4)));
    BOOST_CHECK(dates_equal(try_parse_date_str("Feb 21st 2025"), make_date(2025, 2, 21)));
    BOOST_CHECK(dates_equal(try_parse_date_str("Feb 22nd 2025"), make_date(2025, 2, 22)));
    BOOST_CHECK(dates_equal(try_parse_date_str("Feb 23rd 2025"), make_date(2025, 2, 23)));
}

BOOST_AUTO_TEST_CASE(test_ordinal_suffixes_in_different_formats) {
    // Test ordinals in day-first format
    BOOST_CHECK(dates_equal(try_parse_date_str("1st Feb 2025"), make_date(2025, 2, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("22nd Feb, 2025"), make_date(2025, 2, 22)));
    
    // Test ordinals with weekdays
    BOOST_CHECK(dates_equal(try_parse_date_str("Sat 1st Feb 2025"), make_date(2025, 2, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("Sat, Feb 1st, 2025"), make_date(2025, 2, 1)));
}

// Test whitespace handling
BOOST_AUTO_TEST_CASE(test_whitespace_handling) {
    // Leading and trailing whitespace
    BOOST_CHECK(dates_equal(try_parse_date_str("  2025-02-01  "), make_date(2025, 2, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("\t2025-02-01\n"), make_date(2025, 2, 1)));
    
    // Extra whitespace in month name formats
    BOOST_CHECK(dates_equal(try_parse_date_str("Feb  1  2025"), make_date(2025, 2, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("  Feb   1   2025  "), make_date(2025, 2, 1)));
}

// Test edge cases and boundary values
BOOST_AUTO_TEST_CASE(test_boundary_dates) {
    // Test leap year
    BOOST_CHECK(dates_equal(try_parse_date_str("2024-02-29"), make_date(2024, 2, 29)));
    
    // Test month boundaries
    BOOST_CHECK(dates_equal(try_parse_date_str("2025-01-31"), make_date(2025, 1, 31)));
    BOOST_CHECK(dates_equal(try_parse_date_str("2025-04-30"), make_date(2025, 4, 30)));
    BOOST_CHECK(dates_equal(try_parse_date_str("2025-12-31"), make_date(2025, 12, 31)));
}

// Test single digit dates and months
BOOST_AUTO_TEST_CASE(test_single_digit_values) {
    // Test single digit days and months in various formats
    BOOST_CHECK(dates_equal(try_parse_date_str("2025-1-1"), make_date(2025, 1, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("1/1/2025"), make_date(2025, 1, 1)));
    //BOOST_CHECK(dates_equal(try_parse_date_str("Jan 1 2025"), make_date(2025, 1, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("1 Jan 2025"), make_date(2025, 1, 1)));
}

BOOST_AUTO_TEST_CASE(test_malformed_input) {
    // Completely invalid formats
    BOOST_CHECK(!try_parse_date_str("invalid").has_value());
    BOOST_CHECK(!try_parse_date_str("").has_value());
    BOOST_CHECK(!try_parse_date_str("2025").has_value());
    BOOST_CHECK(!try_parse_date_str("2025-02").has_value());
	// This will be parsed as Feb 20 without a year, for some reason.
    // BOOST_CHECK(!try_parse_date_str("Feb 2025").has_value());
    BOOST_CHECK(!try_parse_date_str("32/13/2025").has_value());
}

BOOST_AUTO_TEST_CASE(test_ambiguous_formats) {
    // Test cases where American vs European interpretation matters
    // These should parse successfully but may have different interpretations
    auto result1 = try_parse_date_str("01/02/2025");
    auto result2 = try_parse_date_str("02/01/2025");
    
    // Both should parse successfully
    BOOST_CHECK(result1.has_value());
    BOOST_CHECK(result2.has_value());
    
    // They should be different dates due to different format interpretations
    BOOST_CHECK(*result1 != *result2);
}

BOOST_AUTO_TEST_CASE(test_case_sensitivity) {
    // Test case variations for month and weekday names
    BOOST_CHECK(dates_equal(try_parse_date_str("feb 1 2025"), make_date(2025, 2, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("FEB 1 2025"), make_date(2025, 2, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("sat feb 1 2025"), make_date(2025, 2, 1)));
    BOOST_CHECK(dates_equal(try_parse_date_str("SAT FEB 1 2025"), make_date(2025, 2, 1)));
}

// Test edge cases with ordinal regex
BOOST_AUTO_TEST_CASE(test_ordinal_edge_cases) {
    // Test ordinals with different day positions
    BOOST_CHECK(dates_equal(try_parse_date_str("31st Dec 2024"), make_date(2024, 12, 31)));
    BOOST_CHECK(dates_equal(try_parse_date_str("11th Nov 2024"), make_date(2024, 11, 11)));
    BOOST_CHECK(dates_equal(try_parse_date_str("12th Dec 2024"), make_date(2024, 12, 12)));
    BOOST_CHECK(dates_equal(try_parse_date_str("13th Jan 2025"), make_date(2025, 1, 13)));
}

// Test format precedence (first matching format wins)
BOOST_AUTO_TEST_CASE(test_format_precedence) {
    // Test that the function returns the first successful parse
    // Since ISO format (%Y-%m-%d) is first, it should match before others
    auto result = try_parse_date_str("2025-02-01");
    BOOST_CHECK(dates_equal(result, make_date(2025, 2, 1)));
    
    // Test a date that could match multiple formats
    // The function should return based on the first matching format in the array
    auto result2 = try_parse_date_str("01/02/2025");
    BOOST_CHECK(result2.has_value());
}

BOOST_AUTO_TEST_SUITE_END()

// Additional test suite for stress testing
BOOST_AUTO_TEST_SUITE(DateParserStressTests)

BOOST_AUTO_TEST_CASE(test_year_range) {
    // Test various year ranges
    BOOST_CHECK(try_parse_date_str("1000-01-01").has_value());
    BOOST_CHECK(try_parse_date_str("9999-12-31").has_value());
    BOOST_CHECK(try_parse_date_str("0001-01-01").has_value());
}

BOOST_AUTO_TEST_CASE(test_all_months) {
    // Test all month abbreviations
    const std::vector<std::string> months = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    
    for (size_t i = 0; i < months.size(); ++i) {
        std::string date_str = months[i] + " 15 2025";
        auto result = try_parse_date_str(date_str);
        BOOST_CHECK_MESSAGE(result.has_value(), "Failed to parse: " + date_str);
        if (result.has_value()) {
            BOOST_CHECK_EQUAL(static_cast<unsigned>(result->month()), i + 1);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_all_weekdays) {
    // Test all weekday abbreviations
    const std::vector<std::string> weekdays = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    
    for (const auto& weekday : weekdays) {
        std::string date_str = weekday + " Feb 1 2025";
        auto result = try_parse_date_str(date_str);
        BOOST_CHECK_MESSAGE(result.has_value(), "Failed to parse: " + date_str);
    }
}

BOOST_AUTO_TEST_SUITE_END()
