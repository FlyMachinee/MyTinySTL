#ifndef __CORE_H__
#define __CORE_H__

#define __MY_NAMESPACE my

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

#endif // ifndef __CORE_H__
