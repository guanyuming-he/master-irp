/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements the class that represents a webpage
 * and related helpers.
 *
 * @author Guanyuming He
 */

#include "webpage.h"
#include <lexbor/html/interfaces/document.h>
#include <stdexcept>

extern "C" {
#include <lexbor/core/base.h>
#include <lexbor/html/parser.h>
}

parser::parser() :
	handle(lxb_html_parser_create())
{
	auto status = lxb_html_parser_init(handle);
	if (LXB_STATUS_OK != status)
	{
		throw std::runtime_error("Can't create a HTML parser.");
	}
}

parser::~parser()
{
	lxb_html_parser_destroy(handle);
}

html::html(const ustring& content, parser& pser) :
	handle(lxb_html_parse(
		pser.handle, content.c_str(), content.size()
	))
{
	if (!handle)
	{
		throw std::runtime_error("Can't parse HTML.");
	}
}

html::~html()
{
	lxb_html_document_destroy(handle);
}
