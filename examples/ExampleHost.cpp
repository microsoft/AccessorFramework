// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <chrono>
#include "ExampleHost.h"
#include "IntegerAdder.h"
#include "SpontaneousCounter.h"
#include "SumVerifier.h"

ExampleHost::ExampleHost(const std::string& name) : Host(name)
{
    using namespace std::chrono_literals;
    auto spontaneousInterval = 1000ms;
    this->AddChild(std::make_unique<SpontaneousCounter>(s1, spontaneousInterval.count()));
    this->AddChild(std::make_unique<SpontaneousCounter>(s2, spontaneousInterval.count()));
    this->AddChild(std::make_unique<IntegerAdder>(a1));
    this->AddChild(std::make_unique<SumVerifier>(v1));
}

void ExampleHost::AdditionalSetup()
{
    // This could also be done in the constructor, but we do it here to illustrate the additional setup feature
    // Connect s1's output to a1's left input
    this->ConnectChildren(s1, SpontaneousCounter::CounterValueOutput, a1, IntegerAdder::LeftInput);

    // Connect s2's output to a1's right input
    this->ConnectChildren(s2, SpontaneousCounter::CounterValueOutput, a1, IntegerAdder::RightInput);

    // Connect a1's output to v1's input
    this->ConnectChildren(a1, IntegerAdder::SumOutput, v1, SumVerifier::SumInput);
}