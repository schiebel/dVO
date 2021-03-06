// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(CPPRX_RX_INCLUDES_HPP)
#define CPPRX_RX_INCLUDES_HPP

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

#include <stdlib.h>

#include <iostream>
#include <iomanip>

#include <exception>
#include <functional>
#include <memory>
#include <vector>
#include <algorithm>
#include <atomic>
#include <map>
#include <mutex>
#include <deque>
#include <thread>
#include <future>
#include <vector>
#include <list>
#include <queue>
#include <chrono>
#include <condition_variable>

// some configuration macros
#if defined(_MSC_VER)

#if _MSC_VER > 1600 
#define RXCPP_USE_RVALUEREF 1
#endif

#if _MSC_VER >= 1800
#define RXCPP_USE_VARIADIC_TEMPLATES 1
#endif

#if _CPPRTTI
#define RXCPP_USE_RTTI 1
#endif

#elif defined(__clang__)

#if __has_feature(cxx_rvalue_references)
#define RXCPP_USE_RVALUEREF 1
#endif
#if __has_feature(cxx_rtti)
#define RXCPP_USE_RTTI 1
#endif
#if __has_feature(cxx_variadic_templates)
#define RXCPP_USE_VARIADIC_TEMPLATES 1
#endif

#endif

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#define RXCPP_USE_WINRT 0
#else
#define RXCPP_USE_WINRT 1
#endif

#if defined(RXCPP_FORCE_USE_VARIADIC_TEMPLATES)
#undef RXCPP_USE_VARIADIC_TEMPLATES
#define RXCPP_USE_VARIADIC_TEMPLATES RXCPP_FORCE_USE_VARIADIC_TEMPLATES
#endif

#if defined(RXCPP_FORCE_USE_RVALUEREF)
#undef RXCPP_USE_RVALUEREF
#define RXCPP_USE_RVALUEREF RXCPP_FORCE_USE_RVALUEREF
#endif

#if defined(RXCPP_FORCE_USE_RTTI)
#undef RXCPP_USE_RTTI
#define RXCPP_USE_RTTI RXCPP_FORCE_USE_RTTI
#endif

#if defined(RXCPP_FORCE_USE_WINRT)
#undef RXCPP_USE_WINRT
#define RXCPP_USE_WINRT RXCPP_FORCE_USE_WINRT
#endif

#include "rx-util.hpp"
#include "rx-base.hpp"
#include "rx-scheduler.hpp"
#include "rx-windows.hpp"
#include "rx-operators.hpp"
#include "rx-winrt.hpp"

#pragma pop_macro("min")
#pragma pop_macro("max")

#endif
