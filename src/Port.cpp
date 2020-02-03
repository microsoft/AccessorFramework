// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Port.h"
#include "AccessorImpl.h"
#include "PrintDebug.h"

Port::Port(const std::string& name, Accessor::Impl* owner) :
    BaseObject(name, owner),
    m_source(nullptr)
{
}

Accessor::Impl* Port::GetOwner() const
{
    return static_cast<Accessor::Impl*>(this->GetParent());
}

bool Port::IsSpontaneous() const
{
    return false;
}

bool Port::IsConnectedToSource() const
{
    return (this->m_source != nullptr);
}

const Port* Port::GetSource() const
{
    return this->m_source;
}

std::vector<const Port*> Port::GetDestinations() const
{
    return std::vector<const Port*>(this->m_destinations.begin(), this->m_destinations.end());
}

void Port::SendData(std::shared_ptr<IEvent> data)
{
#ifdef PRINT_VERBOSE
    if (!(this->m_destinations.empty()))
    {
        PRINT_VERBOSE("Port %s is sending event data at address %p", this->GetFullName().c_str(), data.get());
    }
#endif

    for (auto destination : this->m_destinations)
    {
        destination->ReceiveData(data);
    }
}

void Port::Connect(Port* source, Port* destination)
{
    ValidateConnection(source, destination);
    PRINT_VERBOSE("Source port '%s' is connecting to destination port '%s'", source->GetFullName().c_str(), destination->GetFullName().c_str());
    destination->m_source = source;
    source->m_destinations.push_back(destination);
}

void Port::ValidateConnection(Port* source, Port* destination)
{
    if (destination->IsConnectedToSource() && destination->GetSource() != source)
    {
        std::ostringstream exceptionMessage;
        exceptionMessage << "Destination port '" << destination->GetFullName() << "' is already connected to source port '" << destination->GetSource()->GetFullName() << "'";
        throw std::invalid_argument(exceptionMessage.str());
    }
    else if (destination->IsSpontaneous())
    {
        std::ostringstream exceptionMessage;
        exceptionMessage << "Destination port" << destination->GetFullName() << "is spontaneous, so it cannot be connected to source port " << source->GetFullName();
        throw std::invalid_argument(exceptionMessage.str());
    }
}

InputPort::InputPort(const std::string& name, Accessor::Impl* owner) :
    Port(name, owner),
    m_waitingForInputHandler(false)
{
}

IEvent* InputPort::GetLatestInput() const
{
    IEvent* latestInput = (this->m_inputQueue.empty() ? nullptr : this->m_inputQueue.front().get());
    return latestInput;
}

std::shared_ptr<IEvent> InputPort::ShareLatestInput() const
{
    std::shared_ptr<IEvent> latestInput = (this->m_inputQueue.empty() ? nullptr : this->m_inputQueue.front());
    return latestInput;
}

int InputPort::GetInputQueueLength() const
{
    int inputQueueLength = static_cast<int>(this->m_inputQueue.size());
    return inputQueueLength;
}

bool InputPort::IsWaitingForInputHandler() const
{
    return this->m_waitingForInputHandler;
}

// Should only be called by the port's owner in AtomicAccessor::Impl::ProcessInputs()
void InputPort::DequeueLatestInput()
{
    if (!this->m_inputQueue.empty())
    {
        this->m_inputQueue.pop();
        if (this->m_inputQueue.empty())
        {
            this->m_waitingForInputHandler = false;
        }
        else
        {
            this->m_waitingForInputHandler = (this->m_inputQueue.front() != nullptr);
        }
    }
}

void InputPort::ReceiveData(std::shared_ptr<IEvent> input)
{
    auto myParent = static_cast<Accessor::Impl*>(this->GetParent());
    if (!(myParent->IsInitialized()))
    {
        PRINT_VERBOSE("Input port %s is dropping event data at address %p because its parent has not been initialized", this->GetFullName().c_str(), input.get());
        return;
    }

    PRINT_VERBOSE("Input port %s is receiving event data at address %p", this->GetFullName().c_str(), input.get());
    if (myParent->IsComposite())
    {
        this->SendData(input);
    }
    else
    {
        bool wasWaitingForInputHandler = this->m_waitingForInputHandler;
        this->QueueInput(input);
        if (!wasWaitingForInputHandler && this->m_waitingForInputHandler)
        {
            myParent->AlertNewInput();
            this->SendData(input);
        }
    }
}

void InputPort::QueueInput(std::shared_ptr<IEvent> input)
{
    this->m_inputQueue.push(input);
    this->m_waitingForInputHandler = (this->m_inputQueue.front() != nullptr);
}

OutputPort::OutputPort(const std::string& name, Accessor::Impl* owner, bool spontaneous) :
    Port(name, owner),
    m_spontaneous(spontaneous)
{
}

bool OutputPort::IsSpontaneous() const
{
    return this->m_spontaneous;
}

void OutputPort::ReceiveData(std::shared_ptr<IEvent> input)
{
    auto myParent = static_cast<Accessor::Impl*>(this->GetParent());
    if (!(myParent->IsInitialized()))
    {
        PRINT_VERBOSE("Output port %s is dropping event data at address %p because its parent has not been initialized", this->GetFullName().c_str(), input.get());
        return;
    }

    PRINT_VERBOSE("Output port %s is receiving event data at address %p", this->GetFullName().c_str(), input.get());
    this->SendData(input);
}