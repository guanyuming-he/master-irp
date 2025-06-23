/**
* The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements those declared in index.h
 *
 * @author Guanyuming He
 */

#include "index.h"

#include "webpage.h"
#include <xapian.h>

bool index::has_document(const url& u) const 
{
	return url2id.contains(u.get_essential());
}

bool index::has_document(const webpage& w) const 
{
	return has_document(w.url);
}

void writable_index::add_document(const webpage& w)
{ 
	// do not index an empty document.
	if (w.title.empty() && w.get_text().empty())
		return;

	xp::Document doc;
	tg.set_document(doc);

	// The magical strings "S" and "XD" are from the official
	// example https://getting-started-with-xapian.readthedocs.io/en
	// /latest/practical_example/indexing/writing_the_code.html,
	// which claims that they are the conventional prefixes of the 
	// omega search engine.
	tg.index_text(w.title, 1, "S");
	tg.index_text(w.get_text(), 1, "XD");

	// Index them without prefixes for free search 
	tg.index_text(w.title);
	tg.increase_termpos();
	tg.index_text(w.get_text());

	// We can now store the doc in the database.
	wdb.add_document(doc);
}
