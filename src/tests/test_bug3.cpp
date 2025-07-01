#include "../search/searcher.h"
#include <iostream>

#include <xapian.h>

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr
			<< "Usage: "
			<< argv[0] << " db_path" << std::endl;
		return -1;
	}	

	searcher s(argv[1]);
	
	// Investigate why dd/mm/yyyy..dd/mm/yyyy 
	// will give the exception.
	try 
	{
		s.query("Trump 01/01/2003..01/01/2025");
	}
	catch (const xp::QueryParserError& e)
	{
		std::cerr 
			<< "QueryParserError:\n"
			<< e.get_msg() << '\n'
			<< e.get_description() << '\n'
			<< e.get_error_string() << '\n'
			<< e.get_context() << '\n';
	}


	return 0;
}
