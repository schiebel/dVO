// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#include "util.hpp"
#include "linq_cursor.hpp"

namespace cpplinq 
{
    namespace detail 
    {
        struct default_select_many_selector
        {
            template <class T1, class T2>
            auto operator()(T1&& t1, T2&& t2) const
            -> decltype(std::forward<T2>(t2))
            {
                return std::forward<T2>(t2);
            }
        };
    }

    // cur<T> -> (T -> cur<element_type>) -> cur<element_type>
    template <class Container1, class Fn, class Fn2>
    class linq_select_many
    {
        template <class T> static T instance(); // for type inference

        Container1      c1;
        Fn              fn;
        Fn2             fn2;

        typedef typename Container1::cursor Cur1;
        typedef decltype(from(instance<Fn>()(instance<Cur1>().get()))) Container2;
        typedef typename Container2::cursor Cur2;

    public:
        class cursor
        {
        public:
            typedef typename util::min_cursor_category<typename Cur1::cursor_category,
                                                       typename Cur2::cursor_category,
                                                       forward_cursor_tag>::type
                cursor_category;
            typedef typename Cur2::reference_type reference_type;
            typedef typename Cur2::element_type element_type;

        private:
            // TODO: we need to lazy eval somehow, but this feels wrong.
            Cur1                            cur1;
            dynamic_cursor<reference_type>  cur2;
            Fn                              fn;
            Fn2                             fn2;

        public:
            cursor(Cur1 cur1, const Fn& fn, const Fn2& fn2)
            : cur1(std::move(cur1)), fn(fn), fn2(fn2)
            {
                auto container2 = fn(cur1.get());
                cur2 = from(container2).get_cursor();
            }

            bool empty() const 
            {
                return cur2.empty();
            }

            void inc() 
            {
                cur2.inc();
                thunk();
            }

            reference_type get() const 
            {
                return fn2(cur1.get(), cur2.get());
            }

        private:
            void thunk() 
            {
                // refill cur2
                while (cur2.empty() && !cur1.empty()) {
                    cur1.inc();
                    if (cur1.empty()) 
                        break;

                    auto container2 = fn(cur1.get());
                    cur2 = from(container2).get_cursor();
                }
            }
        };

        linq_select_many(Container1 c1, Fn fn, Fn2 fn2) 
        : c1(std::move(c1)), fn(std::move(fn)), fn2(std::move(fn2))
        {
        }

        cursor get_cursor() const
        {
            return cursor(c1.get_cursor(), fn, fn2);
        }
    };
}



