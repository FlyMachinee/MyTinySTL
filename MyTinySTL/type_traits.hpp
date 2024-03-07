#pragma once
#include "macros.h"

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
	 * @brief 根据传入的布尔常量, 决定该别名模板是否有效
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




	#if __HAS_CPP17
	
	#endif // __HAS_CPP17

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

	/**
	 * @brief adds an lvalue reference to the given type
	 * @brief 为类型引入左值引用, 遵循引用折叠原则
	 * @brief 包含成员别名 type, 表示添加的结果
	 * 
	 * @tparam T 需要添加左值引用的类型
	*/
	template <typename T>
	struct add_lvalue_reference { using type = typename __try_add_reference<T>::lvalue_type; };

	/**
	 * @brief adds an rvalue reference to the given type
	 * @brief 为类型引入右值引用, 遵循引用折叠原则
	 * @brief 包含成员别名 type, 表示添加的结果
	 * 
	 * @tparam T 需要添加右值引用的类型
	*/
	template <typename T>
	struct add_rvalue_reference { using type = typename __try_add_reference<T>::rvalue_type; };

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
	template <typename T> requires requires { typename type_identity<typename remove_reference<T>::type*>; }
	struct add_pointer<T> { using type = typename remove_reference<T>::type*; };

	#else // ^^^ __HAS_CPP20 / vvv !__HAS_CPP20
	template <typename T, typename = void>
	struct __try_add_pointer {
		using pointer_type = T;
	};

	template <typename T>
	struct __try_add_pointer<T, typename __void_t<typename remove_reference<T>::type*>> {
		using pointer_type = typename remove_reference<T>::type*;
	};

	/**
	 * @brief adds a pointer to the given type
	 * @brief 对给定类型添加一层指针
	 * @brief 包含成员别名 type, 表示添加的结果
	 * 
	 * @tparam T 需要添加指针的类型
	*/
	template <typename T>
	struct add_pointer { using type = typename __try_add_pointer<T>::pointer_type; };
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



	// ============================================
	//
	//  Type properties and judges in compile-time
	//  类型的性质及其分类的编译期内判断
	//
	// ============================================

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
	#pragma region is_null_pointer
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

	template <typename T, ::std::size_t N>
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




} // namespace __MY_NAMESPACE
#endif // __HAS_CPP11
