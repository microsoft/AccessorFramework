// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <AccessorFramework/Accessor.h>

// Description
// An actor that takes two integers received on its two input ports and outputs the sum on its output port
//
class IntegerAdder : public AtomicAccessor
{
public:
    explicit IntegerAdder(const std::string& name);

    // Input Port Names
    static const char* LeftInput;
    static const char* RightInput;

    // Connected Output Port Names
    static const char* SumOutput;

private:
    void Fire() override;

    int m_latestLeftInput = 0;
    int m_latestRightInput = 0;
};