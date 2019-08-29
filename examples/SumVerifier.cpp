// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <iostream>
#include "SumVerifier.h"

const char* SumVerifier::SumInput = "Sum";

SumVerifier::SumVerifier(const std::string& name) :
    AtomicAccessor(name, { SumInput }),
    m_expectedSum(0)
{
    this->AddInputHandler(
        SumInput,
        [this](IEvent* inputSumEvent)
        {
            int actualSum = static_cast<Event<int>*>(inputSumEvent)->payload;
            this->VerifySum(actualSum);
            this->CalculateNextExpectedSum();
        });
}

void SumVerifier::VerifySum(int actualSum) const
{
    if (actualSum == this->m_expectedSum)
    {
        std::cout << "SUCCESS: actual sum of " << actualSum << " matched expected" << std::endl;
    }
    else
    {
        std::cout << "FAILURE: received actual sum of " << actualSum << ", but expected expected " << this->m_expectedSum << std::endl;
    }
}

void SumVerifier::CalculateNextExpectedSum()
{
    this->m_expectedSum += 2;
}