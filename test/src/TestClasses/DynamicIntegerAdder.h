// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef DYNAMICINTEGERADDER_H
#define DYNAMICINTEGERADDER_H

#include <AccessorFramework/Accessor.h>

// Description
// An actor that takes n integers received on its input ports and outputs the sum on its output port
//
class DynamicIntegerAdder : public AtomicAccessor
{
public:
    explicit DynamicIntegerAdder(const std::string& name) :
        AtomicAccessor(name, {} /*inputPortNames*/, { SumOutput })
    {
        this->AddNextPort();
        this->AddNextPort();
    }

    static std::string GetInputPortName(int portIndex)
    {
        std::ostringstream oss;
        oss << InputPrefix << "-" << portIndex;
        return oss.str();
    }

    // Connected Output Port Names
    static const char* SumOutput;

private:
    void AddNextPort()
    {
        int portIndex = this->m_nextPortIndex++;
        this->m_latestInputs.resize(this->m_nextPortIndex);
        std::string portName = this->GetInputPortName(portIndex);
        this->AddInputPort(portName);
        this->AddHandler(portIndex);
    }

    void AddHandler(int portIndex)
    {
        std::string portName = GetInputPortName(portIndex);
        this->AddInputHandler(portName,
            [this, portIndex](IEvent* event)
            {
                this->m_latestInputs[portIndex] = static_cast<Event<int>*>(event)->payload;
            });
    }

    int CalculateSum()
    {
        int sum = 0;
        for (int i : this->m_latestInputs)
        {
            sum += i;
        }

        return sum;
    }

    void Fire() override
    {
        int sum = this->CalculateSum();
        this->SendOutput(SumOutput, std::make_shared<Event<int>>(sum));
        this->AddNextPort();
    }

    std::vector<int> m_latestInputs;
    static const char* InputPrefix;
    int m_nextPortIndex = 0;
};

const char* DynamicIntegerAdder::InputPrefix = "Input-";
const char* DynamicIntegerAdder::SumOutput = "SumOutput";

#endif // DYNAMICINTEGERADDER_H