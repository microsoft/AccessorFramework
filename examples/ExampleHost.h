// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <AccessorFramework/Host.h>

class ExampleHost : public Host
{
public:
    explicit ExampleHost(const std::string& name);
    void AdditionalSetup() override;

private:
    const std::string s1 = "SpontaneousCounterOne";
    const std::string s2 = "SpontaneousCounterTwo";
    const std::string a1 = "IntegerAdder";
    const std::string v1 = "SumVerifier";
};
