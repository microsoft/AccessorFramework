// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "AtomicAccessorImpl.h"
#include "CompositeAccessorImpl.h"
#include "PrintDebug.h"

template<class Key>
static void SetSubtract(std::set<Key>& minuend, const std::set<Key>& subtrahend)
{
    for (const auto& element : subtrahend)
    {
        minuend.erase(element);
    }
}

AtomicAccessor::Impl::Impl(
    const std::string& name,
    AtomicAccessor* container,
    std::function<void(Accessor&)> initializeFunction,
    const std::vector<std::string>& inputPortNames,
    const std::vector<std::string>& connectedOutputPortNames,
    const std::vector<std::string>& spontaneousOutputPortNames,
    std::map<std::string, std::vector<AtomicAccessor::InputHandler>> inputHandlers,
    std::function<void(AtomicAccessor&)> fireFunction) :
        Accessor::Impl(name, container, initializeFunction, inputPortNames, connectedOutputPortNames),
        m_inputHandlers(inputHandlers),
        m_fireFunction(fireFunction),
        m_stateDependsOnInputPort(false)
{
    this->AddSpontaneousOutputPorts(spontaneousOutputPortNames);
}

bool AtomicAccessor::Impl::IsComposite() const
{
    return false;
}

std::vector<const InputPort*> AtomicAccessor::Impl::GetEquivalentPorts(const InputPort* inputPort) const
{
    if (this->m_forwardPrunedDependencies.empty() || this->GetNumberOfInputPorts() == 1 || this->GetNumberOfOutputPorts() == 0)
    {
        return this->GetInputPorts();
    }

    std::set<const InputPort*> equivalentPorts{};
    std::set<const OutputPort*> dependentPorts{};
    this->FindEquivalentPorts(inputPort, equivalentPorts, dependentPorts);
    return std::vector<const InputPort*>(equivalentPorts.begin(), equivalentPorts.end());
}

std::vector<const InputPort*> AtomicAccessor::Impl::GetInputPortDependencies(const OutputPort* outputPort) const
{
    const std::vector<const InputPort*>& allInputPorts = this->GetInputPorts();
    if (this->m_backwardPrunedDependencies.find(outputPort) == this->m_backwardPrunedDependencies.end())
    {
        return allInputPorts;
    }

    std::set<const InputPort*> inputPortDependencies(allInputPorts.begin(), allInputPorts.end());
    SetSubtract(inputPortDependencies, this->m_backwardPrunedDependencies.at(outputPort));
    return std::vector<const InputPort*>(inputPortDependencies.begin(), inputPortDependencies.end());
}

std::vector<const OutputPort*> AtomicAccessor::Impl::GetDependentOutputPorts(const InputPort* inputPort) const
{
    const std::vector<const OutputPort*>& allOutputPorts = this->GetOutputPorts();
    if (this->m_forwardPrunedDependencies.find(inputPort) == this->m_forwardPrunedDependencies.end())
    {
        return allOutputPorts;
    }

    std::set<const OutputPort*> dependentOutputPorts(allOutputPorts.begin(), allOutputPorts.end());
    SetSubtract(dependentOutputPorts, this->m_forwardPrunedDependencies.at(inputPort));
    return std::vector<const OutputPort*>(dependentOutputPorts.begin(), dependentOutputPorts.end());
}

void AtomicAccessor::Impl::ProcessInputs()
{
    PRINT_DEBUG("%s is reacting to inputs on all ports", this->GetName().c_str());
    auto inputPorts = this->GetOrderedInputPorts();
    for (InputPort* inputPort : inputPorts)
    {
        if (inputPort->IsWaitingForInputHandler())
        {
            this->InvokeInputHandlers(inputPort->GetName());
            inputPort->DequeueLatestInput();
            if (inputPort->IsWaitingForInputHandler())
            {
                // Schedule another reaction to process the next queued input
                auto myParent = static_cast<CompositeAccessor::Impl*>(this->GetParent());
                if (myParent != nullptr)
                {
                    myParent->ScheduleReaction(this, this->GetPriority());
                }

                inputPort->SendData(inputPort->ShareLatestInput());
            }
        }
    }

    if (this->m_fireFunction != nullptr)
    {
        this->m_fireFunction(*(static_cast<AtomicAccessor*>(this->m_container)));
    }

    PRINT_DEBUG("%s has finished reacting to all inputs", this->GetName().c_str());
}

