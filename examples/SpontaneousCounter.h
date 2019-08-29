// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <AccessorFramework/Accessor.h>

// Description
// An actor that increments a counter and outputs the value of the counter at regular intervals
//
class SpontaneousCounter : public AtomicAccessor
{
public:
    SpontaneousCounter(const std::string& name, int intervalInMilliseconds);

    static const char* CounterValueOutput;

private:
    void Initialize() override;

    bool m_initialized;
    int m_intervalInMilliseconds;
    int m_callbackId;
    int m_count;
};