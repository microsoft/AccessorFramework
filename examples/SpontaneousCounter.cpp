// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SpontaneousCounter.h"

const char* SpontaneousCounter::CounterValueOutput = "CounterValue";

SpontaneousCounter::SpontaneousCounter(const std::string& name, int intervalInMilliseconds) :
    AtomicAccessor(name, {}, {}, { CounterValueOutput }),
    m_initialized(false),
    m_intervalInMilliseconds(intervalInMilliseconds),
    m_callbackId(0),
    m_count(0)
{
}

void SpontaneousCounter::Initialize()
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