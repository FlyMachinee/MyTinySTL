#include <iostream>
#include "type_traits.hpp"

int main()
{
	std::cout << "Hello World!\n";

	std::cout << ::my::true_type::value << std::endl;
	std::cout << ::my::false_type::value << std::endl;

	std::cout << ::my::is_void<void>::value << std::endl;
	std::cout << ::my::is_void<int>::value << std::endl;
	std::cout << ::my::is_void<const void>::value << std::endl;
	std::cout << ::my::is_void<const volatile void>::value << std::endl;

	std::cout << ::my::is_same<int&, ::my::remove_reference_t<int&&>>::value << std::endl;

	{
	}
	
}
