#pragma once
#include "core.h"

#if __HAS_CPP11
#include <initializer_list>
#endif // __HAS_CPP11

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

} // namespace __MY_NAMESPACE
