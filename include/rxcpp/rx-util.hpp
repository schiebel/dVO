// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once
#include "rx-includes.hpp"

#if !defined(CPPRX_RX_UTIL_HPP)
#define CPPRX_RX_UTIL_HPP

#if !defined(RXCPP_THREAD_LOCAL)
#if defined(_MSC_VER)
#define RXCPP_THREAD_LOCAL __declspec(thread)
#else
#define RXCPP_THREAD_LOCAL __thread
#endif
#endif

#if !defined(RXCPP_SELECT_ANY)
#if defined(_MSC_VER)
#define RXCPP_SELECT_ANY __declspec(selectany)
#else
#define RXCPP_SELECT_ANY 
#endif
#endif

#define RXCPP_CONCAT(Prefix, Suffix) Prefix ## Suffix
#define RXCPP_CONCAT_EVALUATE(Prefix, Suffix) RXCPP_CONCAT(Prefix, Suffix)

#define RXCPP_MAKE_IDENTIFIER(Prefix) RXCPP_CONCAT_EVALUATE(Prefix, __LINE__)

namespace rxcpp { namespace util {

    template<class Type>
    struct identity 
    {
        typedef Type type;
        Type operator()(const Type& left) const {return left;}
    };

    template <class T>
    class maybe
    {
        bool is_set;
        typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type 
            storage;
    public:
        maybe()
        : is_set(false)
        {
        }

        maybe(T value)
        : is_set(false)
        {
            new (reinterpret_cast<T*>(&storage)) T(value);
            is_set = true;
        }

        maybe(const maybe& other)
        : is_set(false)
        {
            if (other.is_set) {
                new (reinterpret_cast<T*>(&storage)) T(*other.get());
                is_set = true;
            }
        }
        maybe(maybe&& other)
        : is_set(false)
        {
            if (other.is_set) {
                new (reinterpret_cast<T*>(&storage)) T(std::move(*other.get()));
                is_set = true;
                other.reset();
            }
        }

        ~maybe()
        {
            reset();
        }

        void reset()
        {
            if (is_set) {
                is_set = false;
                reinterpret_cast<T*>(&storage)->~T();
            }
        }

        T* get() {
            return is_set ? reinterpret_cast<T*>(&storage) : 0;
        }

        const T* get() const {
            return is_set ? reinterpret_cast<const T*>(&storage) : 0;
        }

        void set(T value) {
            if (is_set) {
                *reinterpret_cast<T*>(&storage) = std::move(value);
            } else {
                new (reinterpret_cast<T*>(&storage)) T(std::move(value));
                is_set = true;
            }
        }

        T& operator*() { return *get(); }
        const T& operator*() const { return *get(); }
        T* operator->() { return get(); }
        const T* operator->() const { return get(); }

        maybe& operator=(const T& other) {
            set(other);
            return *this;
        }
        maybe& operator=(const maybe& other) {
            if (const T* pother = other.get()) {
                set(*pother);
            } else {
                reset();
            }
            return *this;
        }

        // boolean-like operators
        operator T*() { return get(); }
        operator const T*() const { return get(); }

    private:
        
    };

    template<class T>
    struct reveal_type {private: reveal_type();};

#if RXCPP_USE_VARIADIC_TEMPLATES
    template <int... Indices> 
    struct tuple_indices;
    template <> 
    struct tuple_indices<-1> {                // for an empty std::tuple<> there is no entry
        typedef tuple_indices<> type;
    };
    template <int... Indices>
    struct tuple_indices<0, Indices...> {     // stop the recursion when 0 is reached
        typedef tuple_indices<0, Indices...> type;
    };
    template <int Index, int... Indices>
    struct tuple_indices<Index, Indices...> { // recursively build a sequence of indices
        typedef typename tuple_indices<Index - 1, Index, Indices...>::type type;
    };

    template <typename T>
    struct make_tuple_indices {
        typedef typename tuple_indices<std::tuple_size<T>::value - 1>::type type;
    };

    namespace detail {
    template<class T>
    struct tuple_dispatch;
    template<size_t... DisptachIndices>
    struct tuple_dispatch<tuple_indices<DisptachIndices...>> {
        template<class F, class T>
        static
        auto call(F&& f, T&& t) 
            -> decltype (std::forward<F>(f)(std::get<DisptachIndices>(std::forward<T>(t))...)) {
            return       std::forward<F>(f)(std::get<DisptachIndices>(std::forward<T>(t))...);
        }
    };}

