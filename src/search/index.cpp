/**
* The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements those declared in index.h
 *
 * @author Guanyuming He
 */

#include "index.h"

#include <fstream>

#include "webpage.h"
#include <stdexcept>
#include <unistd.h>
#include <xapian.h>

index::~index()
{
	/** 
	 * In principle I would need to call synchronize() here.
	 * However, the Database's commit() is automatically done in the
	 * WritableDatabase's destructor, and thus I don't have to do it
	 * explicitly.
	 * I ascertained it will be done, as the doc indicates 
	 * Database::~Database() is virtual.
	 * Thus, I only need to call save_idmap()
	 */
	if (dirty)
		save_idmap();
}

bool index::has_document(const url& u) const 
{
	auto essential{ u.get_essential() };
	return url2id.contains(essential) || url2id_diff.contains(essential);
}

bool index::has_document(const webpage& w) const 
{
	return has_document(w.url);
}


void index::add_document(const webpage& w)
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
	dirty = true;
	auto id = db.add_document(doc);
	url2id.emplace(w.url.get_essential(), id);
}

void index::synchronize()
{
	if (!dirty)
		return;

	db.commit();
	save_idmap();

	dirty = false;
}

index::idmap_t index::load_idmap(const fs::path& p)
{
	std::ifstream ifs(p, std::ios::binary);
    if (!ifs) // does not exist. Create it.
	{
		ifs.close();

		std::ofstream ofs(p, std::ios::binary);
        if (!ofs) 
			throw std::runtime_error("Unable to create idmap file.");

		// Write size 0 to the new file.
        uint64_t zero = 0;
        ofs.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
		// Return empty map
        return {}; 
	}

	// File exists. Read it.
	idmap_t ret;
	// We have this many entries.
    uint64_t size;
    ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
    for (uint64_t i = 0; i < size; ++i) {
		// Element is \0 terminated string followed by docid.
        std::string key;
        char ch;
        while (ifs.get(ch)) 
		{
            if (ch == '\0') break;
            key += ch;
        }

		xp::docid val;
        ifs.read(reinterpret_cast<char*>(&val), sizeof(val));
        ret.emplace(std::move(key), val);
    }

    return ret;
}

void index::save_idmap()
{
    std::fstream fs(
		url2idpath,
	   	std::ios::in | std::ios::out | std::ios::binary
	);
    if (!fs) 
		throw std::runtime_error("In save_idmap, file must exist.");

    // File exists — read current count
    uint64_t total_count = 0;
    fs.read(reinterpret_cast<char*>(&total_count), sizeof(total_count));
	if (total_count != url2id.size())
		throw std::runtime_error("Index corrupted: idmap size mismatch");

    // Seek to end and append
    fs.seekp(0, std::ios::end);
	for (const auto& [key, val] : url2id_diff) 
	{
        fs.write(key.c_str(), key.size() + 1);
        fs.write(reinterpret_cast<const char*>(&val), sizeof(val));
    }

    // Update count at the beginning
    total_count += url2id_diff.size();
    fs.seekp(0, std::ios::beg);
    fs.write(reinterpret_cast<const char*>(&total_count), sizeof(total_count));

	// Don't forget to clear the diff.
	url2id_diff.clear();
}
