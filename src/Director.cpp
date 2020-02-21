// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Director.h"
#include "PrintDebug.h"
#include <algorithm>
#include <cassert>
#include <ctime>
#include <set>
#include <thread>

static const long long DefaultNextExecutionTime = LLONG_MAX;

Director::Director() :
    m_nextCallbackId(0),
    m_currentLogicalTime(PosixUtcInMilliseconds()),
    m_startTime(this->m_currentLogicalTime),
    m_nextScheduledExecutionTime(DefaultNextExecutionTime),
    m_executionResult(nullptr),
    m_currentExecutionCancellationToken(nullptr)
{
}

Director::~Director()
{
    this->Reset();
}

int Director::ScheduleCallback(
    std::function<void()> callback,
    int delayInMilliseconds,
    bool isPeriodic,
    int priority)
{
    ScheduledCallback newCallback{ callback, delayInMilliseconds, isPeriodic, priority };
    newCallback.nextExecutionTimeInMilliseconds = this->m_currentLogicalTime + delayInMilliseconds;
    int newCallbackId = this->m_nextCallbackId++;
    this->m_scheduledCallbacks[newCallbackId] = newCallback;
    this->QueueScheduledCallback(newCallbackId);
    if (this->m_nextScheduledExecutionTime > newCallback.nextExecutionTimeInMilliseconds)
    {
        this->CancelNextExecution();
        this->m_nextScheduledExecutionTime = newCallback.nextExecutionTimeInMilliseconds;
        this->ScheduleNextExecution();
    }

    return newCallbackId;
}
 
void Director::ClearScheduledCallback(int callbackId)
{
    for (auto it = this->m_callbackQueue.begin(); it != this->m_callbackQueue.end(); ++it)
    {
        if (*it == callbackId)
        {
            this->m_callbackQueue.erase(it);
            break;
        }
    }

    this->RemoveScheduledCallbackFromMap(callbackId);
    if (this->NeedsReset())
    {
        this->Reset();
    }
}

void Director::HandlePriorityUpdate(int oldPriority, int newPriority)
{
    for (auto& i : this->m_scheduledCallbacks)
    {
        if (i.second.priority == oldPriority)
        {
            i.second.priority = newPriority;
            for (auto it = this->m_callbackQueue.begin(); it != this->m_callbackQueue.end(); ++it)
            {
                if (*it == i.first)
                {
                    this->m_callbackQueue.erase(it);
                    this->QueueScheduledCallback(i.first);
                    break;
                }
            }
        }
    }
}

void Director::Execute(std::shared_ptr<CancellationToken> executionCancellationToken, int numberOfIterations)
{
    if (this->m_currentExecutionCancellationToken.get() == nullptr || this->m_currentExecutionCancellationToken->IsCanceled())
    {
        this->ScheduleNextExecution();
    }

    auto executionResult = this->m_executionResult;
    int currentIteration = 0;
    while (!executionCancellationToken->IsCanceled() && executionResult->valid() && (numberOfIterations == 0 || currentIteration < numberOfIterations))
    {
        PRINT_DEBUG("-----NEXT ROUND-----");
        bool wasCanceled = executionResult->get();
        if (wasCanceled && this->m_executionResult.get() == nullptr)
        {
            break;
        }
        else
        {
            executionResult = this->m_executionResult;
            if (numberOfIterations != 0)
            {
                ++currentIteration;
            }
        }
    }

    this->CancelNextExecution();
}

long long Director::GetNextQueuedExecutionTime() const
{
    if (this->m_callbackQueue.empty())
    {
        return DefaultNextExecutionTime;
    }
    else
    {
        return this->m_scheduledCallbacks.at(this->m_callbackQueue.front()).nextExecutionTimeInMilliseconds;
    }
}

void Director::ScheduleNextExecution()
{
    long long executionDelayInMilliseconds = std::max<long long>(this->m_nextScheduledExecutionTime - PosixUtcInMilliseconds(), 0LL);
    this->m_currentExecutionCancellationToken = std::make_shared<CancellationToken>();
    auto executionPromise = std::make_unique<std::promise<bool>>();
    this->m_executionResult = std::make_shared<std::future<bool>>(executionPromise->get_future());

    bool retry = false;
    do
    {
        retry = false;
        try
        {
            std::thread executionThread(&Director::ExecuteInternal, this->shared_from_this(), executionDelayInMilliseconds, std::move(executionPromise), this->m_currentExecutionCancellationToken);
            executionThread.detach();
        }
        catch (const std::system_error& e)
        {
            if (e.code() == std::errc::resource_unavailable_try_again)
            {
                retry = true;
            }
            else
            {
                throw;
            }
        }
    } while (retry);
}

void Director::CancelNextExecution()
{
    if (this->m_currentExecutionCancellationToken.get() != nullptr)
    {
        this->m_currentExecutionCancellationToken->Cancel();
        this->m_currentExecutionCancellationToken = nullptr;
    }
}

void Director::ExecuteInternal(long long executionDelayInMilliseconds, std::unique_ptr<std::promise<bool>> executionPromise, std::shared_ptr<CancellationToken> cancellationToken)
{
    if (executionDelayInMilliseconds != 0LL)
    {
        auto executionDelay = std::chrono::milliseconds(executionDelayInMilliseconds);
        cancellationToken->SleepFor(executionDelay);
    }

    while (!cancellationToken->IsCanceled() && !this->NeedsReset() && this->m_nextScheduledExecutionTime <= PosixUtcInMilliseconds())
    {
        try
        {
            this->ExecuteCallbacks();
        }
        catch (...)
        {
            executionPromise->set_exception(std::current_exception());
            return;
        }

        if (!(cancellationToken->IsCanceled()))
        {
            this->m_nextScheduledExecutionTime = this->GetNextQueuedExecutionTime();
        }
    }

    bool executionWasCanceled = cancellationToken->IsCanceled();
    if (!executionWasCanceled)
    {
        if (this->NeedsReset())
        {
            this->Reset();
        }

        this->ScheduleNextExecution();
    }

    executionPromise->set_value(executionWasCanceled);
}