    template<class F, class T>
    auto tuple_dispatch(F&& f, T&& t) 
        -> decltype(detail::tuple_dispatch<typename make_tuple_indices<typename std::decay<T>::type>::type>::call(std::forward<F>(f), std::forward<T>(t))) {
        return      detail::tuple_dispatch<typename make_tuple_indices<typename std::decay<T>::type>::type>::call(std::forward<F>(f), std::forward<T>(t));
    }

    namespace detail {
    template<class T>
    struct tuple_tie;
    template<size_t... TIndices>
    struct tuple_tie<tuple_indices<TIndices...>> {
        template<class T>
        static
        auto tie(T&& t) 
            -> decltype (std::tie(std::get<TIndices>(std::forward<T>(t))...)) {
            return       std::tie(std::get<TIndices>(std::forward<T>(t))...);
        }
    };}

    template<class T>
    auto tuple_tie(T&& t) 
        -> decltype(detail::tuple_tie<typename make_tuple_indices<typename std::decay<T>::type>::type>::tie(std::forward<T>(t))) {
        return      detail::tuple_tie<typename make_tuple_indices<typename std::decay<T>::type>::type>::tie(std::forward<T>(t));
    }

    struct as_tuple {
        template<class... AsTupleNext>
        auto operator()(AsTupleNext... x) 
            -> decltype(std::make_tuple(std::move(x)...)) {
            return      std::make_tuple(std::move(x)...);}
        template<class... AsTupleNext>
        auto operator()(AsTupleNext... x) const
            -> decltype(std::make_tuple(std::move(x)...)) {
            return      std::make_tuple(std::move(x)...);}
    };
#else
    namespace detail {
    template<size_t TupleSize>
    struct tuple_dispatch;
    template<>
    struct tuple_dispatch<0> {
        template<class F, class T>
        static auto call(F&& f, T&& ) 
            -> decltype (std::forward<F>(f)()) {
            return       std::forward<F>(f)();}
    };
    template<>
    struct tuple_dispatch<1> {
        template<class F, class T>
        static auto call(F&& f, T&& t) 
            -> decltype (std::forward<F>(f)(std::get<0>(std::forward<T>(t)))) {
            return       std::forward<F>(f)(std::get<0>(std::forward<T>(t)));}
    };
    template<>
    struct tuple_dispatch<2> {
        template<class F, class T>
        static auto call(F&& f, T&& t) 
            -> decltype (std::forward<F>(f)(
                         std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)))) {
            return       std::forward<F>(f)(
                         std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)));}
    };
    template<>
    struct tuple_dispatch<3> {
        template<class F, class T>
        static auto call(F&& f, T&& t) 
            -> decltype (std::forward<F>(f)(
                         std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)))) {
            return       std::forward<F>(f)(
                         std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)));}
    };
    template<>
    struct tuple_dispatch<4> {
        template<class F, class T>
        static auto call(F&& f, T&& t) 
            -> decltype (std::forward<F>(f)(
                         std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)))) {
            return       std::forward<F>(f)(
                         std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)));}
    };
    template<>
    struct tuple_dispatch<5> {
        template<class F, class T>
        static auto call(F&& f, T&& t) 
            -> decltype (std::forward<F>(f)(
                         std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)),
                         std::get<4>(std::forward<T>(t)))) {
            return       std::forward<F>(f)(
                         std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)),
                         std::get<4>(std::forward<T>(t)));}
    };
    }

    template<class F, class T>
    auto tuple_dispatch(F&& f, T&& t) 
        -> decltype(detail::tuple_dispatch<std::tuple_size<typename std::decay<T>::type>::value>::call(std::forward<F>(f), std::forward<T>(t))) {
        return      detail::tuple_dispatch<std::tuple_size<typename std::decay<T>::type>::value>::call(std::forward<F>(f), std::forward<T>(t));
    }

    struct as_tuple {
        auto operator()() 
            -> decltype(std::make_tuple()) {
            return      std::make_tuple();}
        template<class AsTupleNext>
        auto operator()(AsTupleNext x) 
            -> decltype(std::make_tuple(std::move(x))) {
            return      std::make_tuple(std::move(x));}
        template<
            class AsTupleNext1, 
            class AsTupleNext2>
        auto operator()(
            AsTupleNext1 x1,
            AsTupleNext2 x2) 
            -> decltype(std::make_tuple(
                std::move(x1),
                std::move(x2))) {
            return      std::make_tuple(
                std::move(x1),
                std::move(x2));}
        template<
            class AsTupleNext1, 
            class AsTupleNext2, 
            class AsTupleNext3>
        auto operator()(
            AsTupleNext1 x1,
            AsTupleNext2 x2,
            AsTupleNext3 x3) 
            -> decltype(std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3))) {
            return      std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3));}
        template<
            class AsTupleNext1, 
            class AsTupleNext2, 
            class AsTupleNext3, 
            class AsTupleNext4>
        auto operator()(
            AsTupleNext1 x1,
            AsTupleNext2 x2,
            AsTupleNext3 x3,
            AsTupleNext4 x4) 
            -> decltype(std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3),
                std::move(x4))) {
            return      std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3),
                std::move(x4));}
        template<
            class AsTupleNext1, 
            class AsTupleNext2, 
            class AsTupleNext3, 
            class AsTupleNext4, 
            class AsTupleNext5>
        auto operator()(
            AsTupleNext1 x1,
            AsTupleNext2 x2,
            AsTupleNext3 x3,
            AsTupleNext4 x4,
            AsTupleNext5 x5) 
            -> decltype(std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3),
                std::move(x4),
                std::move(x5))) {
            return      std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3),
                std::move(x4),
                std::move(x5));}

        auto operator()() const
            -> decltype(std::make_tuple()) {
            return      std::make_tuple();}
        template<class AsTupleNext>
        auto operator()(AsTupleNext x) const
            -> decltype(std::make_tuple(std::move(x))) {
            return      std::make_tuple(std::move(x));}
        template<
            class AsTupleNext1, 
            class AsTupleNext2>
        auto operator()(
            AsTupleNext1 x1,
            AsTupleNext2 x2) const
            -> decltype(std::make_tuple(
                std::move(x1),
                std::move(x2))) {
            return      std::make_tuple(
                std::move(x1),
                std::move(x2));}
        template<
            class AsTupleNext1, 
            class AsTupleNext2, 
            class AsTupleNext3>
        auto operator()(
            AsTupleNext1 x1,
            AsTupleNext2 x2,
            AsTupleNext3 x3) const
            -> decltype(std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3))) {
            return      std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3));}
        template<
            class AsTupleNext1, 
            class AsTupleNext2, 
            class AsTupleNext3, 
            class AsTupleNext4>
        auto operator()(
            AsTupleNext1 x1,
            AsTupleNext2 x2,
            AsTupleNext3 x3,
            AsTupleNext4 x4) const
            -> decltype(std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3),
                std::move(x4))) {
            return      std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3),
                std::move(x4));}
        template<
            class AsTupleNext1, 
            class AsTupleNext2, 
            class AsTupleNext3, 
            class AsTupleNext4, 
            class AsTupleNext5>
        auto operator()(
            AsTupleNext1 x1,
            AsTupleNext2 x2,
            AsTupleNext3 x3,
            AsTupleNext4 x4,
            AsTupleNext5 x5) const
            -> decltype(std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3),
                std::move(x4),
                std::move(x5))) {
            return      std::make_tuple(
                std::move(x1),
                std::move(x2),
                std::move(x3),
                std::move(x4),
                std::move(x5));}
    };
