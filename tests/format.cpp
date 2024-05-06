#include <iostream>

void someFunction()
{
	// this line is splitted - Leo has
	std::cout << "\033[1;33m"
			  << "Reading headers"
			  << "\033[0m" << std::endl;

	// I have
	// std::cout << "\033[1;36m"
	//          << "Entering buildResponse"
	//          << "\033[0m" << std::endl;

	// Leo has
	// std::cout << "\033[1;33m" << "Reading headers" << "\033[0m" << std::endl;

	// a space is added
	// Leo has
	// return ;
	// I have
	// return;
	return;
}

void anotherFunction()
{
	// I have a space after// in the comments, Leo has not
	// std::cout << parser.getBuffer() << std::endl;
	// std::cout << parser.getBuffer() << std::endl;
}

int main()
{
	someFunction();
	anotherFunction();

	return 0;
}
