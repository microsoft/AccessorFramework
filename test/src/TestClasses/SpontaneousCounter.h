// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef SPONTANEOUSCOUNTER_H
#define SPONTANEOUSCOUNTER_H

#include <AccessorFramework/Accessor.h>

// Description
// An actor that increments a counter and outputs the value of the counter at regular intervals
//
class SpontaneousCounter : public AtomicAccessor
{
public:
    SpontaneousCounter(const std::string& name, int intervalInMilliseconds) :
        AtomicAccessor(name, {}, {}, { CounterValueOutput }),
        m_initialized(false),
        m_intervalInMilliseconds(intervalInMilliseconds),
        m_callbackId(0),
        m_count(0)
    {
    }

    static constexpr char* CounterValueOutput = "CounterValue";

private:
    void Initialize() override
    {
        this->m_callbackId = this->ScheduleCallback(
            [this]()
            {
                this->SendOutput(CounterValueOutput, std::make_shared<Event<int>>(this->m_count));
                ++this->m_count;
            },
            m_intervalInMilliseconds,
            true /*repeat*/);

        this->m_initialized = true;
    }

    bool m_initialized;
    int m_intervalInMilliseconds;
    int m_callbackId;
    int m_count;
};

#endif // SPONTANEOUSCOUNTER_H