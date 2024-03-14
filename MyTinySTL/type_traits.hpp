#pragma once
#include "core.h"

// 有关type traits的编译器内建函数, 见 https://learn.microsoft.com/en-us/cpp/extensions/compiler-support-for-type-traits-cpp-component-extensions

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


	// forward declarations
	// 前置声明，便于编程
	#pragma region forward_declarations

	template <typename T>
	struct remove_cv;

	template <typename T>
	struct add_rvalue_reference;
	template <typename T>
	typename add_rvalue_reference<T>::type declval() noexcept;

	template <typename T>
	struct is_void;
	template <typename T>
	struct is_array;
	template <typename T>
	struct is_class;
	template <typename T>
	struct is_function;

	template <typename T>
	struct is_member_pointer;

	#pragma endregion forward_declarations



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
	 * @brief 将任意类型集合 映射至 void, 结合不推导语句与SFINAE 实现对模板形参中的某些类型进行约束
	 * @tparam ... 任意的类型
	 *
	 * @note
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

	__INNER_BEGIN
	template <typename ...>
	using __void_t = void;
	__INNER_END

	#else

	__INNER_BEGIN
	template<typename... Ts>
	struct __make_void { using type = void; };

	template<typename... Ts>
	using __void_t = typename __make_void<Ts...>::type;	
	__INNER_END

	#endif // __HAS_CPP17
	#pragma endregion void_t

	// conditionally removes a function overload or template specialization from overload resolution
	// 条件性地从重载决议移除函数重载或模板特化
	#pragma region enable_if
	/**
	 * @brief conditionally removes a function overload or template specialization from overload resolution
	 * @brief 条件性地从重载决议移除函数重载或模板特化
	 * @brief 根据传入的布尔常量, 决定其是否包含type成员. 结合不推导语句与SFINAE实现对模板形参中的某些类型进行约束
	 * 
	 * @tparam Boolean 标识该结构体是否包含成员type的布尔值 
	 * @tparam T 成员type表示的类型
	 * 
	 * @note
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
	 * 另一常见用法:
	 *		template <typename...>
	 *		enable_if< CONDITION<...>, RETURN_TYPE>::type foo(...) {};
	 * 
	 *		template <typename...>
	 *		enable_if<! CONDITION<...>, RETURN_TYPE>::type foo(...) {};
	 * 
	 *		可根据模板参数是否满足 CONDITION 来调用不同的 foo 重载, 原理同上
	 * 
	 * 在C++20可使用concept进行更人性化的编程
	*/
	template <bool Boolean, typename T = void>
	struct enable_if {};

	template <typename T> 
	struct enable_if<true, T> { using type = T; };
	// specialization for Boolean = true

	#if __HAS_CPP14
	/**
	 * @brief conditionally removes a function overload or template specialization from overload resolution
	 * @brief 条件性地从重载决议移除函数重载或模板特化
	 * @brief 传入 true 时，该别名指向传入的类型 T，否则该别名非良构
	 * 
	 * @tparam Boolean 标识该别名模板是否有效的布尔值, true 则有效
	 * @tparam T 当 Boolean 为 true 时, 该别名模板所代表的类型 
	*/
	template <bool Boolean, typename T = void>
	using enable_if_t = typename enable_if<Boolean, T>::type;
	#endif // __HAS_CPP14
	#pragma endregion enable_if

	// chooses one type or another based on compile-time boolean
	// 根据传入的布尔常量, 在两个传入的类型中进行选择
	#pragma region conditional
	/**
	 * @brief chooses one type or another based on compile-time boolean
	 * @brief 根据传入的布尔常量, 在两个传入的类型中进行选择
	 * @brief 其成员别名 type 包含了类型选择的结果
	 * 
	 * @tparam Boolean 传入的布尔常量
	 * @tparam TrueType 布尔常量为 true 时的类型
	 * @tparam FalseType 布尔常量为 false 时的类型
	*/
	template <bool Boolean, typename TrueType, typename FalseType>
	struct conditional { using type = TrueType; };

	template <typename TrueType, typename FalseType>
	struct conditional<false, TrueType, FalseType> { using type = FalseType; };

	#if __HAS_CPP14
	/**
	 * @brief chooses one type or another based on compile-time boolean
	 * @brief 根据传入的布尔常量, 在两个传入的类型中进行选择
	 * @brief 自身的类型即为类型选择的结果
	 * 
	 * @tparam Boolean 传入的布尔常量
	 * @tparam TrueType 布尔常量为true时的类型
	 * @tparam FalseType 布尔常量为false时的类型
	*/
	template <bool Boolean, typename TrueType, typename FalseType>
	using conditional_t = typename conditional<Boolean, TrueType, FalseType>::type;
	#endif // __HAS_CPP14
	#pragma endregion conditional



	// ============================================
	//
	//  compile-time boolean calculation (>=C++17)
	//	编译期布尔运算 
	// 
	// ============================================

	// variadic logical AND metafunction
	// 变参的逻辑与元函数
	#pragma region conjunction
	#if __HAS_CPP17
	/**
	 * @brief variadic logical AND metafunction
	 * @brief 变参的逻辑与元函数
	 * @brief 包含成员 value, 表示合取结果
	 * 
	 * @tparam ...B 参与逻辑与运算的, 含有可显式转换为 bool 的成员 type 的, 类 类型名
	 * 
	 * @detail
	 * 特化 my::conjunction<B1, ..., BN> 有一个公开且无歧义的基类，即: 
	 * 若 sizeof...(B) == 0, 则是 my::true_type
	 * 否则, 若 B1, ..., Bn 中有 bool(Bi::value) == false, 则为首个 Bi
	 * 否则若无这种类型, 则为 Bn.
	 * 
	 * 合取是短路的：若存在模板类型参数 Bi 满足 bool(Bi::value) == false, 
	 * 则实例化 conjunction<B1, ..., Bn>::value 中不会进行对 j > i 的 Bj::value 的实例化
	*/
	template <typename... B>
	struct conjunction: true_type {};

	template <typename B1>
	struct conjunction<B1>: B1 {};

	template <typename B1, typename... Bn>
	struct conjunction<B1, Bn...>: conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};

	/**
	 * @brief variadic logical AND metafunction
	 * @brief 变参的逻辑与元函数
	 * @brief 其值即表示合取结果
	 * 
	 * @tparam ... 参与逻辑与运算的, 含有可显式转换为 bool 的成员 type 的, 类 类型名
	*/
	template <typename... B>
	inline constexpr bool conjunction_v = conjunction<B...>::value;
	#endif // __HAS_CPP17
	#pragma endregion conjunction

	// variadic logical OR metafunction
	// 变参的逻辑或元函数
	#pragma region disjuction
	#if __HAS_CPP17
	/**
	 * @brief variadic logical OR metafunction
	 * @brief 变参的逻辑或元函数
	 * @brief 包含成员 value, 表示析取结果
	 * 
	 * @tparam ...B 参与逻辑或运算的, 含有可显式转换为 bool 的成员 type 的, 类 类型名
	 * 
	 * @detail
	 * 特化 my::disjunction<B1, ..., BN> 有一个公开且无歧义的基类，即: 
	 * 若 sizeof...(B) == 0, 则是 my::false_type
	 * 否则, 若 B1, ..., Bn 中有 bool(Bi::value) == true, 则为首个 Bi
	 * 否则若无这种类型, 则为 Bn.
	 * 
	 * 析取是短路的：若存在模板类型参数 Bi 满足 bool(Bi::value) == true, 
	 * 则实例化 disjunction<B1, ..., Bn>::value 中不会进行对 j > i 的 Bj::value 的实例化
	*/
	template <typename... B>
	struct disjunction: false_type {};

	template <typename B1>
	struct disjunction<B1>: B1 {};

	template <typename B1, typename... Bn>
	struct disjunction<B1, Bn...>: conditional<bool(B1::value), B1, disjunction<Bn...>>::type {};

	/**
	 * @brief variadic logical OR metafunction
	 * @brief 变参的逻辑或元函数
	 * @brief 其值即表示析取结果
	 * 
	 * @tparam ... 参与逻辑或运算的, 含有可显式转换为 bool 的成员 type 的, 类 类型名
	*/
	template <typename... B>
	inline constexpr bool disjunction_v = disjunction<B...>::value;
	#endif // __HAS_CPP17
	#pragma endregion disjunction

	// logical NOT metafunction
	// 逻辑非元函数
	#pragma region negation
	#if __HAS_CPP17
	/**
	 * @brief logical NOT metafunction
	 * @brief 逻辑非元函数
	 * @brief 包含成员 value, 表示取反结果
	 * 
	 * @tparam B 参与逻辑非运算的, 含有可显式转换为 bool 的成员 type 的, 类 类型名
	*/
	template <typename B>
	struct negation: bool_constant<!bool(B::value)> {};

	/**
	 * @brief logical NOT metafunction
	 * @brief 逻辑非元函数
	 * @brief 其值即表示取反结果
	 * 
	 * @tparam B 参与逻辑非运算的, 含有可显式转换为 bool 的成员 type 的, 类 类型名
	*/
	template <typename B>
	inline constexpr bool negation_v = negation<B>::value;
	#endif // __HAS_CPP17
	#pragma endregion negation

	

	// ===============================================
	//  
	//	type relationships
	//  类型关系
	//
	// ===============================================

	// checks if two types are the same
	// 检查两个类型是否相同, 其值即表示其结果
	#pragma region is_same
	/**
	 * @brief checks if two types are the same
	 * @brief 检查两个类型是否相同, 包含成员value表示其结果值 
	*/
	template <typename, typename>
	struct is_same: false_type {};

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
	#pragma endregion is_same

	// (non-standard feature) checks if one type equals to any other types
	// (非标准内容) 判断一个类型是否与后续类型之一相等
	#pragma region is_any_of
	#if __HAS_CPP17
	/**
	 * @brief (non-standard feature) checks if one type equals to any other types
	 * @brief (非标准内容) 判断一个类型是否包含于后续类型之中
	 * @brief 包含成员 value, 表示判断结果
	 * 
	 * @tparam T 一个类型
	 * @tparam ...Ts 被判断是否包含T的类型集合
	*/
	template <typename T, typename... Ts>
	struct is_any_of: bool_constant<(is_same_v<T, Ts> || ...)> {};

	/**
	 * @brief (non-standard feature) checks if one type equals to any other types
	 * @brief (非标准内容) 判断一个类型是否包含于后续类型之中
	 * @brief 其自身即表示判断结果
	 * 
	 * @tparam T 一个类型
	 * @tparam ...Ts 被判断是否包含T的类型集合
	*/
	template <typename T, typename... Ts>
	inline constexpr bool is_any_of_v = is_any_of_v<T, Ts...>;

	#else // ^^^__HAS_CPP17 / vvv !__HAS_CPP17
	/**
	 * @brief (non-standard feature) checks if one type equals to any other types
	 * @brief (非标准内容) 判断一个类型是否包含于后续类型之中
	 * @brief 包含成员 value, 表示判断结果
	 * 
	 * @tparam T 一个类型
	 * @tparam ...Ts 被判断是否包含T的类型集合
	*/
	template <typename T, typename...>
	struct is_any_of: integral_constant<bool, false> {};

	template <typename T1, typename T2>
	struct is_any_of<T1, T2>: is_same<T1, T2> {};

	template <typename T1, typename T2, typename... Ts>
	struct is_any_of<T1, T2, Ts...>: integral_constant<bool, is_same<T1, T2>::value || is_any_of<T1, Ts...>::value> {};
	#endif // __HAS_CPP17
	#pragma endregion is_any_of

	// checks if a type is derived from the other type
	// 检查一个类型是否派生自另一个类型
	#pragma region is_base_of

	// 解法来自 https://zh.cppreference.com/w/cpp/types/is_base_of

	__INNER_BEGIN
	template <typename Base>
	true_type __test_convert_ptr_to_base(const volatile Base*) {};
	template <typename>
	false_type __test_convert_ptr_to_base(const volatile void*) {};

	template <typename Base, typename Derived>
	auto __test_is_base_of(int) -> decltype(__test_convert_ptr_to_base<Base>(static_cast<Derived*>(nullptr))) {};
	template <typename, typename>
	auto __test_is_base_of(...) -> true_type {}; // 处理私有、受保护或有歧义的基类
	__INNER_END

	/**
	 * @brief checks if a type is derived from the other type
	 * @brief 检查一个类型是否派生自另一个类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam Base 需要进行判断的基类类型
	 * @tparam Derived 需要进行判断的派生类类型
	*/
	template <typename Base, typename Derived>
	struct is_base_of: integral_constant<
		bool,
		is_class<Base>::value &&
		is_class<Derived>::value &&
		decltype(__INNER_NAMESPACE::__test_is_base_of<Base, Derived>(0))::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is derived from the other type
	 * @brief 检查一个类型是否派生自另一个类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam Base 需要进行判断的基类类型
	 * @tparam Derived 需要进行判断的派生类类型
	*/
	template <typename T>
	inline constexpr bool is_base_of_v = is_base_of<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_base_of

	// checks if a type can be converted to the other type
	// 检查是否能转换一个类型为另一类型
	#pragma region is_convertible

	// 要求虚构函数 To test() { return std::declval<From>(); } 良构
	// https://zh.cppreference.com/w/cpp/types/is_convertible

	__INNER_BEGIN
	template <typename To>
	To __try_implicitly_convert_to(To) {};
	// 用 To 作为返回值类型，保证 To 能够作为返回值类型（即 !is_array 及 !is_function）

	template <typename From, typename To, 
		typename = decltype(__try_implicitly_convert_to<To>(declval<From>()))
	>
	true_type __is_implicitly_convertible(int) {};

	template <typename, typename>
	false_type __is_implicitly_convertible(...) {};
	__INNER_END

	/**
	 * @brief checks if a type can be implicitly converted to the other type
	 * @brief 检查是否能隐式转换一个类型为另一类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam From 判断可否隐式转换的起始类型
	 * @tparam To 判断可否隐式转换的目标类型
	*/
	template <typename From, typename To>
	struct is_convertible: integral_constant<
		bool,
		// 保证 std::declval<From>() 能隐式转换为 To 
		decltype(__INNER_NAMESPACE::__is_implicitly_convertible<From, To>(0))::value ||
		// 或 From 和 To 均为可有 cv 限定的 void
		is_void<From>::value && is_void<To>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type can be implicitly converted to the other type
	 * @brief 检查是否能隐式转换一个类型为另一类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam From 判断可否隐式转换的起始类型
	 * @tparam To 判断可否隐式转换的目标类型
	*/
	template <typename From, typename To>
	inline constexpr bool is_convertible_v = is_convertible<From, To>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_convertible



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
	/**
	 * @brief returns the type argument unchanged
	 * @brief 恒等类型变换, 常用于构造不推导语境
	 * @brief 包含成员别名 type, 与输入的T相同
	 * 
	 * @tparam T 参与变换的类型
	*/
	template <typename T>
	struct type_identity { using type = T; };
	#endif // __HAS_CPP20
	#pragma endregion type_identity



	// ====================================================
	// 
	//	cv specifiers add and remove
	//  cv 限定符(const 与 volatile) 的引入与移除
	//
	// ====================================================

	// removes const specifiers from the given type
	// 从给定类型移除 const 限定符
	#pragma region remove_const
	/**
	 * @brief removes const specifiers from the given type
	 * @brief 从给定类型移除 const 限定符
	 * @brief 包含成员别名 type, 表示移除的结果
	 * 
	 * @tparam T 需要移除 const 限定符的类型
	*/
	template <typename T>
	struct remove_const { using type = T; };

	template <typename T>
	struct remove_const<const T> { using type = T; };

	#if __HAS_CPP14
	/**
	 * @brief removes const specifiers from the given type
	 * @brief 从给定类型移除 const 限定符
	 * @brief 该别名本身即表示移除的结果
	 * 
	 * @tparam T 需要移除 const 限定符的类型
	*/
	template <typename T>
	using remove_const_t = typename remove_const<T>::type;
	#endif // __HAS_CPP14
	#pragma endregion remove_const

	// removes voatile specifiers from the given type
	// 从给定类型移除 volatile 限定符
	#pragma region remove_volatile
	/**
	 * @brief removes volatile specifiers from the given type
	 * @brief 从给定类型移除 volatile 限定符
	 * @brief 包含成员别名 type, 表示移除的结果
	 * 
	 * @tparam T 需要移除 volatile 限定符的类型
	*/
	template <typename T>
	struct remove_volatile { using type = T; };

	template <typename T>
	struct remove_volatile<volatile T> { using type = T; };

	#if __HAS_CPP14
	/**
	 * @brief removes volatile specifiers from the given type
	 * @brief 从给定类型移除 volatile 限定符
	 * @brief 该别名本身即表示移除的结果
	 * 
	 * @tparam T 需要移除 volatile 限定符的类型
	*/
	template <typename T>
	using remove_volatile_t = typename remove_volatile<T>::type;
	#endif // __HAS_CPP14
	#pragma endregion remove_volatile

	// removes const and volatile specifiers from the given type
	// 从给定类型移除 const 和 volatile 限定符
	#pragma region remove_cv
	/**
	 * @brief removes const and volatile specifiers from the given type
	 * @brief 从给定类型移除 const 和 volatile 限定符
	 * @brief 包含成员别名 type, 表示移除的结果
	 * 
	 * @tparam T 需要移除 const 和 volatile 限定符的类型
	*/
	template <typename T>
	struct remove_cv { using type = T; };

	template <typename T>
	struct remove_cv<const T> { using type = T; };

	template <typename T>
	struct remove_cv<volatile T> { using type = T; };

	template <typename T>
	struct remove_cv<const volatile T> { using type = T; };

	#if __HAS_CPP14
	/**
	 * @brief removes const and volatile specifiers from the given type
	 * @brief 从给定类型移除 const 和 volatile 限定符
	 * @brief 该别名本身即表示移除的结果
	 * 
	 * @tparam T 需要移除 const 和 volatile 限定符的类型
	*/
	template <typename T>
	using remove_cv_t = typename remove_cv<T>::type;
	#endif // __HAS_CPP14
	#pragma endregion remove_cv

	// add const specifiers from the given type
	// 添加 const 限定符到给定类型
	#pragma region add_const
	/**
	 * @brief add const specifiers from the given type
	 * @brief 添加 const 限定符到给定类型
	 * @brief 包含成员别名 type, 表示添加的结果
	 * 
	 * @tparam T 需要添加 const 限定符的类型
	*/
	template <typename T>
	struct add_const { using type = const T; };

	#if __HAS_CPP14
	/**
	 * @brief add const specifiers from the given type
	 * @brief 添加 const 限定符到给定类型
	 * @brief 该别名本身即表示添加的结果
	 * 
	 * @tparam T 需要添加 const 限定符的类型
	*/
	template <typename T>
	using add_const_t = typename add_const<T>::type;
	#endif // __HAS_CPP14
	#pragma endregion add_const	

	// add volatile specifiers from the given type
	// 添加 volatile 限定符到给定类型
	#pragma region add_volatile
	/**
	 * @brief add volatile specifiers from the given type
	 * @brief 添加 volatile 限定符到给定类型
	 * @brief 包含成员别名 type, 表示添加的结果
	 * 
	 * @tparam T 需要添加 volatile 限定符的类型
	*/
	template <typename T>
	struct add_volatile { using type = volatile T; };

	#if __HAS_CPP14
	/**
	 * @brief add volatile specifiers from the given type
	 * @brief 添加 volatile 限定符到给定类型
	 * @brief 该别名本身即表示添加的结果
	 * 
	 * @tparam T 需要添加 volatile 限定符的类型
	*/
	template <typename T>
	using add_volatile_t = typename add_volatile<T>::type;
	#endif // __HAS_CPP14
	#pragma endregion add_volatile

	// add const and volatile specifiers from the given type
	// 添加 const 和 volatile 限定符到给定类型
	#pragma region add_cv
	/**
	 * @brief add const and volatile specifiers from the given type
	 * @brief 添加 const 和 volatile 限定符到给定类型
	 * @brief 包含成员别名 type, 表示添加的结果
	 * 
	 * @tparam T 需要添加 const 和 volatile 限定符的类型
	*/
	template <typename T>
	struct add_cv { using type = const volatile T; };

	#if __HAS_CPP14
	/**
	 * @brief add const and volatile specifiers from the given type
	 * @brief 添加 const 和 volatile 限定符到给定类型
	 * @brief 该别名本身即表示添加的结果
	 * 
	 * @tparam T 需要添加 const 和 volatile 限定符的类型
	*/
	template <typename T>
	using add_cv_t = typename add_cv<T>::type;
	#endif // __HAS_CPP14
	#pragma endregion add_cv



	// ====================================================
	// 
	//	reference add and remove
	//  引用的引入与移除
	//
	// ====================================================

	// removes a reference from the given type
	// 移除类型中的引用
	#pragma region remove_reference
	/**
	 * @brief removes a reference from the given type
	 * @brief 移除类型中的引用
	 * @brief 包含成员别名 type, 表示移除的结果
	 * 
	 * @tparam T 需要移除引用的类型
	*/
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
	/**
	 * @brief removes a reference from the given type
	 * @brief 移除类型中的引用
	 * @brief 该别名即表示移除的结果
	 * 
	 * @tparam T 需要移除引用的类型
	*/
	template <typename T>
	using remove_reference_t = typename remove_reference<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion remove_reference

	// adds an lvalue and rvalue reference to the given type
	// 为类型引入左值引用及右值引用, 遵循引用折叠原则
	#pragma region add_lvalue_reference add_rvalue_reference

	__INNER_BEGIN
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

	#else // ^^^ __HAS_CPP20 / vvv !__HAS_CPP20
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
	__INNER_END

	/**
	 * @brief adds an lvalue reference to the given type
	 * @brief 为类型引入左值引用, 遵循引用折叠原则
	 * @brief 包含成员别名 type, 表示添加的结果
	 * 
	 * @tparam T 需要添加左值引用的类型
	*/
	template <typename T>
	struct add_lvalue_reference { using type = typename __INNER_NAMESPACE::__try_add_reference<T>::lvalue_type; };

	/**
	 * @brief adds an rvalue reference to the given type
	 * @brief 为类型引入右值引用, 遵循引用折叠原则
	 * @brief 包含成员别名 type, 表示添加的结果
	 * 
	 * @tparam T 需要添加右值引用的类型
	*/
	template <typename T>
	struct add_rvalue_reference { using type = typename __INNER_NAMESPACE::__try_add_reference<T>::rvalue_type; };

	#if __HAS_CPP14
	/**
	 * @brief adds an lvalue reference to the given type
	 * @brief 为类型引入左值引用, 遵循引用折叠原则
	 * @brief 该别名即表示添加的结果
	 * 
	 * @tparam T 需要添加左值引用的类型
	*/
	template <typename T>
	using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

	/**
	 * @brief adds an rvalue reference to the given type
	 * @brief 为类型引入右值引用, 遵循引用折叠原则
	 * @brief 该别名即表示添加的结果
	 * 
	 * @tparam T 需要添加右值引用的类型
	*/
	template <typename T>
	using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;
	#endif // __HAS_CPP14

	#pragma endregion add_lvalue_reference add_rvalue_reference

	// obtains a reference to its argument for use in unevaluated context
	// 获取到其实参的引用，用于不求值语境中
	#pragma region declval
	/**
	 * @brief obtains a reference to its argument for use in unevaluated context
	 * @brief 获取到其实参的引用，用于不求值语境中
	 * 
	 * @tparam T 用于转换的类型T
	 * @return 不能被调用，无返回值。返回类型为 T&&，除非 T = void
	*/
	template <typename T>
	typename add_rvalue_reference<T>::type declval() noexcept {
		// static_assert(false, "ODR 使用 declval 的程序非良构");
		// https://zh.cppreference.com/w/cpp/utility/declval
		// https://zh.cppreference.com/w/cpp/language/definition#ODR_.E4.BD.BF.E7.94.A8
	}
	#pragma endregion declval



	// ====================================================
	// 
	//	pointer add and remove
	//  指针的引入与移除
	//
	// ====================================================

	// removes a pointer from the given type
	// 移除给定类型的一层指针
	#pragma region remove_pointer
	/**
	 * @brief removes a pointer from the given type
	 * @brief 移除给定类型的一层指针
	 * @brief 包含成员别名 type, 表示移除的结果
	 * 
	 * @tparam T 需要移除指针的类型
	*/
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
	/**
	 * @brief removes a pointer from the given type
	 * @brief 移除给定类型的一层指针
	 * @brief 别名即表示移除的结果
	 * 
	 * @tparam T 需要移除指针的类型
	*/
	template <typename T>
	using remove_pointer_t = typename remove_pointer<T>::type;
	#endif // __HAS_CPP14
	#pragma endregion remove_pointer

	// adds a pointer to the given type
	// 为类型引入指针, 对于不可引用的类型(除去void及其cv衍生物), 返回其本身
	#pragma region add_pointer

	#if __HAS_CPP20
	// handle non-reference-able type except void
	// 处理不可引用的类型, 除了void

	/**
	 * @brief adds a pointer to the given type
	 * @brief 对给定类型添加一层指针
	 * @brief 包含成员别名 type, 表示添加的结果
	 * 
	 * @tparam T 需要添加指针的类型
	*/
	template <typename T>
	struct add_pointer { using type = T; };

	// handle reference-able type and void
	// 处理可引用的类型, 以及void
	template <typename T> 
		requires requires {
		typename type_identity<typename remove_reference<T>::type*>;
	}
	struct add_pointer<T> { using type = typename remove_reference<T>::type*; };

	#else // ^^^ __HAS_CPP20 / vvv !__HAS_CPP20

	__INNER_BEGIN
	template <typename T, typename = void>
	struct __try_add_pointer {
		using pointer_type = T;
	};

	template <typename T>
	struct __try_add_pointer<T, typename __void_t<typename remove_reference<T>::type*>> {
		using pointer_type = typename remove_reference<T>::type*;
	};
	__INNER_END

	/**
	 * @brief adds a pointer to the given type
	 * @brief 对给定类型添加一层指针
	 * @brief 包含成员别名 type, 表示添加的结果
	 * 
	 * @tparam T 需要添加指针的类型
	*/
	template <typename T>
	struct add_pointer { using type = typename __INNER_NAMESPACE::__try_add_pointer<T>::pointer_type; };
	#endif // definition of add_pointer

	#if __HAS_CPP14
	/**
	 * @brief adds a pointer to the given type
	 * @brief 对给定类型添加一层指针
	 * @brief 该别名即表示添加的结果
	 * 
	 * @tparam T 需要添加指针的类型
	*/
	template <typename T>
	using add_pointer_t = typename add_pointer<T>::type;
	#endif // __HAS_CPP14
	#pragma endregion add_pointer



	// ========================================
	// 
	//  reduction in the dimension of an array
	//  数组维数的降低
	//
	// ========================================

	// removes one extent from the given array type
	// 从给定数组类型移除一个维度
	#pragma region remove_extent
	/**
	 * @brief removes one extent from the given array type
	 * @brief 从给定数组类型移除一个维度
	 * @brief 包含成员别名 type, 表示移除的结果
	 * 
	 * @tparam T 需要移除维度的类型
	*/
	template <typename T>
	struct remove_extent { using type = T; };

	template <typename T>
	struct remove_extent<T[]> { using type = T; };

	template <typename T, ::size_t N>
	struct remove_extent<T[N]> { using type = T; };

	#if __HAS_CPP14
	/**
	 * @brief removes one extent from the given array type
	 * @brief 从给定数组类型移除一个维度
	 * @brief 该别名即表示移除的结果
	 * 
	 * @tparam T 需要移除内容的类型
	*/
	template <typename T>
	using remove_extent_t = typename remove_extent<T>::type;
	#endif // __HAS_CPP14
	#pragma endregion remove_extent

	// removes all extents from the given array type
	// 移除给定数组类型的所有维度
	#pragma region remove_all_extents
	/**
	 * @brief removes all extents from the given array type
	 * @brief 移除给定数组类型的所有维度
	 * @brief 包含成员别名 type, 表示移除的结果
	 * 
	 * @tparam T 需要移除所有维度的类型
	*/
	template <typename T>
	struct remove_all_extents { using type = T; };

	template <typename T>
	struct remove_all_extents<T[]> { using type = typename remove_all_extents<T>::type; };

	template <typename T, ::size_t N>
	struct remove_all_extents<T[N]> { using type = typename remove_all_extents<T>::type; };

	#if __HAS_CPP14
	/**
	 * @brief removes all extents from the given array type
	 * @brief 移除给定数组类型的所有维度
	 * @brief 该别名即表示移除的结果
	 * 
	 * @tparam T 需要移除所有维度的类型
	*/
	template <typename T>
	using remove_all_extents_t = typename remove_all_extents<T>::type;
	#endif // __HAS_CPP14
	#pragma endregion remove_all_extents



	// ============================================
	//
	//  Type properties and judges in compile-time
	//  类型的性质及其分类的编译期内判断
	//
	// ============================================

	// obtains the type's alignment requirements
	// 获取类型的对齐要求
	#pragma region alignment_of
	/**
	 * @brief obtains the type's alignment requirements
	 * @brief 获取类型的对齐要求
	 * @brief 包含成员 value，表示对齐要求的值
	 *
	 * @tparam T 需要获取对齐要求的类型
	*/
	template <typename T>
	struct alignment_of: integral_constant<::size_t, alignof(T)> {};

	#if __HAS_CPP17
	/**
	 * @brief obtains the type's alignment requirements
	 * @brief 获取类型的对齐要求
	 * @brief 该常量即表示对齐要求的值
	 *
	 * @tparam T 需要获取对齐要求的类型
	*/
	template <typename T>
	inline constexpr ::size_t alignment_of_v = alignof(T);
	#endif // __HAS_CPP17
	#pragma endregion alignment_of

	// obtains the number of dimensions of an array type
	// 获取数组类型的维数
	#pragma region rank
	/**
	 * @brief obtains the number of dimensions of an array type
	 * @brief 获取数组类型的维数
	 * @brief 包含成员 value，表示其维数的值
	 * 
	 * @tparam T 需要获取维数的类型
	*/
	template <typename T>
	struct rank: integral_constant<::size_t, 0ULL> {};

	template <typename T>
	struct rank<T[]>: integral_constant<::size_t, 1ULL + rank<T>::value> {};

	template <typename T, ::size_t N>
	struct rank<T[N]>: integral_constant<::size_t, 1ULL + rank<T>::value> {};

	#if __HAS_CPP17
	/**
	 * @brief obtains the number of dimensions of an array type
	 * @brief 获取数组类型的维数
	 * @brief 该常量即表示对齐要求的值
	 * 
	 * @tparam T 需要获取维数的类型
	*/
	template <typename T>
	inline constexpr ::size_t rank_v = rank<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion rank

	// obtains the size of an array type along a specified dimension
	// 获取数组类型在指定维度的大小
	#pragma region extent
	/**
	 * @brief obtains the size of an array type along a specified dimension
	 * @brief 获取数组类型在指定维度的大小
	 * @brief 包含成员 value，表示其维度的大小
	 * 
	 * @tparam T 需要获取维度大小的类型
	 * @tparam Dm 指定的维度
	*/
	template <typename T, unsigned Dm = 0>
	struct extent: integral_constant<::size_t, 0ULL> {};

	template <typename T, unsigned Dm>
	struct extent<T[], Dm>: extent<T, Dm - 1U> {};
	template <typename T>
	struct extent<T[], 0U>: integral_constant<::size_t, 0LL> {};

	template <typename T, ::size_t N, unsigned Dm>
	struct extent<T[N], Dm>: extent<T, Dm - 1U> {};
	template <typename T, ::size_t N>
	struct extent<T[N], 0U>: integral_constant<::size_t, N> {};

	#if __HAS_CPP17
	/**
	 * @brief obtains the size of an array type along a specified dimension
	 * @brief 获取数组类型在指定维度的大小
	 * @brief 该常量即表示其维度的大小
	 * 
	 * @tparam T 需要获取维度大小的类型
	 * @tparam Dm 指定的维度
	*/
	template <typename T, unsigned Dm = 0>
	inline constexpr ::size_t extent_v = extent<T, Dm>::value;
	#endif // __HAS_CPP17
	#pragma endregion extent

	// checks if a type is const-qualified
	// 检查类型是否为 const 限定
	#pragma region is_const
	/**
	 * @brief checks if a type is const-qualified
	 * @brief 检查类型是否为 const 限定
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_const: false_type {};

	template <typename T>
	struct is_const<const T>: true_type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is const-qualified
	 * @brief 检查类型是否为 const 限定
	 * @brief 该常量即为判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_const_v = is_const<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_const

	// checks if a type is volatile-qualified
	// 检查类型是否为 volatile 限定
	#pragma region is_volatile
	/**
	 * @brief checks if a type is volatile-qualified
	 * @brief 检查类型是否为 volatile 限定
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_volatile: false_type {};

	template <typename T>
	struct is_volatile<volatile T>: true_type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is volatile-qualified
	 * @brief 检查类型是否为 volatile 限定
	 * @brief 该常量即为判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_volatile_v = is_volatile<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_volatile



	// ====================================================
	// 
	//	Primary type categories and judges in compile-time
	//  原始类型及其分类的编译期内判断
	//
	// ====================================================

	#pragma region primary series

	// checks if a type is void
	// 检查类型是否为 void 及其cv衍生物
	#pragma region is_void
	/**
	 * @brief checks if a type is void
	 * @brief 检查类型是否为 void 及其cv衍生物
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_void: is_same<void, typename remove_cv<T>::type> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is void
	 * @brief 检查类型是否为 void 及其cv衍生物
	 * @brief 该常量即为判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_void_v = is_void<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_void

	// checks if a type is std::nullptr_t
	// 检查类型是否为 std::nullptr_t 及其cv衍生物
	#pragma region is_null_pointer (>=C++14)
	#if __HAS_CPP14
	/**
	 * @brief checks if a type is void
	 * @brief 检查类型是否为 std::nullptr_t 及其cv衍生物
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_null_pointer: is_same<typename remove_cv<T>::type, ::std::nullptr_t> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is void
	 * @brief 检查类型是否为 std::nullptr_t 及其cv衍生物
	 * @brief 该常量即为判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_null_pointer_v = is_null_pointer<T>::type;
	#endif // __HAS_CPP17
	#endif // __HAS_CPP14
	#pragma endregion is_null_pointer

	// checks if a type is an integral type
	// 检查类型是否为整数类型
	#pragma region is_integral
	/**
	 * @brief checks if a type is an integral type
	 * @brief 检查类型是否为整数类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_integral: is_any_of<typename remove_cv<T>::type, bool, char, unsigned char, signed char, 
		#if __HAS_CPP20
		char8_t,
		#endif // __HAS_CPP20
		char16_t, char32_t, wchar_t, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>
	{};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is an integral type
	 * @brief 检查类型是否为整数类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_integral_v = is_integral<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_integral

	// checks if a type is a floating-point type
	// 检查类型是否是浮点类型
	#pragma region is_floating_point
	/**
	 * @brief checks if a type is a floating-point type
	 * @brief 检查类型是否是浮点类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_floating_point: is_any_of<typename remove_cv<T>::type, float, double, long double> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a floating-point type
	 * @brief 检查类型是否是浮点类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_floating_point_v = is_floating_point<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_floating_point

	// checks if a type is an array type
	// 检查类型是否是数组类型
	#pragma region is_array
	/**
	 * @brief checks if a type is a array type
	 * @brief 检查类型是否是数组类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_array: false_type {};

	template <typename T>
	struct is_array<T[]>: true_type {};

	template <typename T, ::size_t N>
	struct is_array<T[N]>: true_type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a array type
	 * @brief 检查类型是否是数组类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_array_v = is_array<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_array

	// checks if a type is a union type
	// 检查类型是否为联合体类型
	#pragma region is_union
	/**
	 * @brief checks if a type is a union type
	 * @brief 检查类型是否为联合体类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_union: integral_constant<bool, __is_union(T)> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a union type
	 * @brief 检查类型是否为联合体类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_union_v = is_union<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_union

	// checks if a type is a non-union class type
	// 检查类型是否为非联合类类型
	#pragma region is_class

	__INNER_BEGIN
	template <typename T, typename = void>
	struct __is_class_or_union: false_type {};
	template <typename T>
	struct __is_class_or_union<T, __void_t<int T::*>>: true_type {};
	__INNER_END

	/**
	 * @brief checks if a type is a non-union class type
	 * @brief 检查类型是否为非联合类类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_class: integral_constant<
		bool, 
		__INNER_NAMESPACE::__is_class_or_union<T>::value && 
		!is_union<T>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a non-union class type
	 * @brief 检查类型是否为非联合类类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_class_v = is_class<T>::value;
	#endif
	#pragma endregion is_class

	// checks if a type is a pointer type
	// 检查类型是否为指针类型
	#pragma region is_pointer

	__INNER_BEGIN
	template <typename T>
	struct __is_raw_pointer: false_type {};
	template <typename T>
	struct __is_raw_pointer<T*>: true_type {};
	__INNER_END

	/**
	 * @brief checks if a type is a pointer type
	 * @brief 检查类型是否为指针类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_pointer: __INNER_NAMESPACE::__is_raw_pointer<typename remove_cv<T>::type> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a pointer type
	 * @brief 检查类型是否为指针类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_pointer_v = is_pointer<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_pointer

	// checks if a type is an lvalue reference
	// 检查类型是否为左值引用
	#pragma region is_lvalue_reference
	/**
	 * @brief checks if a type is an lvalue reference
	 * @brief 检查类型是否为左值引用
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_lvalue_reference: false_type {};
	template <typename T>
	struct is_lvalue_reference<T&>: true_type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is an lvalue reference
	 * @brief 检查类型是否为左值引用
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_lvalue_reference

	// checks if a type is an rvalue reference
	// 检查类型是否为右值引用
	#pragma region is_rvalue_reference
	/**
	 * @brief checks if a type is an rvalue reference
	 * @brief 检查类型是否为右值引用
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_rvalue_reference: false_type {};
	template <typename T>
	struct is_rvalue_reference<T&&>: true_type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is an rvalue reference
	 * @brief 检查类型是否为右值引用
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_rvalue_reference

	// checks if a type is a function type
	// 检查是否为函数类型
	#pragma region is_function
	/**
	 * @brief checks if a type is a function type
	 * @brief 检查类型是否为函数类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	 * 
	 * @note
	 *		...函数类型和引用类型以外的任何（可能不完整的）类型都是包含以下...
	 *		见 https://zh.cppreference.com/w/cpp/language/cv
	*/
	template <typename T>
	struct is_function: integral_constant<bool,
		!is_const<const T>::value &&
		!is_lvalue_reference<T>::value &&
		!is_rvalue_reference<T>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a function type
	 * @brief 检查类型是否为函数类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_function_v = is_function<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_function

	// checks if a type is a pointer to a non-static member function
	// 检查类型是否为指向非静态成员函数的指针
	#pragma region is_member_function_pointer

	__INNER_BEGIN
	template <typename T>
	struct __is_member_function_pointer: false_type {};
	template <typename Func, class C>
	struct __is_member_function_pointer<Func C::*>: is_function<Func> {};
	__INNER_END

	/**
	 * @brief checks if a type is a pointer to a non-static member function
	 * @brief 检查类型是否为指向非静态成员函数的指针
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_member_function_pointer: __INNER_NAMESPACE::__is_member_function_pointer<typename remove_cv<T>::type> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a pointer to a non-static member function
	 * @brief 检查类型是否为指向非静态成员函数的指针
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_member_function_pointer_v = is_member_function_pointer<T>::type;
	#endif // __HAS_CPP17

	#pragma endregion is_member_function_pointer

	// checks if a type is a pointer to a non-static member object
	// 检查类型是否为指向非静态成员对象的指针
	#pragma region is_member_object_pointer
	/**
	 * @brief checks if a type is a pointer to a non-static member object
	 * @brief 检查类型是否为指向非静态成员对象的指针
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_member_object_pointer: integral_constant<
		bool,
		is_member_pointer<T>::value &&
		!is_member_function_pointer<T>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a pointer to a non-static member object
	 * @brief 检查类型是否为指向非静态成员对象的指针
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_member_object_pointer_v = is_member_object_pointer<T>::type;
	#endif // __HAS_CPP17
	#pragma endregion is_member_object_pointer

	// checks if a type is an enumeration type
	// 检查类型是否是枚举类型
	#pragma region is_enum

	/**
	 * @brief checks if a type is an enumeration type
	 * @brief 检查类型是否是枚举类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	 * 
	 * @note https://zh.cppreference.com/w/cpp/language/type
	*/
	template <typename T>
	struct is_enum: integral_constant<
		bool,
		!is_integral<T>::value &&
		!is_floating_point<T>::value &&
		!is_null_pointer<T>::value &&
		!is_void<T>::value &&
		!is_array<T>::value &&
		!__INNER_NAMESPACE::__is_class_or_union<T>::value &&
		!is_member_pointer<T>::value &&
		!is_pointer<T>::value &&
		!is_function<T>::value &&
		!is_lvalue_reference<T>::value &&
		!is_rvalue_reference<T>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is an enumeration type
	 * @brief 检查类型是否是枚举类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_enum_v = is_enum<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_enum

	#pragma endregion primary series



	// =========================================================================
	//
	//  Composite type categories and judges in compile-time
	//  复合类型分类的编译期内判断
	//  类型框图 https://upload.cppreference.com/mwiki/images/9/96/cpp_types.svg
	//
	// =========================================================================

	#pragma region composite series

	// checks if a type is an arithmetic type
	// 检查类型是否为算术类型
	#pragma region is_arithmetic
	/**
	 * @brief checks if a type is an arithmetic type
	 * @brief 检查类型是否为算术类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_arithmetic: integral_constant<
		bool,
		is_integral<T>::value ||
		is_floating_point<T>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is an arithmetic type
	 * @brief 检查类型是否为算术类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_arithmetic_v = is_arithmetic<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_arithmetic

	// checks if a type is a fundamental type
	// 检查是否是基础类型
	#pragma region is_fundamental
	/**
	 * @brief checks if a type is a fundamental type
	 * @brief 检查类型是否为基础类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_fundamental: integral_constant<
		bool,
		is_arithmetic<T>::value ||
		is_same<typename remove_cv<T>::type, ::std::nullptr_t>::value ||
		is_void<T>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a fundamental type
	 * @brief 检查类型是否为基础类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_fundamental_v = is_fundamental<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_fundamental
	
	// checks if a type is a pointer to a non-static member function or object
	// 检查类型是否为指向非静态成员函数或对象的指针类型
	#pragma region is_member_pointer

	__INNER_BEGIN
	template <typename T>
	struct __is_member_pointer: false_type {};
	template <typename T, class C>
	struct __is_member_pointer<T C::*>: true_type {};
	__INNER_END

	/**
	 * @brief checks if a type is a pointer to a non-static member function or object
	 * @brief 检查类型是否为指向非静态成员函数或对象的指针类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_member_pointer: __INNER_NAMESPACE::__is_member_pointer<typename remove_cv<T>::type> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a pointer to a non-static member function or object
	 * @brief 检查类型是否为指向非静态成员函数或对象的指针类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_member_pointer_v = is_member_pointer<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_member_pointer

	// checks if a type is a scalar type
	// 检查类型是否为标量类型
	#pragma region is_scalar
	/**
	 * @brief checks if a type is a scalar type
	 * @brief 检查类型是否为标量类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_scalar: integral_constant<
		bool, 
		is_arithmetic<T>::value ||
		is_same<typename remove_cv<T>::type, ::std::nullptr_t>::value ||
		is_enum<T>::value ||
		is_member_pointer<T>::value ||
		is_pointer<T>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a scalar type
	 * @brief 检查类型是否为标量类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_scalar_v = is_scalar<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_scalar

	// checks if a type is an object type
	// 检查是否是对象类型
	#pragma region is_object
	/**
	 * @brief checks if a type is an object type
	 * @brief 检查类型是否为对象类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_object: integral_constant<
		bool, 
		is_scalar<T>::value ||
		is_array<T>::value ||
		__INNER_NAMESPACE::__is_class_or_union<T>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is an object type
	 * @brief 检查类型是否为对象类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_object_v = is_object<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_object

	// checks if a type is a compound type
	// 检查是否为复合类型
	#pragma region is_compound
	/**
	 * @brief checks if a type is a compound type
	 * @brief 检查是否为复合类型
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_compound: integral_constant<bool, !is_fundamental<T>::value> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is a compound type
	 * @brief 检查是否为复合类型
	 * @brief 包含成员 value, 表示其判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_compound_v = is_compound<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_compound

	// checks if a type is either an lvalue reference or rvalue reference
	// 检查类型是否为左值引用或右值引用
	#pragma region is_reference
	/**
	 * @brief checks if a type is either an lvalue reference or rvalue reference
	 * @brief 检查类型是否为左值引用或右值引用
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	struct is_reference: integral_constant<
		bool, 
		is_lvalue_reference<T>::value ||
		is_rvalue_reference<T>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type is either an lvalue reference or rvalue reference
	 * @brief 检查类型是否为左值引用或右值引用
	 * @brief 其本身即表示判断结果
	 * 
	 * @tparam T 需要进行判断的类型
	*/
	template <typename T>
	inline constexpr bool is_reference_v = is_reference<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_reference

	#pragma endregion composite series



	// ===============================
	// 
	//  Miscellaneous transformations
	//  杂项类型变换
	//
	// ===============================

	// applies type transformations as when passing a function argument by value
	// 实施当按值传递实参给函数时所进行的类型变换
	#pragma region decay
	/**
	 * @brief applies type transformations as when passing a function argument by value
	 * @brief 实施当按值传递实参给函数时所进行的类型变换
	 * @brief 包含成员别名 type, 表示变换的结果
	 * 
	 * @tparam T 需要进行变换的类型
	*/
	template <typename T>
	struct decay {
	private:
		using _rm_ref_t = typename remove_reference<T>::type;
	public:
		using type = typename conditional<
			// 如果 T 是“U 的数组”或“到 U 的数组的引用”类型
			is_array<_rm_ref_t>::value,
			// 那么成员 typedef type 是 U*
			typename remove_extent<_rm_ref_t>::type*,
			// 否则
			typename conditional<
				// 如果 T 是函数类型 F 或到它的引用
				is_function<_rm_ref_t>::value,
				// 那么成员 typedef type 是add_pointer<F>::type
				typename add_pointer<_rm_ref_t>::type,
				// 否则，成员 typedef type 是 remove_cv<remove_reference<T>::type>::type
				typename remove_cv<_rm_ref_t>::type
			>::type
		>::type;
	}; // struct decay

	#if __HAS_CPP14
	template <typename T>
	using decay_t = typename decay<T>::type;
	#endif // __HAS_CPP14
	/**
	 * @brief applies type transformations as when passing a function argument by value
	 * @brief 实施当按值传递实参给函数时所进行的类型变换
	 * @brief 该别名即表示变换的结果
	 * 
	 * @tparam T 需要进行变换的类型
	*/
	#pragma endregion decay

	// determines the common type of a group of types
	// 确定一组类型的公共类型
	#pragma region common_type
	/**
	 * @brief determines the common type of a group of types
	 * @brief 确定一组类型的公共类型
	 * @brief 如果存在此公共类型，则有成员别名 type 指向它
	 * 
	 * @tparam ...T 需要确定公共类型的一组类型
	*/
	template <typename...T>
	struct common_type {}; // 若 sizeof...(T) 为零，则无成员 type 

	// 若 sizeof...(T) 为一（即 T...只含一个类型 T0）
	// 则成员 type 指名与 common_type<T0, T0>::type 相同的类型，若它存在
	// 否则无成员 type
	template <typename T0>
	struct common_type<T0>: common_type<T0, T0> {};

	__INNER_BEGIN //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	template <typename, typename, typename = void>
	struct __get_two_common_type { /* 否则：无成员 type */ };

	template <typename T1, typename T2>
	struct __get_two_common_type<T1, T2,

		/* 若 decay<decltype(false ? declval<T1>() : declval<T2>())>::type 是合法类型 */
		__void_t<typename decay<decltype(false ? declval<T1>() : declval<T2>())>::type>
	> {
		/* 则：成员 type 代表该类型 */
		using type = typename decay<decltype(false ? declval<T1>() : declval<T2>())>::type;
	};
	__INNER_END //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// 若 sizeof...(T) 为二（即 T... 含恰好二个成员 T1 与 T2）
	template <typename T1, typename T2>
	struct common_type<T1, T2>: conditional<

		/* 若应用 std::decay 到至少 T1 与 T2 中至少一个类型后产生相异类型 */
		! is_same<T1, typename decay<T1>::type>::value ||
		! is_same<T2, typename decay<T2>::type>::value,

		// 则：成员 type 指名与 common_type<decay<T1>::type, decay<T2>::type>::type 相同的类型，若它存在
		// 若不存在，则无成员 type
		common_type<typename decay<T1>::type, typename decay<T2>::type>,

		// 否则：若 decay<decltype(false ? declval<T1>() : declval<T2>())>::type 是合法类型
		// 则成员 type 代表该类型
		// 否则，无成员 type
		__INNER_NAMESPACE::__get_two_common_type<T1, T2>
	>::type {};

	__INNER_BEGIN //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv 
	template <typename, typename, typename, typename...>
	struct __get_multi_common_type { /* 其他所有情况下，无成员 type */ };

	template <typename T1, typename T2, typename... R>
	struct __get_multi_common_type<

		/* 则：若 common_type<T1, T2>::type 存在 */
		__void_t<typename common_type<T1, T2>::type>, 
		T1, T2, R...
	> {
		/* 则成员 type 指代 common_type<common_type<T1, T2>::type, R...>::type ，若存在这种类型 */
		using type = common_type<typename common_type<T1, T2>::type, R...>::type;
	};
	__INNER_END //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// 若 sizeof...(T) 大于二（即 T... 由类型 T1, T2, R... 组成）
	// 则：若 common_type<T1, T2>::type 存在
	// 则成员 type 指代 common_type<common_type<T1, T2>::type, R...>::type ，若存在这种类型
	// 其他所有情况下，无成员 type 
	template <typename T1, typename T2, typename... R>
	struct common_type<T1, T2, R...>:
		__INNER_NAMESPACE::__get_multi_common_type<void, T1, T2, R...> {};

	#if __HAS_CPP14
	/**
	 * @brief determines the common type of a group of types
	 * @brief 确定一组类型的公共类型
	 * @brief 如果存在此公共类型，则该别名指向它，否则该别名非良构
	 * 
	 * @tparam ...T 需要确定公共类型的一组类型
	*/
	template <typename...T>
	using common_type_t = typename common_type<T...>::type;
	#endif // __HAS_CPP14
	#pragma endregion common_type



	// ======================
	//
	//  Supported operations
	//  判断类型是否支持某种行为
	// 
	// ======================

	// (non-standard feature) checks if a type is a referenceable type
	// （非标准内容）检查类型是否是可引用类型
	#pragma region is_referenceable

	__INNER_BEGIN
	template <typename T, typename = void>
	struct __test_is_referenceable: false_type {};

	template <typename T>
	struct __test_is_referenceable<T, __void_t<T&>>: true_type {};
	__INNER_END

	/**
	 * @brief (non-standard feature) checks if a type is a referenceable type
	 * @brief （非标准内容）检查类型是否是可引用类型
	 * @brief 包含成员 type，表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_referenceable: __INNER_NAMESPACE::__test_is_referenceable<T> {};

	#if __HAS_CPP17
	/**
	 * @brief (non-standard feature) checks if a type is a referenceable type
	 * @brief （非标准内容）检查类型是否是可引用类型
	 * @brief 其本身即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_referenceable_v = is_referenceable<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_referenceable

	#pragma region constructible series

	// checks if a type has a constructor for specific arguments
	// 检查类型是否带有针对特定实参的构造函数
	#pragma region is_constructible

	__INNER_BEGIN
	// 要求变量定义 T obj(std::declval<Args>()...); 为良构
	template <typename T, typename... Args, typename = decltype(T(declval<Args>()...))>
	auto __try_construct(int) -> true_type {}

	template <typename...>
	auto __try_construct(...) -> false_type {}
	__INNER_END

	/**
	 * @brief checks if a type has a constructor for specific arguments
	 * @brief 检查类型是否带有针对特定实参的构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam ...Args 构造函数的特定实参
	*/
	template <typename T, typename... Args>
	struct is_constructible: conditional<
		// 如果 T 是对象或引用类型
		is_object<T>::value || is_reference<T>::value,
		// 且变量定义 T obj(std::declval<Args>()...); 为良构
		decltype(__INNER_NAMESPACE::__try_construct<T, Args...>(0)), 
		// 对于其它类型，value 等于 false
		false_type
	>::type {};
		
	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a constructor for specific arguments
	 * @brief 检查类型是否带有针对特定实参的构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam ...Args 构造函数的特定实参
	*/
	template <typename T, typename... Args>
	inline constexpr bool is_constructible_v = is_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_constructible

	#pragma region is_trivially_constructible
	/**
	 * @brief checks if a type has a trivial constructor for specific arguments
	 * @brief 检查类型是否带有针对特定实参的平凡的构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam ...Args 构造函数的特定实参
	*/
	template <typename T, typename... Args>
	struct is_trivially_constructible: integral_constant<bool, __is_trivially_constructible(T, Args...)> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a trivial constructor for specific arguments
	 * @brief 检查类型是否带有针对特定实参的平凡的构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam ...Args 构造函数的特定实参
	*/
	template <typename T, typename... Args>
	inline constexpr bool is_trivially_constructible_v = is_trivially_constructible<T, Args...>::type;
	#endif // __HAS_CPP17
	#pragma endregion is_trivially_constructible

	#pragma region is_nothrow_constructible
	/**
	 * @brief checks if a type has a nothrow constructor for specific arguments
	 * @brief 检查类型是否带有不抛出异常的针对特定实参的构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam ...Args 构造函数的特定实参
	*/
	template <typename T, typename... Args>
	struct is_nothrow_constructible: integral_constant<
		bool,
		is_constructible<T, Args...>::value &&
		noexcept(T(declval<Args>()...))
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a nothrow constructor for specific arguments
	 * @brief 检查类型是否带有不抛出异常的针对特定实参的构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam ...Args 构造函数的特定实参
	*/
	template <typename T, typename... Args>
	inline constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_nothrow_constructible

	// checks if a type has a default constructor
	// 检查类型是否有默认构造函数 
	#pragma region is_default_constructible
	/**
	 * @brief checks if a type has a default constructor
	 * @brief 检查类型是否有默认构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_default_constructible: is_constructible<T> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a default constructor
	 * @brief 检查类型是否有默认构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_default_constructible_v = is_default_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_default_constructible

	#pragma region is_trivially_default_constructible
	/**
	 * @brief checks if a type has a trivial default constructor
	 * @brief 检查类型是否有平凡的默认构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_trivially_default_constructible: is_trivially_constructible<T> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a trivial default constructor
	 * @brief 检查类型是否有平凡的默认构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_trivially_default_constructible_v = is_trivially_default_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_trivially_default_constructible

	#pragma region is_nothrow_default_constructible
	/**
	 * @brief checks if a type has a nothrow default constructor
	 * @brief 检查类型是否有不抛出异常的默认构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_nothrow_default_constructible: is_nothrow_constructible<T> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a nothrow default constructor
	 * @brief 检查类型是否有不抛出异常的默认构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_nothrow_default_constructible_v = is_nothrow_default_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_nothrow_default_constructible

	// checks if a type has a copy constructor
	// 检查类型是否拥有复制构造函数
	#pragma region is_copy_constructible
	/**
	 * @brief checks if a type has a copy constructor
	 * @brief 检查类型是否有复制构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_copy_constructible: conditional<
		is_referenceable<T>::value,
		is_constructible<T, typename add_lvalue_reference<typename add_const<T>::type>::type>, 
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a copy constructor
	 * @brief 检查类型是否有复制构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_copy_constructible

	#pragma region is_trivially_copy_constructible
	/**
	 * @brief checks if a type has a trivial copy constructor
	 * @brief 检查类型是否有平凡的复制构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_trivially_copy_constructible: conditional<
		is_referenceable<T>::value,
		is_trivially_constructible<T, typename add_lvalue_reference<typename add_const<T>::type>::type>, 
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a trivial copy constructor
	 * @brief 检查类型是否有平凡的复制构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_trivially_copy_constructible_v = is_trivially_copy_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_trivially_copy_constructible

	#pragma region is_nothrow_copy_constructible
	/**
	 * @brief checks if a type has a nothrow copy constructor
	 * @brief 检查类型是否有不抛出异常的复制构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_nothrow_copy_constructible: conditional<
		is_referenceable<T>::value,
		is_nothrow_constructible<T, typename add_lvalue_reference<typename add_const<T>::type>::type>, 
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a nothrow copy constructor
	 * @brief 检查类型是否有不抛出异常的复制构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_nothrow_copy_constructible_v = is_nothrow_copy_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_nothrow_copy_constructible

	// checks if a type can be constructed from an rvalue reference
	// 检查类型是否能从右值引用构造 
	#pragma region is_move_constructible
	/**
	 * @brief checks if a type can be constructed from an rvalue reference
	 * @brief 检查类型是否有移动构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_move_constructible: conditional<
		is_referenceable<T>::value,
		is_constructible<T, typename add_rvalue_reference<T>::type>, 
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type can be constructed from an rvalue reference
	 * @brief 检查类型是否有移动构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_move_constructible_v = is_move_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_move_constructible

	#pragma region is_trivially_move_constructible
	/**
	 * @brief checks if a type can be trivially constructed from an rvalue reference
	 * @brief 检查类型是否有平凡的移动构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_trivially_move_constructible: conditional<
		is_referenceable<T>::value,
		is_trivially_constructible<T, typename add_rvalue_reference<T>::type>, 
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type can be trivially constructed from an rvalue reference
	 * @brief 检查类型是否有平凡的移动构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_trivially_move_constructible_v = is_trivially_move_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_trivially_move_constructible

	#pragma region is_nothrow_move_constructible
	/**
	 * @brief checks if a type can be nothrown constructed from an rvalue reference
	 * @brief 检查类型是否有不抛出异常的移动构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_nothrow_move_constructible: conditional<
		is_referenceable<T>::value,
		is_nothrow_constructible<T, typename add_rvalue_reference<T>::type>, 
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type can be nothrown constructed from an rvalue reference
	 * @brief 检查类型是否有不抛出异常的移动构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_nothrow_move_constructible_v = is_nothrow_move_constructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_nothrow_move_constructible

	#pragma endregion constructible series

	#pragma region assignable series

	// checks if a type has an assignment operator for a specific argument
	// 检查类型是否拥有针对特定实参的赋值运算符
	#pragma region is_assignable

	__INNER_BEGIN
	// 要求表达式 declval<T>() = declval<U>() 在不求值语境为良态
	template <typename T, typename U, typename = decltype(declval<T>() = declval<U>())>
	auto __try_assign(int) -> true_type {}

	template <typename...>
	auto __try_assign(...) -> false_type {}
	__INNER_END

	/**
	 * @brief checks if a type has an assignment operator for a specific argument
	 * @brief 检查类型是否拥有针对特定实参的赋值运算符
	 * @brief 包含成员 type，表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam U 赋值运算符的实参类型
	*/
	template <typename T, typename U>
	struct is_assignable: decltype(__INNER_NAMESPACE::__try_assign<T, U>(0)) {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has an assignment operator for a specific argument
	 * @brief 检查类型是否拥有针对特定实参的赋值运算符
	 * @brief 其本身即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam U 赋值运算符的实参类型
	*/
	template <typename T, typename U>
	inline constexpr bool is_assignable_v = is_assignable<T, U>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_assignable

	#pragma region is_trivially_assignable
	/**
	 * @brief checks if a type has a trivial assignment operator for a specific argument
	 * @brief 检查类型是否拥有平凡的针对特定实参的赋值运算符
	 * @brief 包含成员 type，表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam U 赋值运算符的实参类型
	*/
	template <typename T, typename U>
	struct is_trivially_assignable: integral_constant<bool, __is_trivially_assignable(T, U)> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a trivial assignment operator for a specific argument
	 * @brief 检查类型是否拥有平凡的针对特定实参的赋值运算符
	 * @brief 其本身即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam U 赋值运算符的实参类型
	*/
	template <typename T, typename U>
	inline constexpr bool is_trivially_assignable_v = is_trivially_assignable<T, U>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_trivially_assignable

	#pragma region is_nothrow_assignable
	/**
	 * @brief checks if a type has a nothrow assignment operator for a specific argument
	 * @brief 检查类型是否拥有不抛出异常的针对特定实参的赋值运算符
	 * @brief 包含成员 type，表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam U 赋值运算符的实参类型
	*/
	template <typename T, typename U>
	struct is_nothrow_assignable: integral_constant<
		bool,
		is_assignable<T, U>::value &&
		noexcept(declval<T>() = declval<U>())
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a nothrow assignment operator for a specific argument
	 * @brief 检查类型是否拥有不抛出异常的针对特定实参的赋值运算符
	 * @brief 其本身即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	 * @tparam U 赋值运算符的实参类型
	*/
	template <typename T, typename U>
	inline constexpr bool is_nothrow_assignable_v = is_nothrow_assignable<T, U>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_nothrow_assignable
 

	// checks if a type has a copy assignment operator
	// 检查类型是否拥有复制赋值运算符
	#pragma region is_copy_assignable
	/**
	 * @brief checks if a type has a copy assignment operator
	 * @brief 检查类型是否拥有复制赋值运算符
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_copy_assignable: conditional<
		is_referenceable<T>::value,
		is_assignable<
			typename add_lvalue_reference<T>::type,
			typename add_lvalue_reference<typename add_const<T>::type>::type
		>,
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a copy assignment operator
	 * @brief 检查类型是否拥有复制赋值运算符
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_copy_assignable_v = is_copy_assignable<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_copy_assignable

	#pragma region is_trivially_copy_assignable
	/**
	 * @brief checks if a type has a trivial copy assignment operator
	 * @brief 检查类型是否拥有平凡的复制赋值运算符
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_trivially_copy_assignable: conditional<
		is_referenceable<T>::value,
		is_trivially_assignable<
			typename add_lvalue_reference<T>::type,
			typename add_lvalue_reference<typename add_const<T>::type>::type
		>,
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a trivial copy assignment operator
	 * @brief 检查类型是否拥有平凡的复制赋值运算符
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_trivially_copy_assignable_v = is_trivially_copy_assignable<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_trivially_copy_assignable

	#pragma region is_nothrow_copy_assignable
	/**
	 * @brief checks if a type has a nothrow copy constructor
	 * @brief 检查类型是否有不抛出异常的复制构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_nothrow_copy_assignable: conditional<
		is_referenceable<T>::value,
		is_nothrow_assignable<
			typename add_lvalue_reference<T>::type,
			typename add_lvalue_reference<typename add_const<T>::type>::type
		>,
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a nothrow copy constructor
	 * @brief 检查类型是否有不抛出异常的复制构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_nothrow_copy_assignable_v = is_nothrow_copy_assignable<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_nothrow_copy_assignable


	// checks if a type can be constructed from an rvalue reference
	// 检查类型是否能从右值引用构造 
	#pragma region is_move_assignable
	/**
	 * @brief checks if a type can be constructed from an rvalue reference
	 * @brief 检查类型是否有移动构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_move_assignable: conditional<
		is_referenceable<T>::value,
		is_assignable<
			typename add_lvalue_reference<T>::type, 
			typename add_rvalue_reference<T>::type
		>, 
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type can be constructed from an rvalue reference
	 * @brief 检查类型是否有移动构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_move_assignable_v = is_move_assignable<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_move_assignable

	#pragma region is_trivially_move_assignable
	/**
	 * @brief checks if a type can be trivially constructed from an rvalue reference
	 * @brief 检查类型是否有平凡的移动构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_trivially_move_assignable: conditional<
		is_referenceable<T>::value,
		is_trivially_assignable<
			typename add_lvalue_reference<T>::type, 
			typename add_rvalue_reference<T>::type
		>, 
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type can be trivially constructed from an rvalue reference
	 * @brief 检查类型是否有平凡的移动构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_trivially_move_assignable_v = is_trivially_move_assignable<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_trivially_move_assignable

	#pragma region is_nothrow_move_assignable
	/**
	 * @brief checks if a type can be nothrown constructed from an rvalue reference
	 * @brief 检查类型是否有不抛出异常的移动构造函数
	 * @brief 包含成员 value, 表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_nothrow_move_assignable: conditional<
		is_referenceable<T>::value,
		is_nothrow_assignable<
			typename add_lvalue_reference<T>::type, 
			typename add_rvalue_reference<T>::type
		>, 
		false_type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type can be nothrown constructed from an rvalue reference
	 * @brief 检查类型是否有不抛出异常的移动构造函数
	 * @brief 该常量即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_nothrow_move_assignable_v = is_nothrow_move_assignable<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_nothrow_move_assignable

	#pragma endregion assignable series

	#pragma region destructible series

	// checks if a type has a non-deleted destructor
	// 检查类型是否拥有未被弃置的析构函数
	#pragma region is_destructible

	__INNER_BEGIN
	// 要求表达式 declval<U&>().~U() 在不求值语境合法
	template <typename U, typename = decltype(declval<U&>().~U())>
	auto __try_destruct(int) -> true_type {};
	template <typename...>
	auto __try_destruct(...) -> false_type {};

	template <typename T>
	struct __is_unknown_bound_array: false_type {};
	template <typename T>
	struct __is_unknown_bound_array<T[]>: true_type {};
	__INNER_END

	/**
	 * @brief checks if a type has a non-deleted destructor
	 * @brief 检查类型是否拥有未被弃置的析构函数
	 * @brief 包含成员 type，表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_destructible: conditional<
		// 如果 T 是引用类型
		is_reference<T>::value,
		// 则成员常量 value 等于 true
		true_type,
		typename conditional<
			// 如果 T 是（可以有 cv 限定的） void、函数类型或未知边界数组
			is_void<T>::value || 
			is_function<T>::value || 
			__INNER_NAMESPACE::__is_unknown_bound_array<T>::value,
			// 则 value 等于 false
			false_type,
			typename conditional<
				// 如果 T 是对象类型
				is_object<T>::value, 
				// 且对于作为 remove_all_extents<T>::type 的类型 U
				// 表达式 declval<U&>().~U() 在不求值语境合法
				// 则 value 等于 true
				decltype(__INNER_NAMESPACE::__try_destruct<typename remove_all_extents<T>::type>(0)),
				// 否则， value 等于 false
				false_type
			>::type
		>::type
	>::type {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a non-deleted destructor
	 * @brief 检查类型是否拥有未被弃置的析构函数
	 * @brief 其本身即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_destructible_v = is_destructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_destructible

	#pragma region is_trivially_destructible
	/**
	 * @brief checks if a type has a trivial non-deleted destructor
	 * @brief 检查类型是否拥有平凡的未被弃置的析构函数
	 * @brief 包含成员 type，表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_trivially_destructible: integral_constant<bool, __is_trivially_destructible(T)> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a trivial non-deleted destructor
	 * @brief 检查类型是否拥有平凡的未被弃置的析构函数
	 * @brief 其本身即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_trivially_destructible_v = is_trivially_destructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_trivially_destructible

	#pragma region is_nothrow_destructible

	__INNER_BEGIN
	template <typename U>
	struct __test_nothrow_destructible: integral_constant<bool, noexcept(declval<U&>().~U())> {};
	__INNER_END

	/**
	 * @brief checks if a type has a nothrow non-deleted destructor
	 * @brief 检查类型是否拥有不抛出异常的未被弃置的析构函数
	 * @brief 包含成员 type，表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	struct is_nothrow_destructible: integral_constant<
		bool,
		is_destructible<T>::value &&
		__INNER_NAMESPACE::__test_nothrow_destructible<typename remove_all_extents<T>::type>::value
	> {};

	#if __HAS_CPP17
	/**
	 * @brief checks if a type has a nothrow non-deleted destructor
	 * @brief 检查类型是否拥有不抛出异常的未被弃置的析构函数
	 * @brief 其本身即表示检查的结果
	 * 
	 * @tparam T 需要检查的类型
	*/
	template <typename T>
	inline constexpr bool is_nothrow_destructible_v = is_nothrow_destructible<T>::value;
	#endif // __HAS_CPP17
	#pragma endregion is_nothrow_destructible

	#pragma endregion destructible series

} // namespace __MY_NAMESPACE
#endif // __HAS_CPP11
