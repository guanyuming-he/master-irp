#include <cstddef>
#include <iostream>
extern "C" {
#include <lexbor/core/types.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/interface.h>
#include <lexbor/html/interfaces/document.h>
}

const char html_with_links[] = R"(
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

lxb_html_token_t *
token_callback(
	[[maybe_unused]] lxb_html_tokenizer_t *tkz, 
	lxb_html_token_t *token, 
	[[maybe_unused]] void *ctx
)
{
	return token;
}

int main()
{
	auto* parser = lxb_html_parser_create();
	lxb_html_parser_init(parser);

	parser->tkz->callback_token_done = &token_callback;
	parser->tkz->callback_token_ctx = nullptr;

	auto* doc = lxb_html_parse(
		parser, 
		(const lxb_char_t*)html_with_links,
		sizeof(html_with_links) - 1
	);

	std::cout << doc->head << ", " <<
		doc->body << std::endl;

	lxb_html_document_destroy(doc);
	lxb_html_parser_destroy(parser);

	return 0;
}
