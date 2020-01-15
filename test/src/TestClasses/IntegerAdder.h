// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef INTEGERADDER_H
#define INTEGERADDER_H

#include <AccessorFramework/Accessor.h>

// Description
// An actor that takes two integers received on its two input ports and outputs the sum on its output port
//
class IntegerAdder : public AtomicAccessor
{
public:
    explicit IntegerAdder(const std::string& name) :
        AtomicAccessor(name, { LeftInput, RightInput }, { SumOutput })
    {
        this->AddInputHandler(LeftInput,
            [this](IEvent* event)
            {
                this->m_latestLeftInput = static_cast<Event<int>*>(event)->payload;
            });

        this->AddInputHandler(RightInput,
            [this](IEvent* event)
            {
                this->m_latestRightInput = static_cast<Event<int>*>(event)->payload;
            });
    }

    // Input Port Names
    static const char* LeftInput;
    static const char* RightInput;

    // Connected Output Port Names
    static const char* SumOutput;

private:
    void Fire() override
    {
        this->SendOutput(SumOutput, std::make_shared<Event<int>>(this->m_latestLeftInput + this->m_latestRightInput));
    }

    int m_latestLeftInput = 0;
    int m_latestRightInput = 0;
};

const char* IntegerAdder::LeftInput = "LeftInput";
const char* IntegerAdder::RightInput = "RightInput";
const char* IntegerAdder::SumOutput = "SumOutput";

#endif // INTEGERADDER_H