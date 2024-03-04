#pragma once
#include "macros.h"

#if __HAS_CPP11
namespace __MY_NAMESPACE {

	// ==========================
	//
	//	Helper classes and alias
	//  帮助类及别名
	//
	// ==========================


	/**
	 * @brief helper class, as a compile-time constant
	 * @brief 一个可作为编译器常量的帮助类
	 * @tparam T 存放常量的类型
	 * @tparam Value 存放常量的值
	*/
	template <typename T, T Value>
	struct integral_constant {

		/** @brief 存放常量的类型 */
		using value_type = T;

		/** @brief 自身的类型 */
		using type = integral_constant; 
		// same as integral_constant<T, Value>

		/** @brief 存储该常量的静态成员 */
		constexpr static value_type value = Value;

		/** @brief 使用C风格强制类型转换获取其存放的常量值 */
		constexpr operator value_type() const noexcept { return value; }

		#if __HAS_CPP14
		/**
		 * @brief 使用仿函数风格获取其存放的常量值
		 * @return 其存放的常量值
		*/
		constexpr value_type operator()() const noexcept { return value; }
		#endif // __HAS_CPP14

	}; // struct integral_constant

	#if __HAS_CPP17
	/**
	 * @brief helper alias template for integral_constant, when T is bool
	 * @brief 编译期布尔常量帮助类的别名模板
	 * @tparam Boolean 存放的布尔值
	*/
	template <bool Boolean>
	using bool_constant = integral_constant<bool, Boolean>;
	#endif // __HAS_CPP17

	/**
	 * @brief alias, compile-time boolean true constant
	 * @brief 表示编译期布尔常量 true 的帮助类
	*/
	using true_type = integral_constant<bool, true>;

	/**
	 * @brief alias, compile-time boolean false constant
	 * @brief 表示编译期布尔常量 false 的帮助类
	*/
	using false_type = integral_constant<bool, false>;



	// =======================================================================
	//
	//  base tools for metaprograming before the C++20's concepts published, 
	//  based on the technique: SFINAE (Substitution Failure Is Not An Error)
	//  在C++20的概念与约束出现之前，用于模版元编程的一些好用的帮助类
	//  通过它们，能够很好地利用SFINAE（替换失败不是错误）特性来进行模板元编程
	//
	// =======================================================================

	// void variadic alias template
	// 将任意类型集合 映射至 void, 结合不推导语句与SFINAE 实现对模板形参中的某些类型进行约束
	#pragma region void_t (>=C++17)

	#if __HAS_CPP17
	/**
	 * @brief void variadic alias template
	 * @breif 将任意类型集合 映射至 void, 结合不推导语句与SFINAE 实现对模板形参中的某些类型进行约束
	 * @tparam ... 任意的类型
	 *
	 * 常见用法: 
	 *		template <class T, class = void> struct XXX {}; <== 特殊情况下(EXPRESSION_of_T 不符合语法)的模版
	 *		template <class T> struct XXX<T, typename void_t< EXPRESSION_of_T >> {}; <== 一般情况下(EXPRESSION_of_T 符合语法)的模版
	 *		template <class T> using XXX_WRAPPER = struct XXX<T>;
	 * 解释：
	 *		这里的 <T, typename void_t< EXPRESSION_of_T >> 中, 第二个形参typename void_t< EXPRESSION_of T >为不推导语境
	 *		其类型将根据第一个类型形参T进行计算, 而不是根据传入的第二个实参参与推导
	 *		当T满足EXPRESSION时, void_t< EXPRESSION_of_T >这句话是语法正确的, 故此时选择了struct XXX的偏特化版本, 即所谓的一般情况
	 *		而当T不满足EXPRESSION时, void_t< EXPRESSION_of_T >这句话是不符合语法的, 是无法编译的, 出现了替换失败(Substitution failure)
	 *		根据SFINAF, 这不是错误, 此时会转向去寻找其他的备选方案, 从而特化了struct XXX的主模版, 即所谓的特殊情况
	 * 
	 * 在C++20可使用concept进行更人性化的编程
	*/
	template <typename ...>
	using void_t = void;

	template <typename ...>
	using __void_t = void;
	#else
	template<typename... Ts>
	struct __make_void { using type = void; };

