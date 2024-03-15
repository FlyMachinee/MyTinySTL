﻿#ifndef __CORE_H__
#define __CORE_H__

#define __MY_NAMESPACE my

#define __INNER_NAMESPACE inner
#define __INNER_BEGIN namespace __INNER_NAMESPACE {
#define __INNER_END }

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

#if __HAS_CPP17
#define __NODISCARD [[nodiscard]]
#else // ^^^ __HAS_CPP17 / vvv !__HAS_CPP17
#define __NODISCARD
#endif // __HAS_CPP17

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

typedef unsigned long long size_t;

#endif // ifndef __CORE_H__