#endif //RXCPP_USE_VARIADIC_TEMPLATES

    struct pass_through {
        template<class X>
        typename std::decay<X>::type operator()(X&& x) {return std::forward<X>(x);}
        template<class X>
        typename std::decay<X>::type operator()(X&& x) const {return std::forward<X>(x);}
    };

    struct pass_through_second {
        template<class X, class Y>
        typename std::decay<Y>::type operator()(X&& , Y&& y) {return std::forward<Y>(y);}
        template<class X, class Y>
        typename std::decay<Y>::type operator()(X&& , Y&& y) const {return std::forward<Y>(y);}
    };

    template<typename Function>
	class unwinder
	{
	public:
		~unwinder()
		{
			if (!!function)
			{
				try {
					(*function)();
				} catch (...) {
					std::unexpected();
				}
			}
		}

		explicit unwinder(Function* functionArg)
			: function(functionArg)
		{
		}

		void dismiss()
		{
			function = nullptr;
		}

	private:
		unwinder();
		unwinder(const unwinder&);
		unwinder& operator=(const unwinder&);

		Function* function;
	};

}}

#define RXCPP_UNWIND(Name, Function) \
	RXCPP_UNWIND_EXPLICIT(uwfunc_ ## Name, Name, Function)

#define RXCPP_UNWIND_AUTO(Function) \
	RXCPP_UNWIND_EXPLICIT(RXCPP_MAKE_IDENTIFIER(uwfunc_), RXCPP_MAKE_IDENTIFIER(unwind_), Function)

#define RXCPP_UNWIND_EXPLICIT(FunctionName, UnwinderName, Function) \
	auto FunctionName = (Function); \
	rxcpp::util::unwinder<decltype(FunctionName)> UnwinderName(std::addressof(FunctionName))

#endif