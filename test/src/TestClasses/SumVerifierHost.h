// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef SUMVERIFIERHOST_H
#define SUMVERIFIERHOST_H

#include <chrono>
#include <AccessorFramework/Host.h>
#include "IntegerAdder.h"
#include "SpontaneousCounter.h"
#include "SumVerifier.h"

class SumVerifierHost : public Host
{
public:
    explicit SumVerifierHost(const std::string& name, std::shared_ptr<int> latestSum, std::shared_ptr<bool> error) : Host(name)
    {
        using namespace std::chrono_literals;
        auto spontaneousInterval = 1000ms;
        this->AddChild(std::make_unique<SpontaneousCounter>(s1, spontaneousInterval.count()));
        this->AddChild(std::make_unique<SpontaneousCounter>(s2, spontaneousInterval.count()));
        this->AddChild(std::make_unique<IntegerAdder>(a1));
        this->AddChild(std::make_unique<SumVerifier>(v1, latestSum, error));
    }

    void AdditionalSetup() override
    {
        // This could also be done in the constructor, but we do it here to illustrate the additional setup feature
        // Connect s1's output to a1's left input
        this->ConnectChildren(s1, SpontaneousCounter::CounterValueOutput, a1, IntegerAdder::LeftInput);

        // Connect s2's output to a1's right input
        this->ConnectChildren(s2, SpontaneousCounter::CounterValueOutput, a1, IntegerAdder::RightInput);

        // Connect a1's output to v1's input
        this->ConnectChildren(a1, IntegerAdder::SumOutput, v1, SumVerifier::SumInput);
    }

private:
    const std::string s1 = "SpontaneousCounterOne";
    const std::string s2 = "SpontaneousCounterTwo";
    const std::string a1 = "IntegerAdder";
    const std::string v1 = "SumVerifier";
};

#endif // SUMVERIFIERHOST_H