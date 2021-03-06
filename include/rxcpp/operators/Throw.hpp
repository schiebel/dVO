 // Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once
#include "../rx-operators.hpp"

#if !defined(CPPRX_RX_OPERATORS_THROW_HPP)
#define CPPRX_RX_OPERATORS_THROW_HPP

namespace rxcpp
{

    namespace detail
    {
        template<class T>
        class ThrowObservable : public Producer<ThrowObservable<T>, T>
        {
            typedef ThrowObservable<T> This;
            typedef std::shared_ptr<This> Parent;

            std::exception_ptr exception;
            Scheduler::shared scheduler;

            class _ : public Sink<_, T>
            {
                Parent parent;

            public:
                typedef Sink<_, T> SinkBase;

                _(Parent parent, std::shared_ptr < Observer < T >> observer, Disposable cancel) :
                    SinkBase(std::move(observer), std::move(cancel)),
                    parent(parent)
                {
                }

                Disposable Run()
                {
                    auto local = parent;
                    auto that = this->shared_from_this();
                    return parent->scheduler->Schedule(
                        [=](Scheduler::shared) -> Disposable {
                            that->SinkBase::observer->OnError(local->exception);
                            that->SinkBase::Dispose();
                            return Disposable::Empty();
                    });
                }
            };

            typedef Producer<This, T> ProducerBase;
        public:

            ThrowObservable(std::exception_ptr exception, Scheduler::shared scheduler) :
                ProducerBase([](Parent parent, std::shared_ptr < Observer < T >> observer, Disposable && cancel, typename ProducerBase::SetSink setSink) -> Disposable
                {
                    auto sink = std::shared_ptr<ThrowObservable::_>(new ThrowObservable::_(parent, observer, std::move(cancel)));
                    setSink(sink->GetDisposable());
                    return sink->Run();
                }),
                exception(exception),
                scheduler(scheduler)
            {
                if (!scheduler)
                {
                    this->scheduler = std::make_shared<CurrentThreadScheduler>();
                }
            }
        };

        struct throw_ptr_tag{};
        struct throw_instance_tag{};

        template <class T>
        std::shared_ptr<Observable<T>> make_throw(
            throw_ptr_tag&&,
            std::exception_ptr exception,
            Scheduler::shared scheduler = nullptr
            )
        {
            return std::make_shared<detail::ThrowObservable<T>>(std::move(exception), std::move(scheduler));
        }

        template <class T, class E>
        std::shared_ptr<Observable<T>> make_throw(
            throw_instance_tag&&,
            E e,
            Scheduler::shared scheduler = nullptr
            )
        {
            std::exception_ptr exception;
            try {throw e;} catch(...) {exception = std::current_exception();}
            return std::make_shared<detail::ThrowObservable<T>>(std::move(exception), std::move(scheduler));
        }
    }
    template <class T, class E>
    std::shared_ptr<Observable<T>> Throw(
        E e,
        Scheduler::shared scheduler = nullptr
        )
    {
        return detail::make_throw<T>(typename std::conditional<std::is_same<std::exception_ptr, typename std::decay<E>::type>::value, detail::throw_ptr_tag, detail::throw_instance_tag>::type(), std::forward<E>(e), std::move(scheduler));
    }
}

#endif
