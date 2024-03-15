#pragma once
#include "core.h"

#if __HAS_CPP11
#include <initializer_list>
#include "type_traits.hpp"

#if __HAS_CPP20
// #include "compare.hpp"
#endif // __HAS_CPP20

namespace __MY_NAMESPACE {

	namespace 
		#if __HAS_CPP20
		[[deprecated("As of C++20, my::rel_ops are deprecated in favor of operator<=>.")]] 
		#endif // __HAS_CPP20
	rel_ops {

		template <typename T>
		/**
		 * @brief 使用 operator== 实现 operator!=
		 * @tparam T 参与比较的类型
		 * @param lhs 比较中的左操作数
		 * @param rhs 比较中的右操作数
		 * @return lhs != rhs 的值，更确切地，!(lhs == rhs)
		*/
		__NODISCARD bool operator!=(const T& lhs, const T& rhs) {
			return !(lhs == rhs);
		}

		template <typename T>
		/**
		 * @brief 使用 operator< 实现 operator>
		 * @tparam T 参与比较的类型
		 * @param lhs 比较中的左操作数
		 * @param rhs 比较中的右操作数
		 * @return lhs > rhs 的值，更确切地，rhs < lhs
		*/
		__NODISCARD bool operator>(const T& lhs, const T& rhs) {
			return rhs < lhs;
		}

		template <typename T>
		/**
		 * @brief 使用 operator< 实现 operator<=
		 * @tparam T 参与比较的类型
		 * @param lhs 比较中的左操作数
		 * @param rhs 比较中的右操作数
		 * @return lhs <= rhs 的值，更确切地，!(rhs < lhs)
		*/
		__NODISCARD bool operator<=(const T& lhs, const T& rhs) {
			return !(rhs < lhs);
		}

		template <typename T>
		/**
		 * @brief 使用 operator< 实现 operator>=
		 * @tparam T 参与比较的类型
		 * @param lhs 比较中的左操作数
		 * @param rhs 比较中的右操作数
		 * @return lhs >= rhs 的值，更确切地，!(lhs < rhs)
		*/
		__NODISCARD bool operator>=(const T& lhs, const T& rhs) {
			return !(lhs < rhs);
		}

	} // namespace __MY_NAMESPACE::rel_ops
	
	#pragma region move
	/**
	 * @brief 用于指示对象 t 可以“被移动”，即允许从 t 到另一对象的有效率的资源传递
	 * @brief 生成标识其参数 t 的亡值表达式，准确等价于到右值引用类型的 static_cast
	 * @tparam T （无需填写）
	 * @param t 要被移动的对象
	 * @return static_cast<typename remove_reference<T>::type&&>(t)
	*/
	template <typename T>
	__CONSTEXPR14 typename remove_reference<T>::type&& move(T&& t) noexcept {
		return static_cast<typename remove_reference<T>::type&&>(t);
	}
	#pragma endregion move

	#pragma region forward
	/**
	 * @brief forwards a function argument
	 * @brief 转发一个函数实参，配合万能（转发）引用以实现完美转发
	 * @brief 该重载转发左值为左值或右值
	 * @tparam T 由万能引用 T&& 推导出的实参 T
	 * @param t 要转发的对象
	 * @return static_cast<T&&>(t)
	*/
	template <typename T>
	__CONSTEXPR14 T&& forward(typename std::remove_reference<T>::type& t) noexcept {
		return static_cast<T&&>(t);
	}

	/**
	 * @brief forwards a function argument
	 * @brief 转发一个函数实参，配合万能（转发）引用以实现完美转发
	 * @brief 该重载转发右值为右值并禁止右值的转发为左值
	 * @tparam T 由万能引用 T&& 推导出的实参 T
	 * @param t 要转发的对象
	 * @return static_cast<T&&>(t)
	*/
	template <typename T>
	__CONSTEXPR14 T&& forward(typename std::remove_reference<T>::type&& t) noexcept {
		return static_cast<T&&>(t);
	}
	#pragma endregion forward

	#pragma region swap

	#if __HAS_CPP17
	// 此重载只有在 is_move_constructible_v<T> && is_move_assignable_v<T> 是 true 时才会参与重载决议。 (C++17 起)
	template <typename T, typename/*  = enable_if_t<is_move_constructible_v<T> && is_move_assignable_v<T>> [in type_traits] */>
	#else // ^^^ __HAS_CPP17 / vvv !__HAS_CPP17
	template <typename T>
	#endif // __HAS_CPP17
	/**
	 * @brief swaps the values of two objects
	 * @brief 交换两个对象的值
	 * @tparam T 交换对象的类型（可自动推导）
	 * @param left 要交换的值
	 * @param right 要交换的值
	*/
	__CONSTEXPR20 void swap(T& left, T& right) noexcept(
		is_nothrow_move_constructible<T>::value&&
		is_nothrow_move_assignable<T>::value
	) {
		T temp = move(left);
		left = move(right);
		right = move(temp);
	}

	#if __HAS_CPP17
	// 此重载只有在 is_swappable_v<T2> 是 true 时才会参与重载决议。 (C++17 起)
	template <typename T, ::size_t N, typename /* = enable_if_t<is_swappable<T>::value> [in type_traits] */>
	#else // ^^^ __HAS_CPP17 / vvv !__HAS_CPP17
	template <typename T, ::size_t N>
	#endif // __HAS_CPP17
	/**
	 * @brief swaps the values of two arrays
	 * @brief 交换两个等长内建数组的值
	 * @tparam T 数组中存储对象的类型（可自动推导）
	 * @tparam N 数组的长度（可自动推导）
	 * @param left_arr 要交换的数组
	 * @param right_arr 要交换的数组
	*/
	__CONSTEXPR20 void swap(T(&left_arr)[N], T(&right_arr)[N])
		#if __HAS_CPP17
		noexcept(is_nothrow_swappable<T>::value)
		#else // ^^^ __HAS_CPP17 / vvv !__HAS_CPP17 
		noexcept(noexcept(swap(*a, *b)))
		#endif // __HAS_CPP17
	{
		T* l_ptr = left_arr;
		T* l_end_ptr = l_ptr + N;
		T* r_ptr = right_arr;
		for (; l_ptr != l_end_ptr; ++l_ptr, ++r_ptr) {
			swap(*l_ptr, *r_ptr);
		}
	}
	#pragma endregion swap

} // namespace __MY_NAMESPACE
