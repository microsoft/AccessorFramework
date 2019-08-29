// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef CANCELLATION_TOKEN_H
#define CANCELLATION_TOKEN_H

#include <atomic>
#include <chrono>
#include <mutex>

// Description
// The CancellationToken is a wrapper for an atomic_bool with some convenience methods. This clarifies the intent when
// the Director cancels scheduled execution tasks.
//
class CancellationToken
{
public:
    CancellationToken() :
        m_isCanceled(false)
    {
        this->m_timedLock.lock();
    }

    ~CancellationToken()
    {
        if (!(this->m_isCanceled.load()))
        {
            this->m_timedLock.unlock();
        }
    }

    void Cancel()
    {
        if (!(this->IsCanceled()))
        {
            this->m_isCanceled.store(true);
            this->m_timedLock.unlock();
        }
    }

    bool IsCanceled() const
    {
        return this->m_isCanceled.load();
    }

    template<class Rep, class Period>
    void SleepFor(const std::chrono::duration<Rep, Period>& duration)
    {
        using namespace std::literals::chrono_literals;

        // sleep in 1-hour chunks to avoid overflow error from very large duration
        auto timeLeft = duration;
        auto sleepInterval = 1h;
        while (timeLeft > std::chrono::duration<Rep, Period>::zero())
        {
            auto timeToSleep = std::min<std::common_type_t<std::chrono::duration<Rep, Period>, std::chrono::hours>>(timeLeft, sleepInterval);
            if (this->m_timedLock.try_lock_for(timeToSleep))
            {
                this->m_timedLock.unlock();
                break;
            }
            else
            {
                timeLeft -= timeToSleep;
            }
        }
    }

private:
    std::atomic_bool m_isCanceled;
    std::timed_mutex m_timedLock;
};

#endif // CANCELLATION_TOKEN_H
