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
		using namespace my;
		using T1 = add_lvalue_reference<void>::type;
		using T2 = add_lvalue_reference<int>::type;
		__try_add_pointer<const void>::pointer_type;
		is_same<int, int>::value;
		enable_if<true, int>::type;
		conditional<true, int, float>::type;
		
	}
	
}
