#include "Preprocessor.h"
#include <iostream>

int main()
{
	try
	{
		Preprocessor p;
		std::cout << p.doFile("e:/file.c");
	}
	catch (std::string &s)
	{
		std::cout << s;
	}

	return 0;
}