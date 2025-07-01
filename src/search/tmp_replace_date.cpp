#include "index.h"
#include "url2html.h"
#include "webpage.h"
#include <iostream>
#include <string>

#include <boost/url.hpp>

unsigned num_upd = 0;
url2html convertor{};

bool replace_date_fun(xp::Document& doc)
{
	auto date_str = doc.get_value(index::DATE_SLOT);
	//urls::url u(data_str.substr(
	//			// \t separates url and title.
	//	0, data_str.find_first_of('\t')
	//));
	//webpage wp(std::move(u), convertor);

	//// YYYYMMDD
	//std::string date_str(4+2+2, '\0'); 
	//std::snprintf(
	//	date_str.data(),
	//	date_str.size()+1,
	//	"%04d%02d%02d", 
	//	(int)wp.date.year(),
	//	(unsigned)wp.date.month(),
	//	(unsigned)wp.date.day()
	//);

	//doc.remove_value(index::DATE_SLOT);
	//doc.add_value(index::DATE_SLOT, date_str);
	
	if (date_str.size() != 8)
	{
		std::cout << "date_str=" << date_str << ".size() != 8\n";
	}

	++num_upd;
	if (num_upd % 1000 == 1)
	{
	//	std::cout
	//		<< std::to_string(num_upd) << "th updated.\n";
		std::cout << date_str << std::endl;
	}

	return false;
	//return true;
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr 
			<< "Usage: "
			<< argv[0] << " db_path\n";
		return -1;
	}

	class index i(argv[1]);
	i.upd_all(&replace_date_fun);

	return 0;
}
