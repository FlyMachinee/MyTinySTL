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
	


} // namespace __MY_NAMESPACE
