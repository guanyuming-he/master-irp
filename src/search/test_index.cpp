/**
 * Unit tests for the index class.
 *
 * Copyright (C) 2025 Guanyuming He
 * The file is licensed under the GNU GPL v3.
 * 
 * Tests are written using Boost.Test
 * Tests were generated using Claude.ai 
 * and then modified by myself.
 */
#define BOOST_TEST_MODULE IndexTests

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <xapian.h>
#include <filesystem>
#include <fstream>

#include "index.h"
#include "webpage.h"
#include "url.h"
#include "url2html.h"

namespace fs = std::filesystem;
namespace xp = Xapian;

