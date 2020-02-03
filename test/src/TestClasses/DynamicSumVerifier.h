// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef DYNAMICSUMVERIFIER_H
#define DYNAMICSUMVERIFIER_H

#include <AccessorFramework/Accessor.h>

// Description
// An actor that verifies the DynamicIntegerAdder's output when used in DynamicIntegerAdderHost
//
class DynamicSumVerifier : public AtomicAccessor
{
public:
    DynamicSumVerifier(const std::string& name, std::shared_ptr<int> latestSum, std::shared_ptr<bool> error) :
        AtomicAccessor(name, { SumInput }),
        m_nextAddition(0),
        m_expectedSum(0),
        m_latestSum(latestSum),
        m_error(error)
    {
        this->AddInputHandler(
            SumInput,
            [this](IEvent* inputSumEvent)
            {
                int actualSum = static_cast<Event<int>*>(inputSumEvent)->payload;
                *(this->m_latestSum) = actualSum;
                this->VerifySum(actualSum);
                this->CalculateNextExpectedSum();
            });
    }

    static constexpr char* SumInput = "Sum";

private:
    void VerifySum(int actualSum) const
    {
        *(this->m_error) |= !(actualSum == this->m_expectedSum);
        if (*(this->m_error))
        {
            std::cerr << "FAILURE: received actual sum of " << actualSum << ", but expected " << this->m_expectedSum << std::endl;
        }
        else
        {
            std::cout << "SUCCESS: actual sum of " << actualSum << " matched expected" << std::endl;
        }
    }

    void CalculateNextExpectedSum()
    {
        ++(this->m_nextAddition);
        this->m_expectedSum = *(this->m_latestSum) + this->m_nextAddition;
    }

    int m_nextAddition;
    int m_expectedSum;
    std::shared_ptr<int> m_latestSum;
    std::shared_ptr<bool> m_error;
};

#endif // DYNAMICSUMVERIFIER_H