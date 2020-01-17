// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef DYNAMICSUMVERIFIERHOST_H
#define DYNAMICSUMVERIFIERHOST_H

#include <chrono>
#include <AccessorFramework/Host.h>
#include "DynamicIntegerAdder.h"
#include "DynamicSumVerifier.h"
#include "SpontaneousCounter.h"

class DynamicSumVerifierHost : public Host
{
public:
    explicit DynamicSumVerifierHost(const std::string& name, std::shared_ptr<int> latestSum, std::shared_ptr<bool> error) : Host(name)
    {
        using namespace std::chrono_literals;
        this->AddChild(std::make_unique<DynamicIntegerAdder>(a1));
        this->AddChild(std::make_unique<DynamicSumVerifier>(v1, latestSum, error));
        this->ConnectChildren(a1, DynamicIntegerAdder::SumOutput, v1, DynamicSumVerifier::SumInput);
    }

private:

    void Initialize() override
    {
        this->ScheduleCallback(
            [this]()
            {
                int counterIndex = this->m_counterIndex++;
                std::string counterName = this->GetCounterName(counterIndex);
                this->AddChild(std::make_unique<SpontaneousCounter>(counterName, this->m_spontaneousInterval.count()));
                std::string adderInputPortName = DynamicIntegerAdder::GetInputPortName(counterIndex);
                this->ConnectChildren(counterName, SpontaneousCounter::CounterValueOutput, a1, adderInputPortName);
                this->ChildrenChanged();
            },
            this->m_spontaneousInterval.count(),
            true /*repeat*/);
    }

    std::string GetCounterName(int counterIndex)
    {
        std::ostringstream oss;
        oss << this->spontaneousCounterPrefix << "-" << counterIndex;
        return oss.str();
    }

    const std::chrono::milliseconds m_spontaneousInterval = std::chrono::milliseconds(1000);
    const std::string spontaneousCounterPrefix = "SpontaneousCounter-";
    const std::string a1 = "DynamicIntegerAdder";
    const std::string v1 = "SumVerifier";
    int m_counterIndex = 0;
};

#endif // DYNAMICSUMVERIFIERHOST_H