bool Director::ScheduledCallbackExistsInMap(int scheduledCallbackId) const
{
    return (this->m_scheduledCallbacks.find(scheduledCallbackId) != this->m_scheduledCallbacks.end());
}

void Director::RemoveScheduledCallbackFromMap(int scheduledCallbackId)
{
    this->m_scheduledCallbacks.erase(scheduledCallbackId);
}

// Callbacks are sorted by execution time, then by accessor priority, then by callback ID (i.e. instantiation order)
void Director::QueueScheduledCallback(int newCallbackId)
{
    if (this->m_callbackQueue.empty())
    {
        this->m_callbackQueue.push_back(newCallbackId);
        return;
    }

    size_t callbackQueueLength = this->m_callbackQueue.size();
    size_t insertionIndex = 0;
    while (insertionIndex < callbackQueueLength)
    {
        const int queuedCallbackId(this->m_callbackQueue.at(insertionIndex));

        // Sort Level 1: Execution Time
        if (this->m_scheduledCallbacks.at(newCallbackId).nextExecutionTimeInMilliseconds <
            this->m_scheduledCallbacks.at(queuedCallbackId).nextExecutionTimeInMilliseconds)
        {
            // New callback's execution time is sooner than queued callback's execution time
            break;
        }
        else if (this->m_scheduledCallbacks.at(newCallbackId).nextExecutionTimeInMilliseconds >
            this->m_scheduledCallbacks.at(queuedCallbackId).nextExecutionTimeInMilliseconds)
        {
            // New callback's execution time is later than queued callback's execution time
            ++insertionIndex;
        }
        else
        {
            // Execution times are equal
            // Sort Level 2: Accessor Priority
            if (this->m_scheduledCallbacks.at(newCallbackId).priority < this->m_scheduledCallbacks.at(queuedCallbackId).priority)
            {
                // New callback's priority is higher than queued callback's priority
                break;
            }
            else if (this->m_scheduledCallbacks.at(newCallbackId).priority > this->m_scheduledCallbacks.at(queuedCallbackId).priority)
            {
                // New callback's priority is lower than queued callback's priority
                ++insertionIndex;
            }
            else
            {
                // Priorities are equal
                // Sort Level 3: Callback ID
                if (newCallbackId < queuedCallbackId)
                {
                    // new callback's ID is lower than queued callback's ID
                    break;
                }
                else if (newCallbackId > queuedCallbackId)
                {
                    // new callback's ID is higher than queued callback's ID
                    ++insertionIndex;
                }
                else
                {
                    // Trying to queue two callbacks with the same ID should never happen.
                    // If it does, it is indicative of a bug in the Director.
                    assert(queuedCallbackId != newCallbackId);
                }
            }
        }
    }

    assert(insertionIndex <= callbackQueueLength);
    auto insertionIt = (insertionIndex == callbackQueueLength ? this->m_callbackQueue.end() : this->m_callbackQueue.begin() + insertionIndex);
    this->m_callbackQueue.insert(insertionIt, newCallbackId);
}

void Director::ExecuteCallbacks()
{
    this->m_currentLogicalTime = this->m_nextScheduledExecutionTime;
    PRINT_DEBUG("Current logical time is t + %lld ms", this->m_currentLogicalTime - this->m_startTime);
    while (!this->m_callbackQueue.empty() && this->GetNextQueuedExecutionTime() <= this->m_nextScheduledExecutionTime)
    {
        int callbackId = this->m_callbackQueue.front();
        this->m_callbackQueue.erase(this->m_callbackQueue.begin());
        try
        {
            this->m_scheduledCallbacks.at(callbackId).callbackFunction();
        }
        catch (const std::exception& e)
        {
            this->RemoveScheduledCallbackFromMap(callbackId);
            throw;
        }

        if (this->ScheduledCallbackExistsInMap(callbackId))
        {
            // Callback did not cancel itself - either reschedule (if periodic) or remove
            if (this->m_scheduledCallbacks.at(callbackId).isPeriodic)
            {
                // Schedule next occurrence of this periodic callback
                this->m_scheduledCallbacks.at(callbackId).nextExecutionTimeInMilliseconds +=
                    this->m_scheduledCallbacks.at(callbackId).delayInMilliseconds;
                this->QueueScheduledCallback(callbackId);
            }
            else
            {
                this->RemoveScheduledCallbackFromMap(callbackId);
            }
        }
    }
}

bool Director::NeedsReset() const
{
    return (this->m_callbackQueue.empty() || this->m_scheduledCallbacks.empty());
}

void Director::Reset()
{
    this->CancelNextExecution();
    this->m_callbackQueue.clear();
    this->m_scheduledCallbacks.clear();
    this->m_nextCallbackId = 0;
    this->m_currentLogicalTime = PosixUtcInMilliseconds();
    this->m_startTime = this->m_currentLogicalTime;
    PRINT_DEBUG("Resetting current logical time to 0");
    this->m_nextScheduledExecutionTime = DefaultNextExecutionTime;
}

// Returns number of milliseconds elapsed since 01/01/1970 00:00:00 UTC
long long Director::PosixUtcInMilliseconds()
{
    struct timespec now;
    int err = timespec_get(&now, TIME_UTC);
    if (err == 0)
    {
        return -1;
    }

    return (((long long)now.tv_sec) * 1000LL) + (((long long)now.tv_nsec) / 1000000LL);
}