void AtomicAccessor::Impl::AccessorStateDependsOn(const std::string& inputPortName)
{
    if (!this->HasInputPortWithName(inputPortName))
    {
        throw std::invalid_argument("Input port not found");
    }

    this->m_stateDependsOnInputPort = true;
}

void AtomicAccessor::Impl::RemoveDependency(const std::string& inputPortName, const std::string& outputPortName)
{
    const InputPort* inputPort = this->GetInputPort(inputPortName);
    const OutputPort* outputPort = this->GetOutputPort(outputPortName);
    this->m_forwardPrunedDependencies[inputPort].insert(outputPort);
    this->m_backwardPrunedDependencies[outputPort].insert(inputPort);
}

void AtomicAccessor::Impl::RemoveDependencies(const std::string& inputPortName, const std::vector<std::string>& outputPortNames)
{
    for (const auto& outputPortName : outputPortNames)
    {
        this->RemoveDependency(inputPortName, outputPortName);
    }
}

void AtomicAccessor::Impl::AddSpontaneousOutputPort(const std::string& portName)
{
    this->AddOutputPort(portName, true);
    auto inputPorts = this->GetInputPorts();
    for (auto inputPort : inputPorts)
    {
        this->RemoveDependency(inputPort->GetName(), portName);
    }
}

void AtomicAccessor::Impl::AddSpontaneousOutputPorts(const std::vector<std::string>& portNames)
{
    for (const std::string& portName : portNames)
    {
        this->AddSpontaneousOutputPort(portName);
    }
}

void AtomicAccessor::Impl::AddInputHandler(const std::string& inputPortName, AtomicAccessor::InputHandler handler)
{
    if (!this->HasInputPortWithName(inputPortName))
    {
        throw std::invalid_argument("Input port not found");
    }

    this->m_inputHandlers[inputPortName].push_back(handler);
}

void AtomicAccessor::Impl::AddInputHandlers(const std::string& inputPortName, const std::vector<AtomicAccessor::InputHandler>& handlers)
{
    if (!this->HasInputPortWithName(inputPortName))
    {
        throw std::invalid_argument("Input port not found");
    }

    this->m_inputHandlers[inputPortName].insert(this->m_inputHandlers[inputPortName].end(), handlers.begin(), handlers.end());
}

void AtomicAccessor::Impl::FindEquivalentPorts(const InputPort* inputPort, std::set<const InputPort*>& equivalentPorts, std::set<const OutputPort*>& dependentPorts) const
{
    if (equivalentPorts.find(inputPort) == equivalentPorts.end())
    {
        equivalentPorts.insert(inputPort);
        const std::vector<const OutputPort*>& dependentOutputPorts = this->GetDependentOutputPorts(inputPort);
        for (auto dependentOutputPort : dependentOutputPorts)
        {
            if (dependentPorts.find(dependentOutputPort) == dependentPorts.end())
            {
                dependentPorts.insert(dependentOutputPort);
                const std::vector<const InputPort*>& inputPortDependencies = this->GetInputPortDependencies(dependentOutputPort);
                for (auto inputPortDependency : inputPortDependencies)
                {
                    this->FindEquivalentPorts(inputPortDependency, equivalentPorts, dependentPorts);
                }
            }
        }
    }
}

void AtomicAccessor::Impl::InvokeInputHandlers(const std::string& inputPortName)
{
    PRINT_DEBUG("%s is handling input on input port \"%s\"", this->GetName().c_str(), inputPortName.c_str());
    
    IEvent* latestInput = this->GetLatestInput(inputPortName);
    const std::vector<InputHandler>& inputHandlers = this->m_inputHandlers.at(inputPortName);
    for (auto it = inputHandlers.begin(); it != inputHandlers.end(); ++it)
    {
        try
        {
            (*it)(latestInput);
        }
        catch (const std::exception& /*e*/)
        {
            this->m_inputHandlers.at(inputPortName).erase(it);
            throw;
        }
    }
}