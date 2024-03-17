#ifndef __CORE_H__
#define __CORE_H__

#define __MY_NAMESPACE my

#define __INNER_NAMESPACE inner
#define __INNER_BEGIN namespace __INNER_NAMESPACE {
#define __INNER_END }

#pragma region __HAS_CPPXX

#ifndef __HAS_CPP11
#if __cplusplus >= 201103L
#define __HAS_CPP11 1
#else
#define __HAS_CPP11 0
#endif
#endif // __HAS_CPP11

#ifndef __HAS_CPP14
#if __cplusplus >= 201402L
#define __HAS_CPP14 1
#else
#define __HAS_CPP14 0
#endif
#endif // __HAS_CPP14

#ifndef __HAS_CPP17
#if __cplusplus >= 201703L
#define __HAS_CPP17 1
#else
#define __HAS_CPP17 0
#endif
#endif // __HAS_CPP17

#ifndef __HAS_CPP20
#if __cplusplus >= 202002L
#define __HAS_CPP20 1
#else
#define __HAS_CPP20 0
#endif
#endif // __HAS_CPP20

#define __HAS_CPP23 0

#pragma endregion __HAS_CPPXX

#if __HAS_CPP17
#define __NODISCARD [[nodiscard]]
#else // ^^^ __HAS_CPP17 / vvv !__HAS_CPP17
#define __NODISCARD
#endif // __HAS_CPP17

#pragma region __CONSTEXPRXX

#if __HAS_CPP11
#define __CONSTEXPR11
#else // ^^^ __HAS_CPP11 / vvv !__HAS_CPP11
#define __CONSTEXPR11
#endif // __HAS_CPP11


#if __HAS_CPP14
#define __CONSTEXPR14 constexpr
#else // ^^^ __HAS_CPP20 / vvv !__HAS_CPP20
#define __CONSTEXPR14
#endif // __HAS_CPP20

#if __HAS_CPP20
#define __CONSTEXPR20 constexpr
#else // ^^^ __HAS_CPP20 / vvv !__HAS_CPP20
#define __CONSTEXPR20
#endif // __HAS_CPP20

#if __HAS_CPP23
#define __CONSTEXPR23 constexpr
#else // ^^^ __HAS_CPP23 / vvv !__HAS_CPP23
#define __CONSTEXPR23
#endif // __HAS_CPP23

#pragma endregion __CONSTEXPRXX

#pragma region __INLINEXX

#if __HAS_CPP17
#define __INLINE17 inline
#else // ^^^ __HAS_CPP17 / vvv !__HAS_CPP17
#define __INLINE17

#endif // __HAS_CPP17

#pragma endregion __INLINEXX


typedef unsigned long long size_t;

#endif // ifndef __CORE_H__