	template<typename... Ts>
	using __void_t = typename __make_void<Ts...>::type;	
	#endif // __HAS_CPP17

	#pragma endregion

	#pragma region enable_if
	/**
	 * @brief conditionally removes a function overload or template specialization from overload resolution
	 * @brief 根据传入的布尔常量, 决定其是否包含type成员. 结合不推导语句与SFINAE实现对模板形参中的某些类型进行约束
	 * @tparam Boolean 标识该结构体是否包含成员type的布尔值 
	 * @tparam T 成员type表示的类型
	 *
	 * 常见用法: 
	 *		template <class T, class = void> struct XXX {}; <== 一般情况下的模版
	 *		template <class T> struct XXX<T, typename enable_if< CONDITION<T> >::type>> {}; <== 特殊情况下(满足CONDITION<T>为true)的模版
	 *		template <class T> using XXX_WRAPPER = struct XXX<T>;
	 * 解释：
	 *		这里的 <T, typename enable_if< CONDITION<T> >::type>> 中, 第二个形参typename enable_if< CONDITION<T> >::type>为不推导语境
	 *		其类型将根据第一个类型形参T进行计算, 而不是根据传入的第二个实参参与推导
	 *		当T满足CONDITION时, enable_if的非类型模板实参为true, 特化了那个包含成员type的enable_if偏特化模板
	 *		此时typename enable_if< CONDITION<T> >::type>这句话是语法正确的, 故此时选择了struct XXX的偏特化版本, 即一般情况
	 *		而当T不满足CONDITION时, enable_if的非类型模板实参为false, 特化了那个没有成员的enable_if主模板
	 *		此时typename enable_if< CONDITION<T> >::type>这句话是无法编译的, 出现了替换失败(Substitution failure)
	 *		根据SFINAF, 这不是错误, 此时会转向去寻找其他的备选方案, 从而特化了struct XXX的主模版, 即特殊情况
	 * 
	 * 在C++20可使用concept进行更人性化的编程
	*/
	template <bool Boolean, typename T = void>
	struct enable_if {};

	template <typename T> // specialization for Boolean = true
	struct enable_if<true, T> { using type = T; };

	#pragma endregion



	// ====================================================
	// 
	//	type relationships and conditional selections
	//  类型关系及条件选择
	//
	// ====================================================

	// checks if two types are the same
	// 检查两个类型是否相同, 其值即表示其结果
	#pragma region is_same

	/**
	 * @brief checks if two types are the same
	 * @brief 检查两个类型是否相同, 包含成员value表示其结果值 
	*/
	template <typename, typename>
	struct is_same: false_type {};

	/**
	 * @brief checks if two types are the same
	 * @brief 检查两个类型是否相同, 包含成员value表示其结果值 
	*/
	template <typename T>
	struct is_same<T, T>: true_type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if two types are the same
	 * @brief 检查两个类型是否相同, 其值即表示其结果 
	*/
	template <typename T1, typename T2>
	inline constexpr bool is_same_v = is_same<T1, T2>::value;
	#endif // __HAS_CPP17

	#pragma endregion

	// (non-standard feature) checks if one type equals to any other types
	// (非标准内容) 判断一个类型是否与后续类型之一相等
	#pragma region is_any_of

	#pragma endregion

	// checks if a type is derived from the other type
	// 检查一个类型是否派生自另一个类型
	#pragma region is_base_of




	#if __HAS_CPP17
	
	#endif // __HAS_CPP17

	#pragma endregion

	// chooses one type or another based on compile-time boolean
	// 根据传入的布尔常量, 在两个传入的类型中进行选择
	#pragma region conditional

	template <bool Boolean, typename TrueType, typename FalseType>
	struct conditional { using type = TrueType; };

	template <typename TrueType, typename FalseType>
	struct conditional<false, TrueType, FalseType> { using type = FalseType; };

	#if __HAS_CPP14
	template <bool Boolean, typename TrueType, typename FalseType>
	using conditional_t = typename conditional<Boolean, TrueType, FalseType>::type;
	#endif // __HAS_CPP14

	#pragma endregion



	// ====================================================
	// 
	//	simple type transformations
	//  简单的类型变换
	//
	// ====================================================

