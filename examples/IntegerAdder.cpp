// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "IntegerAdder.h"

const char* IntegerAdder::LeftInput = "LeftInput";
const char* IntegerAdder::RightInput = "RightInput";
const char* IntegerAdder::SumOutput = "SumOutput";

IntegerAdder::IntegerAdder(const std::string& name) :
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

void IntegerAdder::Fire()
{
    this->SendOutput(SumOutput, std::make_shared<Event<int>>(this->m_latestLeftInput + this->m_latestRightInput));
}