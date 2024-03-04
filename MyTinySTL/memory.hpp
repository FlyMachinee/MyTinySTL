#pragma once
#include "macros.h"
#include <cstddef>

namespace __MY_NAMESPACE {

	template <typename ValueT>
	struct allocator {

	public:
		using value_type = ValueT;
		using pointer = ValueT*;
		using const_pointer = const ValueT*;
		using reference = ValueT&;
		using const_reference = const ValueT&;
		using size_type = ::size_t;
		using difference_type = ::ptrdiff_t;
	
	public:
		constexpr allocator() noexcept;

		constexpr allocator(const allocator& other) noexcept;

		template <typename OtherValueT>
		constexpr allocator(const allocator<OtherValueT>& other) noexcept;

		constexpr ~allocator();

		/**
		 * @brief allocates uninitialized storage
		 * @param n the number of objects to allocate storage for
		 * @return Pointer to the first element of an array of n objects of type T whose elements have not been constructed yet.
		*/
		[[nodiscard]]
		constexpr pointer allocate(size_type n);

		/**
		 * @brief deallocates storage
		 * @param ptr pointer obtained from allocate()
		 * @param n number of objects earlier passed to allocate()
		*/
		constexpr void deallocate(pointer ptr, size_type n);

		/**
		 * @brief compares two allocator instances
		 * @param lhs default allocators to compare
		 * @param rhs default allocators to compare
		 * @return true if two allocators are equivalent
		*/
		template <typename ValueT1, typename ValueT2>
		friend constexpr bool operator==(const allocator<ValueT1>& lhs, const allocator<ValueT2>& rhs) noexcept;

	};
} // namespace My