	// returns the type argument unchanged
	// 恒等类型变换, 常用于构造不推导语境
	#pragma region type_identity (>=C++20)

	#if __HAS_CPP20
	template <typename T>
	struct type_identity { using type = T; };
	#endif // __HAS_CPP20

	#pragma endregion



	// ====================================================
	// 
	//	cv specifiers add and remove
	//  cv 限定符(const 与 volatile) 的引入与移除
	//
	// ====================================================

	// removes const specifiers from the given type
	#pragma region remove_const

	template <typename T>
	struct remove_const { using type = T; };

	template <typename T>
	struct remove_const<const T> { using type = T; };

	#if __HAS_CPP14
	template <typename T>
	using remove_const_t = typename remove_const<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion

	// removes voatile specifiers from the given type
	#pragma region remove_volatile

	template <typename T>
	struct remove_volatile { using type = T; };

	template <typename T>
	struct remove_volatile<volatile T> { using type = T; };

	#if __HAS_CPP14
	template <typename T>
	using remove_volatile_t = typename remove_volatile<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion

	// removes const and volatile specifiers from the given type
	#pragma region remove_cv

	template <typename T>
	struct remove_cv { using type = T; };

	template <typename T>
	struct remove_cv<const T> { using type = T; };

	template <typename T>
	struct remove_cv<volatile T> { using type = T; };

	template <typename T>
	struct remove_cv<const volatile T> { using type = T; };

	#if __HAS_CPP14
	template <typename T>
	using remove_cv_t = typename remove_cv<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion

	// add const specifiers from the given type
	#pragma region add_const

	template <typename T>
	struct add_const { using type = const T; };

	#if __HAS_CPP14
	template <typename T>
	using add_const_t = typename add_const<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion	

	// add volatile specifiers from the given type
	#pragma region add_volatile

	template <typename T>
	struct add_volatile { using type = volatile T; };

	#if __HAS_CPP14
	template <typename T>
	using add_volatile_t = typename add_volatile<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion

	// add const and volatile specifiers from the given type
	#pragma region add_cv

	template <typename T>
	struct add_cv { using type = const volatile T; };

	#if __HAS_CPP14
	template <typename T>
	using add_cv_t = typename add_cv<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion



	// ====================================================
	// 
	//	reference add and remove
	//  引用的引入与移除
	//
	// ====================================================

	// removes a reference from the given type
	// 移除类型中的引用
	#pragma region remove_reference

	template <typename T>
	struct remove_reference { using type = T; };

	template <typename T>
	struct remove_reference<T&> { using type = T; };

	template <typename T>
	struct remove_reference<T&&> { using type = T; }; // 这里的T&&不是万能引用(转发引用)(universal reference), 只是限制了输入的实参为右值引用

	// 解释:
	//		输入			int				int&			int&&
	//		主模板		ok(T=int)<-		ok(T=int&)		ok(T=int&&)
	//		特化<T&>		x				ok(T=int)<-		x
	//		特化<T&&>	x				x				ok(T=int)<-
	//		最终T		int				int				int
	//	模版选择规则(不准确的): 有特化模板可行时选择特化模板, 没有特化模板可行时选择主模板
	//						有多个特化模板可行时, 选择"最特殊"的那个模板
	//						A<>比B<>"特殊"意为, A<>的所有可行输入是B<>所有可行输入的真子集
	//						如果没有这样"最特殊"的模板, 则编译失败

	#if __HAS_CPP14
	template <typename T>
	using remove_reference_t = typename remove_reference<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion remove_reference


	// adds an lvalue and rvalue reference to the given type
	// 为类型引入左值引用及右值引用, 遵循引用折叠原则
	#pragma region add_lvalue_reference add_rvalue_reference

	#if __HAS_CPP20
	// handle non-reference-able type
	// 处理不可引用的类型
	template <typename T>
	struct __try_add_reference {
		using lvalue_type = T;
		using rvalue_type = T;
	};

	// handle reference-able type
	// 处理可引用的类型
	template <typename T> requires requires { typename type_identity<T&>; }
	struct __try_add_reference<T> {
		using lvalue_type = T&;
		using rvalue_type = T&&;
	};

