#include <iostream>
#include <memory>

#include <xapian.h>


namespace xp = Xapian;

int main()
{
	xp::WritableDatabase wd("");
	std::unique_ptr<xp::Database> uptr(std::make_unique<xp::WritableDatabase>(wd));

	xp::WritableDatabase *ptr1 = dynamic_cast<xp::WritableDatabase*>(uptr.get());
 
	std::cout << ptr1 << std::endl;
	// see if it can work.
	ptr1->close();

	return 0;
}
