// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef DIRECTOR_H
#define DIRECTOR_H

#include "CancellationToken.h"
#include <climits>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Description
// The Director manages and executes the accessor model's global callback queue. There is only one director per model.
// The Director prioritizes callbacks first by next execution time, then by the calling accessor's priority, and lastly
// by a monotonically increasing callback ID. This callback ID enables the calling accessor to cancel the scheduled
// callback, and it also ensures that two callbacks scheduled in a given order by a single accessor will execute in the
// order in which they were scheduled. The execution time is calculated using a logical clock loosely tied to physical
// time. However, while physical time is continuous, logical clocks are discrete; that is, the time on the logical clock
// "jumps" instantaneously from one time to the next when the callbacks on the queue are executed. This allows the queued
// callbacks to be executed synchronously while making it appear to the accessors as if they execute atomically and
// concurrently, enabling asynchronous yet coordinated reactions without explicit thread management or locks.
//
class Director
{
public:
    Director();
    ~Director();
    int ScheduleCallback(
        std::function<void()> callback,
        int delayInMilliseconds,
        bool isPeriodic = false,
        int priority = INT_MAX);

    void ClearScheduledCallback(int callbackId);
    void HandlePriorityUpdate(int oldPriority, int newPriority);
    void Execute(int numberOfIterations = 0);
    void StopExecution();

private:
    class ScheduledCallback
    {
    public:
        std::function<void()> callbackFunction = nullptr;
        int delayInMilliseconds = 0;
        bool isPeriodic = false;
        int priority = INT_MAX;
        long long nextExecutionTimeInMilliseconds = 0;
    };

    long long GetNextQueuedExecutionTime() const;
    void ScheduleNextExecution();
    void ExecuteInternal(
        long long executionDelayInMilliseconds,
        std::unique_ptr<std::promise<bool>> executionPromise,
        std::shared_ptr<CancellationToken> cancellationToken);

    bool ScheduledCallbackExistsInMap(int scheduledCallbackId) const;
    void RemoveScheduledCallbackFromMap(int scheduledCallbackId);
    void QueueScheduledCallback(int newCallbackId);
    void ExecuteCallbacks();
    bool NeedsReset() const;
    void Reset();

    int m_nextCallbackId;
    std::map<int, ScheduledCallback> m_scheduledCallbacks;
    std::vector<int> m_callbackQueue;
    long long m_currentLogicalTime;
    long long m_startTime;
    long long m_nextScheduledExecutionTime;
    std::shared_ptr<std::future<bool>> m_executionResult;
    std::shared_ptr<CancellationToken> m_executionCancellationToken;

    static long long PosixUtcInMilliseconds();
};

#endif // DIRECTOR_H
