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

	TEST_CLASS(Test_is_base_of) {
	private:
		class A {};
		class B: A {};
		class C: B {};
		class D {};
		union E {};
	public:
		TEST_METHOD(TestMethod) {
			static_assert(my::is_base_of<A, A>::value     == true  &&
				  my::is_base_of<A, B>::value     == true  &&
				  my::is_base_of<A, C>::value     == true  &&
				  my::is_base_of<A, D>::value     == false &&
				  my::is_base_of<B, A>::value     == false &&
				  my::is_base_of<E, E>::value     == false &&
				  my::is_base_of<int, int>::value == false);
		}
	};

	TEST_CLASS(Test_is_convertible) {
	private:
		class E { public: template<class T> E(T&&) {} };
	public:
		TEST_METHOD(TestMethod) {
			class A {};
			class B: public A {};
			class C {};
			class D { public: operator C() { return c; }  C c; };
			class F {
				~F() = delete;
			};
			class G {};
			class H: private G {};

			static_assert(true == my::is_convertible<B*, A*>::value, "");
			static_assert(false == my::is_convertible<A*, B*>::value, "");
			static_assert(false == my::is_convertible<B*, C*>::value, "");
			static_assert(true == my::is_convertible<D, C>::value, "");

			// 完美转发构造函数使类能从任何类型转换
			static_assert(true == my::is_convertible<A, E>::value, ""); //< B, C, D 等

			// static_assert(false == my::is_convertible<F, F>::value, ""); F为非完整类型，未定义行为
			static_assert(false == my::is_convertible<H, G>::value, "");

			static_assert(false == my::is_convertible<int, void>::value, ""); //< B, C, D 等
			static_assert(false == my::is_convertible<void, int>::value, ""); //< B, C, D 等

			// 数组与函数类型不能进行返回
			static_assert(false == my::is_convertible<int*, int[42]>::value, "");
			static_assert(true == my::is_convertible<int[42], int*>::value, "");
			static_assert(true == my::is_convertible<int[], int*>::value, "");
			static_assert(false == my::is_convertible<void(int), void(int)>::value, "");
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
					my::is_same<my::remove_cv_t<int>, int>::value &&
					my::is_same<my::remove_cv_t<const int>, int>::value &&
					my::is_same<my::remove_cv_t<volatile int>, int>::value &&
					my::is_same<my::remove_cv_t<const volatile int>, int>::value &&
					// remove_cv only works on types, not on pointers
					! my::is_same<my::remove_cv_t<const volatile int*>, int*>::value &&
					my::is_same<my::remove_cv_t<const volatile int*>, const volatile int*>::value &&
					my::is_same<my::remove_cv_t<const int* volatile>, const int*>::value &&
					my::is_same<my::remove_cv_t<int* const volatile>, int*>::value
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

	TEST_CLASS(Test_remove_all_extents) {
	public:
		TEST_METHOD(TestMethod) {
			static_assert(my::is_same<float, my::remove_all_extents<float[1][2][3]>::type>::value, "");
			static_assert(my::is_same<int, my::remove_all_extents<int[3][2]>::type>::value, "");
			static_assert(my::is_same<float, my::remove_all_extents<float[1][1][1][1][2]>::type>::value, "");
			static_assert(my::is_same<double, my::remove_all_extents<double[2][3]>::type>::value, "");
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

	#if __HAS_CPP17

	TEST_CLASS(Test_conjunction) {
	private:
		// 若所有 Ts... 都拥有等同于 T 的类型，则启用 func
		template<typename T, typename... Ts>
		::my::enable_if_t<::my::conjunction_v<::my::is_same<T, Ts>...>, bool>
			func(T, Ts...) {
			return true;
		}

		// 否则
		template<typename T, typename... Ts>
		::my::enable_if_t<!::my::conjunction_v<::my::is_same<T, Ts>...>, bool>
			func(T, Ts...) {
			return false;
		}

	public:
		TEST_METHOD(TestMethod) {
			Assert::AreEqual(true, func(1, 2, 3));
			Assert::AreEqual(false, func(1, 2, "Hello"));
		}
	};

	TEST_CLASS(Test_disjunction) {
	private:

		// 傻逼MSVC在20标准中对value_equal抛出C2993错误
		#if !__HAS_CPP20
		// values_equal<a, b, T>::value 为 true 当且仅当 a == b 。
		template <auto V1, decltype(V1) V2, typename T>
		struct values_equal: std::bool_constant<V1 == V2> {
			using type = T;
		};

		// default_type<T>::value 始终为 true
		template <typename T>
		struct default_type: std::true_type {
			using type = T;
		};

		// 现在我们可以像 switch 语句一样使用 disjunction ：
		template <int I>
		using int_of_size = typename my::disjunction<  //
			values_equal<I, 1, std::int8_t>,            //
			values_equal<I, 2, std::int16_t>,           //
			values_equal<I, 4, std::int32_t>,           //
			values_equal<I, 8, std::int64_t>,           //
			default_type<void>                          // 必须在最后！
		>::type;

		#endif

		// 检查 Foo 是否可从 double 构造将导致硬错误
		struct Foo {
			template<class T>
			struct sfinae_unfriendly_check { static_assert(!std::is_same_v<T, double>); };

			template<class T>
			Foo(T, sfinae_unfriendly_check<T> = {});
		};

		template<class... Ts>
		struct first_constructible {
			template<class T, class...Args>
			struct is_constructible_x: std::is_constructible<T, Args...> {
				using type = T;
			};
			struct fallback {
				static constexpr bool value = true;
				using type = void; // 若找不到则用于返回的类型
			};

			template<class... Args>
			using with = typename my::disjunction<is_constructible_x<Ts, Args...>...,
				fallback>::type;
		};

	public:
		TEST_METHOD(TestMethod) {

			
			#if !__HAS_CPP20
			static_assert(sizeof(int_of_size<1>) == 1);
			static_assert(sizeof(int_of_size<2>) == 2);
			static_assert(sizeof(int_of_size<4>) == 4);
			static_assert(sizeof(int_of_size<8>) == 8);
			static_assert(std::is_same_v<int_of_size<13>, void>);
			#endif

			// OK ，不实例化 is_constructible<Foo, double>
			static_assert(std::is_same_v<first_constructible<std::string, int, Foo>::with<double>,
										 int>);

			static_assert(std::is_same_v<first_constructible<std::string, int>::with<>, std::string>);
			static_assert(std::is_same_v<first_constructible<std::string, int>::with<const char*>,
										 std::string>);
			static_assert(std::is_same_v<first_constructible<std::string, int>::with<void*>, void>);
		}
	};

	TEST_CLASS(Test_negation) {
	public:
		TEST_METHOD(TestMethod) {
			static_assert(std::is_same<std::bool_constant<false>, typename std::negation<std::bool_constant<true>>::type>::value, "");
			static_assert(std::is_same<std::bool_constant<true>, typename std::negation<std::bool_constant<false>>::type>::value, "");
		}
	};
	
	#endif

	TEST_CLASS(Test_is_integral) {
	private:
		class A {};

		enum E: int {};

		template <class T>
		T f(T i) {
			static_assert(my::is_integral<T>::value, "Integral required.");
			return i;
		}

	public:
		TEST_METHOD(TestMethod) {
			static_assert(false == my::is_integral<A>::value, "");
			static_assert(false == my::is_integral<E>::value, "");
			static_assert(false == my::is_integral<float>::value, "");
			static_assert(true == my::is_integral<int>::value, "");
			static_assert(true == my::is_integral<bool>::value, "");
			Assert::AreEqual(123, f(123));
		}
	};

	TEST_CLASS(Test_is_floating_point) {
		class A {};
	public:
		TEST_METHOD(TestMethod) {
			static_assert(false == my::is_floating_point<A>::value, "");
			static_assert(true == my::is_floating_point<float>::value, "");
			static_assert(false == my::is_floating_point<float&>::value, "");
			static_assert(true == my::is_floating_point<double>::value, "");
			static_assert(false == my::is_floating_point<double&>::value, "");
			static_assert(false == my::is_floating_point<int>::value, "");
		}
	};

	TEST_CLASS(Test_is_array) {
		class A {};
	public:
		TEST_METHOD(TestMethod) {
			static_assert(false == my::is_array<A>::value, "");
			static_assert(true == my::is_array<A[]>::value, "");
			static_assert(true == my::is_array<A[3]>::value, "");
			static_assert(false == my::is_array<float>::value, "");
			static_assert(false == my::is_array<int>::value, "");
			static_assert(true == my::is_array<int[]>::value, "");
			static_assert(true == my::is_array<int[3]>::value, "");
			static_assert(false == my::is_array<std::array<int, 3>>::value, "");
		}
	};

	TEST_CLASS(Test_is_union) {
	private:
		struct A {};

		typedef union {
			int a;
			float b;
		} B;

		struct C { B d; };
	public:
		TEST_METHOD(TestMethod) {
			static_assert(false == my::is_union<A>::value, "");
			static_assert(true == my::is_union<B>::value, "");
			static_assert(false == my::is_union<C>::value, "");
			static_assert(false == my::is_union<int>::value, "");
		}
	};

	TEST_CLASS(Test_is_class) {
	private:
		struct A {};

		class B {};

		enum class E {};

		union U { class UC {}; };
	public:
		TEST_METHOD(TestMethod) {
			static_assert(false == my::is_class<U>::value, "");
			static_assert(true == my::is_class<U::UC>::value, "");
			static_assert(true == my::is_class<A>::value, "");
			static_assert(true == my::is_class<B>::value, "");
			static_assert(false == my::is_class<B*>::value, "");
			static_assert(false == my::is_class<B&>::value, "");
			static_assert(true == my::is_class<const B>::value, "");
			static_assert(false == my::is_class<E>::value, "");
			static_assert(false == my::is_class<int>::value, "");
			static_assert(true == my::is_class<struct S>::value, "");
			static_assert(true == my::is_class<class C>::value, "");
		}
	};

	TEST_CLASS(Test_is_pointer) {
	private:
		struct A {
			int m;
			void f() {}
		};

		int A::* mem_data_ptr = &A::m;     // a pointer to member data
		void (A::* mem_fun_ptr)() = &A::f; // a pointer to member function

	public:
		TEST_METHOD(TestMethod) {
			static_assert(
				!my::is_pointer<A>::value &&
				!my::is_pointer<A>() && // same as above, using inherited operator bool
				!my::is_pointer<A>{} && // ditto
				!my::is_pointer<A>()() && // same as above, using inherited operator()
				!my::is_pointer<A>{}() &&  // ditto
				my::is_pointer<A*>::value &&
				my::is_pointer<A const* volatile>::value &&
				!my::is_pointer<A&>::value &&
				!my::is_pointer<decltype(mem_data_ptr)>::value &&
				!my::is_pointer<decltype(mem_fun_ptr)>::value &&
				my::is_pointer<void*>::value &&
				!my::is_pointer<int>::value &&
				my::is_pointer<int*>::value &&
				my::is_pointer<int**>::value &&
				!my::is_pointer<int[10]>::value &&
				!my::is_pointer<std::nullptr_t>::value &&
				my::is_pointer<void (*)()>::value
			);
		}
	};

	TEST_CLASS(Test_is_lvalue_reference) {
	private:
		class A {};

	public:
		TEST_METHOD(TestMethod) {
			static_assert(false == my::is_lvalue_reference<A>::value, "");
			static_assert(true == my::is_lvalue_reference<A&>::value, "");
			static_assert(false == my::is_lvalue_reference<A&&>::value, "");
			static_assert(false == my::is_lvalue_reference<int>::value, "");
			static_assert(true == my::is_lvalue_reference<int&>::value, "");
			static_assert(false == my::is_lvalue_reference<int&&>::value, "");
		}
	};

	TEST_CLASS(Test_is_rvalue_reference) {
	private:
		class A {};

		template <typename T>
		int test(T&& x) {
			static_assert(std::is_same_v<T&&, decltype(x)>);

			int acc = 0;
			acc += 100 * my::is_rvalue_reference<T>::value;
			acc += 10 * my::is_rvalue_reference<T&&>::value;
			acc += 1 * my::is_rvalue_reference<decltype(x)>::value;
			return acc;
		}

	public:
		TEST_METHOD(TestMethod) {
			static_assert(
				my::is_rvalue_reference<A>::value == false &&
				my::is_rvalue_reference<A&>::value == false &&
				my::is_rvalue_reference<A&&>::value != false &&
				my::is_rvalue_reference<char>::value == false &&
				my::is_rvalue_reference<char&>::value == false &&
				my::is_rvalue_reference<char&&>::value != false
			);

			Assert::AreEqual(11, test(42));

			int x = 42;
			Assert::AreEqual(0, test(x));
		}
	};

	TEST_CLASS(Test_is_function) {
	private:
		struct A { int fun() const&; };

		template<typename>
		struct PM_traits {};

		template<class T, class U>
		struct PM_traits<U T::*> { using member_type = U; };

	public:
		TEST_METHOD(TestMethod) {
			static_assert(false == my::is_function<A>::value, "");
			static_assert(true == my::is_function<int(int)>::value, "");
			static_assert(false == my::is_function<int>::value, "");

			using T = PM_traits<decltype(&A::fun)>::member_type; // T is int() const&
			static_assert(true == my::is_function<T>::value, "");
		}
	};

	TEST_CLASS(Test_is_member_function_object) {
	private:
		class A {
		public:
			void member() {}
		};

	public:
		TEST_METHOD(TestMethod) {
			// fails at compile time if A::member is a data member and not a function
			static_assert(my::is_member_function_pointer<decltype(&A::member)>::value,
						  "A::member is not a member function.");
		}
	};

	TEST_CLASS(Test_is_member_object_object) {
	public:
		TEST_METHOD(TestMethod) {
			class A {};
			static_assert(true == my::is_member_object_pointer<int(A::*)>::value, "");
			static_assert(false == my::is_member_object_pointer<int(A::*)()>::value, "");
		}
	};

	TEST_CLASS(Test_is_enum) {
	private:
		struct A { enum E {}; };

		enum E {};

		enum class Ec: int {};

	public:
		TEST_METHOD(TestMethod) {
			static_assert(my::is_enum<A>::value == false, "");
			static_assert(my::is_enum<A::E>::value == true, "");
			static_assert(my::is_enum<E>::value == true, "");
			static_assert(my::is_enum<Ec>::value == true, "");
			static_assert(my::is_enum<int>::value == false, "");
		}
	};

	TEST_CLASS(Test_decay) {
	private:
		template<typename T, typename U>
		constexpr static bool is_decay_equ = my::is_same<typename my::decay<T>::type, U>::value;
	public:
		TEST_METHOD(TestMethod) {
			static_assert (
				is_decay_equ<int, int> &&
				!is_decay_equ<int, float> &&
				is_decay_equ<int&, int> &&
				is_decay_equ<int&&, int> &&
				is_decay_equ<const int&, int> &&
				is_decay_equ<int[2], int*> &&
				!is_decay_equ<int[4][2], int*> &&
				!is_decay_equ<int[4][2], int**> &&
				is_decay_equ<int[4][2], int(*)[2]> &&
				is_decay_equ<int(int), int(*)(int)>
			);
		}
	};

	TEST_CLASS(Test_common_type) {
	private:
		template <class T>
		struct Number { 
			T n; 

			template <typename U>
			Number<typename my::common_type<T, U>::type> operator+(const Number<U>& rhs) {
				return { this->n + rhs.n };
			}
		};
	public:
		TEST_METHOD(TestMethod) {
			Number<int> i1 = { 1 }, i2 = { 2 };
			Number<double> d1 = { 2.3 }, d2 = { 3.5 };

			Assert::IsTrue(my::is_same<Number<int>, decltype(i1 + i2)>::value);
			Assert::AreEqual(3, (i1 + i2).n);
			Assert::IsTrue(my::is_same<Number<double>, decltype(i1 + d2)>::value);
			Assert::AreEqual(4.5, (i1 + d2).n);
			Assert::IsTrue(my::is_same<Number<double>, decltype(d1 + i2)>::value);
			Assert::AreEqual(4.3, (d1 + i2).n);
			Assert::IsTrue(my::is_same<Number<double>, decltype(d1 + d2)>::value);
			Assert::AreEqual(5.8, (d1 + d2).n);
		}
	};
}
