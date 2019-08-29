// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <AccessorFramework/Accessor.h>

// Description
// An actor that verifies the IntegerAdder's output
//
class SumVerifier : public AtomicAccessor
{
public:
    SumVerifier(const std::string& name);

    static const char* SumInput;

private:
    void VerifySum(int actualSum) const;
    void CalculateNextExpectedSum();

    int m_expectedSum;
};