	#else // __HAS_CPP <= 17
	template <typename T, typename = void>
	struct __try_add_reference {
		using lvalue_type = T;
		using rvalue_type = T;
	};

	template <typename T>
	struct __try_add_reference<T, typename __void_t<T&>> {
		using lvalue_type = T&;
		using rvalue_type = T&&;
	};
	#endif // definition of __try_add_reference

	template <typename T>
	struct add_lvalue_reference { using type = typename __try_add_reference<T>::lvalue_type; };

	template <typename T>
	struct add_rvalue_reference { using type = typename __try_add_reference<T>::rvalue_type; };

	#if __HAS_CPP14
	template <typename T>
	using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

	template <typename T>
	using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion add_lvalue_reference add_rvalue_reference



	// ====================================================
	// 
	//	pointer add and remove
	//  指针的引入与移除
	//
	// ====================================================

	// removes a pointer from the given type
	// 移除类型中的指针及其顶层cv限定符
	#pragma region remove_pointer

	template <typename T>
	struct remove_pointer { using type = T; };
	template <typename T>
	struct remove_pointer<T*> { using type = T; };
	template <typename T>
	struct remove_pointer<T* const> { using type = T; };
	template <typename T>
	struct remove_pointer<T* volatile> { using type = T; };
	template <typename T>
	struct remove_pointer<T* const volatile> { using type = T; };

	#if __HAS_CPP14
	template <typename T>
	using remove_pointer_t = typename remove_pointer<T>::type;
	#endif // __HAS_CPP14


	#pragma endregion

	// adds a pointer to the given type
	// 为类型引入指针, 对于不可引用的类型(除去void及其cv衍生物), 返回其本身
	#pragma region add_pointer

	#if __HAS_CPP20
	// handle non-reference-able type except void
	// 处理不可引用的类型, 除了void
	template <typename T>
	struct add_pointer { using type = T; };

	// handle reference-able type and void
	// 处理可引用的类型, 以及void
	template <typename T> requires requires { typename type_identity<typename remove_reference<T>::type*>>; }
	struct add_pointer<T> { using pointer_type = typename remove_reference<T>::type*; };

	#else // __HAS_CPP <= 17
	template <typename T, typename = void>
	struct __try_add_pointer {
		using pointer_type = T;
	};

	template <typename T>
	struct __try_add_pointer<T, typename __void_t<typename remove_reference<T>::type*>> {
		using pointer_type = typename remove_reference<T>::type*;
	};

	template <typename T>
	struct add_pointer { using type = typename __try_add_pointer<T>::pointer_type; };
	#endif // definition of add_pointer

	#if __HAS_CPP14
	template <typename T>
	using add_pointer_t = typename add_pointer<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion add_pointer



	// ====================================================
	// 
	//	Primary type categories and judges in compile-time
	//  原始类型及其分类的编译期内判断
	//
	// ====================================================

	// checks if a type is void
	// 检查类型是否为 void 及其cv衍生物
	#pragma region is_void

	template <typename T>
	struct is_void: is_same<void, typename remove_cv<T>::type> {};

	#if __HAS_CPP17
	template <typename T>
	inline constexpr bool is_void_v = is_void<T>::value;
	#endif // __HAS_CPP17

	#pragma endregion

	// checks if a type is std::nullptr_t
	// 检查类型是否为 std::nullptr_t 及其cv衍生物
	#pragma region is_null_pointer
	#if __HAS_CPP14
	template <typename T>
	struct is_null_pointer: is_same<typename remove_cv<T>::type, ::std::nullptr_t> {};

	#if __HAS_CPP17
	template <typename T>
	inline constexpr bool is_null_pointer_v = is_null_pointer<T>::type;
	#endif // __HAS_CPP17
	#endif // __HAS_CPP14
	#pragma endregion

	// checks if a type is an integral type
	// 检查类型是否为整数类型
	#pragma region is_integral

	#if __HAS_CPP20

	#else // __HAS_CPP <= 17

	#endif // definition of is_integral

	#if __HAS_CPP17
	template <typename T>
	inline constexpr bool is_integral_v = is_integral<T>::value;
	#endif // __HAS_CPP17

	#pragma endregion


} // namespace __MY_NAMESPACE
#endif // __HAS_CPP11
