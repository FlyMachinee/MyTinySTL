#include "pch.h"
#include "CppUnitTest.h"

#include "../MyTinySTL/type_traits.hpp"

#include <iostream>
#include <cstdint>
#include <typeinfo>
#include <map>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Test_type_traits {
	TEST_CLASS(Test_integral_constant) {
public:
		TEST_METHOD(TestMethod) {
			typedef my::integral_constant<int, 2> two_t;
			typedef my::integral_constant<int, 4> four_t;

			static_assert(! my::is_same<two_t, four_t>::value,
						  "two_t and four_t are equal!");

			static_assert(two_t::value * 2 == four_t::value, "2*2 != 4");

			enum class my_e { e1, e2 };

			typedef my::integral_constant<my_e, my_e::e1> my_e_e1;
			typedef my::integral_constant<my_e, my_e::e2> my_e_e2;

			static_assert(my_e_e1() == my_e::e1, "");

			static_assert(my_e_e1::value != my_e::e2,
						 "my_e_e1::value == my_e::e2");

			static_assert(my::is_same<my_e_e2, my_e_e2>::value,
						  "my_e_e2 != my_e_e2");
		}
	};

	#if __HAS_CPP17
	TEST_CLASS(Test_void_t) {
		// An iterator trait those value_type is the value_type of the iterated container,
		// supports even back_insert_iterator (where value_type is void)

		template<typename T, typename = void>
		struct iterator_trait: std::iterator_traits<T> {};

		template<typename T>
		struct iterator_trait<T, my::void_t<typename T::container_type>>
			: std::iterator_traits<typename T::container_type::iterator> {
		};

		class A {};

	public:
		TEST_METHOD(TestMethod) {
			using container_t = std::vector<int>;
			container_t v;

			static_assert(my::is_same<
				container_t::value_type,
				iterator_trait<decltype(std::begin(v))>::value_type
			>::value, "");

			static_assert(my::is_same<
				container_t::value_type,
				iterator_trait<decltype(std::back_inserter(v))>::value_type
			>::value, "");
		}
	};
	#endif

	TEST_CLASS(Test_is_same) {
	public:
		TEST_METHOD(TestMethod) {		 
			// compare the types of a couple variables
			long double num1 = 1.0;
			long double num2 = 2.0;
			static_assert( my::is_same<decltype(num1), decltype(num2)>::value == true ,"");
		 
			// 'float' is never an integral type
			static_assert( my::is_same<float, std::int32_t>::value == false ,"");
		 
			// 'int' is implicitly 'signed'
			static_assert( my::is_same<int, int>::value == true ,"");
			static_assert( my::is_same<int, unsigned int>::value == false ,"");
			static_assert( my::is_same<int, signed int>::value == true ,"");
		 
			// unlike other types, 'char' is neither 'unsigned' nor 'signed'
			static_assert( my::is_same<char, char>::value == true ,"");
			static_assert( my::is_same<char, unsigned char>::value == false ,"");
			static_assert( my::is_same<char, signed char>::value == false ,"");
		 
			// const-qualified type T is not same as non-const T
			static_assert( !my::is_same<const int, int>() ,"");
		}
	};

	TEST_CLASS(Test_conditional) {
	public:
		TEST_METHOD(TestMethod) {
			using Type1 = my::conditional<true, int, double>::type;
			using Type2 = my::conditional<false, int, double>::type;
			using Type3 = my::conditional<sizeof(int) >= sizeof(double), int, double>::type;

			static_assert(my::is_same<Type1, int>::value, "");
			static_assert(my::is_same<Type2, double>::value, "");
			static_assert(my::is_same<Type3, double>::value, "");
		}
	};

	TEST_CLASS(Test_remove_cv) {
	public:
		TEST_METHOD(TestMethod) {
			static_assert
				(
					my::is_same<std::remove_cv_t<int>, int>::value &&
					my::is_same<std::remove_cv_t<const int>, int>::value &&
					my::is_same<std::remove_cv_t<volatile int>, int>::value &&
					my::is_same<std::remove_cv_t<const volatile int>, int>::value &&
					// remove_cv only works on types, not on pointers
					! my::is_same<std::remove_cv_t<const volatile int*>, int*>::value &&
					my::is_same<std::remove_cv_t<const volatile int*>, const volatile int*>::value &&
					my::is_same<std::remove_cv_t<const int* volatile>, const int*>::value &&
					my::is_same<std::remove_cv_t<int* const volatile>, int*>::value
				, "");
		}
	};

	TEST_CLASS(Test_add_cv) {
		struct foo
		{
			int m() { return 0; }
			int m() const { return 1; }
			int m() volatile { return 2; }
			int m() const volatile { return 3; }
		};

	public:
		TEST_METHOD(TestMethod) {
			Assert::AreEqual(foo{}.m(), 0);
			Assert::AreEqual(my::add_const<foo>::type{}.m(), 1);
			Assert::AreEqual(my::add_volatile<foo>::type{}.m(), 2);
			Assert::AreEqual(my::add_cv<foo>::type{}.m(), 3);
		}
	};

	TEST_CLASS(Test_remove_reference) {
	public:
		TEST_METHOD(TestMethod) {
			static_assert(my::is_same<int, my::remove_reference<int>::type>::value == true, "");
			static_assert(my::is_same<int, my::remove_reference<int&>::type>::value == true, "");
			static_assert(my::is_same<int, my::remove_reference<int&&>::type>::value == true, "");
			static_assert(my::is_same<const int, my::remove_reference<const int&>::type>::value == true, "");
		}
	};

	TEST_CLASS(Test_add_reference) {
	public:
		TEST_METHOD(TestMethod) {
			using non_ref = int;
			using l_ref = typename my::add_lvalue_reference_t<non_ref>;
			using r_ref = typename my::add_rvalue_reference_t<non_ref>;
			using void_ref = my::add_lvalue_reference_t<void>;

			static_assert
				(std::is_lvalue_reference_v<non_ref> == false
				&& std::is_lvalue_reference_v<l_ref> == true
				&& std::is_rvalue_reference_v<r_ref> == true
				&& std::is_reference_v<void_ref> == false
				, "");
		}
	};

	TEST_CLASS(Test_remove_pointer) {
	public:
		TEST_METHOD(TestMethod) {
			static_assert
				(
					std::is_same_v<int, int> == true &&
					std::is_same_v<int, int*> == false &&
					std::is_same_v<int, int**> == false &&
					std::is_same_v<int, my::remove_pointer<int>::type> == true &&
					std::is_same_v<int, my::remove_pointer<int*>::type> == true &&
					std::is_same_v<int, my::remove_pointer<int**>::type> == false &&
					std::is_same_v<int, my::remove_pointer<int* const>::type> == true &&
					std::is_same_v<int, my::remove_pointer<int* volatile>::type> == true &&
					std::is_same_v<int, my::remove_pointer<int* const volatile>::type> == true
				, "");
		}
	};

	TEST_CLASS(Test_add_pointer) {
		template<typename F, typename Class>
		static void ptr_to_member_func_cvref_test(F Class::*) {
			// F is an “abominable function type”
			using FF = my::add_pointer<F>::type;
			static_assert(std::is_same_v<F, FF>, "FF should be precisely F");
		}

		struct S {
			void f_ref()& {}
			void f_const() const {}
		};

	public:
		TEST_METHOD(TestMethod) {
			int i = 123;
			int& ri = i;
			typedef my::add_pointer<decltype(i)>::type IntPtr;
			typedef my::add_pointer<decltype(ri)>::type IntPtr2;
			IntPtr pi = &i;
			Assert::AreEqual(i, 123);
			Assert::AreEqual(*pi, 123);

			static_assert(std::is_pointer_v<IntPtr>, "IntPtr should be a pointer");
			static_assert(std::is_same_v<IntPtr, int*>, "IntPtr should be a pointer to int");
			static_assert(std::is_same_v<IntPtr2, IntPtr>, "IntPtr2 should be equal to IntPtr");

			typedef std::remove_pointer<IntPtr>::type IntAgain;
			IntAgain j = i;
			Assert::AreEqual(j, 123);

			static_assert(!std::is_pointer_v<IntAgain>, "IntAgain should not be a pointer");
			static_assert(std::is_same_v<IntAgain, int>, "IntAgain should be equal to int");

			ptr_to_member_func_cvref_test(&S::f_ref);
			ptr_to_member_func_cvref_test(&S::f_const);
		}
	};

	TEST_CLASS(Test_is_void) {
		static void foo() {};

	public:
		TEST_METHOD(TestMethod) {
			static_assert
				(
					my::is_void<void>::value == true &&
					my::is_void<const void>::value == true &&
					my::is_void<volatile void>::value == true &&
					my::is_void<void*>::value == false &&
					my::is_void<int>::value == false &&
					my::is_void<decltype(&foo)>::value == false &&
					my::is_void<my::is_void<void>>::value == false
				, "");
		}
	};

	TEST_CLASS(Test_is_null_pointer) {
	public:
		TEST_METHOD(TestMethod) {
			static_assert(my::is_null_pointer<decltype(nullptr)>::value, "");
			static_assert(!my::is_null_pointer<int*>::value, "");
			static_assert(!std::is_pointer<decltype(nullptr)>::value, "");
			static_assert(std::is_pointer<int*>::value, "");
		}
	};

}
