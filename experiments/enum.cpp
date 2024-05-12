#include <iostream>

enum Color
{
	RED,
	GREEN,
	BLUE
};

int main()
{
	Color shirtColor = RED;

	if (shirtColor == RED)
	{
		std::cout << "The shirt is red." << std::endl;
	}
	else if (shirtColor == GREEN)
	{
		std::cout << "The shirt is green." << std::endl;
	}
	else if (shirtColor == BLUE)
	{
		std::cout << "The shirt is blue." << std::endl;
	}
	else
	{
		std::cout << "Unknown color." << std::endl;
	}

	return 0;
}
