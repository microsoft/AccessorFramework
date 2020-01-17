// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef SUMVERIFIER_H
#define SUMVERIFIER_H

#include <AccessorFramework/Accessor.h>

// Description
// An actor that verifies the IntegerAdder's output
//
class SumVerifier : public AtomicAccessor
{
public:
    SumVerifier(const std::string& name, std::shared_ptr<int> latestSum, std::shared_ptr<bool> error) :
        AtomicAccessor(name, { SumInput }),
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
        this->m_expectedSum += 2;
    }

    int m_expectedSum;
    std::shared_ptr<int> m_latestSum;
    std::shared_ptr<bool> m_error;
};

#endif // SUMVERIFIER_H