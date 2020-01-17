// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "AccessorImpl.h"
#include "CompositeAccessorImpl.h"
#include "Director.h"
#include "PrintDebug.h"

const int Accessor::Impl::DefaultAccessorPriority = INT_MAX;

Accessor::Impl::~Impl() = default;

bool Accessor::Impl::IsInitialized() const
{
    return this->m_initialized;
}

void Accessor::Impl::Initialize()
{
    if (this->m_initializeFunction != nullptr)
    {
        this->m_initializeFunction(*(this->m_container));
    }

    this->m_initialized = true;
}

void Accessor::Impl::SetParent(CompositeAccessor::Impl* parent)
{
    BaseObject::SetParent(parent);
}

int Accessor::Impl::GetPriority() const
{
    return this->m_priority;
}

void Accessor::Impl::SetPriority(int priority)
{
    PRINT_VERBOSE("%s now has priority %d", this->GetFullName().c_str(), priority);
    this->m_priority = priority;
}

void Accessor::Impl::ResetPriority()
{
    this->m_priority = DefaultAccessorPriority;
}

std::shared_ptr<Director> Accessor::Impl::GetDirector() const
{
    auto myParent = static_cast<CompositeAccessor::Impl*>(this->GetParent());
    if (myParent == nullptr)
    {
        return nullptr;
    }
    else
    {
        return myParent->GetDirector();
    }
}

bool Accessor::Impl::HasInputPorts() const
{
    return !(this->m_inputPorts.empty());
}

bool Accessor::Impl::HasOutputPorts() const
{
    return !(this->m_outputPorts.empty());
}

InputPort* Accessor::Impl::GetInputPort(const std::string& portName) const
{
    return this->m_inputPorts.at(portName).get();
}

OutputPort* Accessor::Impl::GetOutputPort(const std::string& portName) const
{
    return this->m_outputPorts.at(portName).get();
}

std::vector<const InputPort*> Accessor::Impl::GetInputPorts() const
{
    return std::vector<const InputPort*>(this->m_orderedInputPorts.begin(), this->m_orderedInputPorts.end());
}

std::vector<const OutputPort*> Accessor::Impl::GetOutputPorts() const
{
    return std::vector<const OutputPort*>(this->m_orderedOutputPorts.begin(), this->m_orderedOutputPorts.end());
}

void Accessor::Impl::AlertNewInput()
{
    auto myParent = static_cast<CompositeAccessor::Impl*>(this->GetParent());
    if (myParent != nullptr)
    {
        myParent->ScheduleReaction(this, this->m_priority);
    }
}

bool Accessor::Impl::operator<(const Accessor::Impl& other) const
{
    return (this->m_priority < other.GetPriority());
}

bool Accessor::Impl::operator>(const Accessor::Impl& other) const
{
    return (this->m_priority > other.GetPriority());
}

int Accessor::Impl::ScheduleCallback(
    std::function<void()> callback,
    int delayInMilliseconds,
    bool repeat)
{
    int callbackId = this->GetDirector()->ScheduleCallback(
        callback,
        delayInMilliseconds,
        repeat,
        this->m_priority);
    this->m_callbackIds.insert(callbackId);
    return callbackId;
}

void Accessor::Impl::ClearScheduledCallback(int callbackId)
{
    this->GetDirector()->ClearScheduledCallback(callbackId);
    this->m_callbackIds.erase(callbackId);
}

bool Accessor::Impl::NewPortNameIsValid(const std::string& newPortName) const
{
    return (
        NameIsValid(newPortName) &&
        !this->HasInputPortWithName(newPortName) &&
        !this->HasOutputPortWithName(newPortName));
}

void Accessor::Impl::AddInputPort(const std::string& portName)
{
    PRINT_VERBOSE("%s is creating a new input port \'%s\'", this->GetName().c_str(), portName.c_str());
    this->ValidatePortName(portName);
    this->m_inputPorts.emplace(portName, std::make_unique<InputPort>(portName, this));
    this->m_orderedInputPorts.push_back(this->m_inputPorts.at(portName).get());
}

void Accessor::Impl::AddInputPorts(const std::vector<std::string>& portNames)
{
    for (const std::string& portName : portNames)
    {
        this->AddInputPort(portName);
    }
}

void Accessor::Impl::AddOutputPort(const std::string& portName)
{
    this->AddOutputPort(portName, false /*isSpontaneous*/);
}

void Accessor::Impl::AddOutputPorts(const std::vector<std::string>& portNames)
{
    for (const std::string& portName : portNames)
    {
        this->AddOutputPort(portName);
    }
}

void Accessor::Impl::ConnectMyInputToMyOutput(const std::string& myInputPortName, const std::string& myOutputPortName)
{
    Port::Connect(this->m_inputPorts.at(myInputPortName).get(), this->m_outputPorts.at(myOutputPortName).get());
}

void Accessor::Impl::ConnectMyOutputToMyInput(const std::string& myOutputPortName, const std::string& myInputPortName)
{
    Port::Connect(this->m_outputPorts.at(myOutputPortName).get(), this->m_inputPorts.at(myInputPortName).get());
}

IEvent* Accessor::Impl::GetLatestInput(const std::string& inputPortName) const
{
    return this->GetInputPort(inputPortName)->GetLatestInput();
}

void Accessor::Impl::SendOutput(const std::string& outputPortName, std::shared_ptr<IEvent> output)
{
    if (!(this->IsInitialized()))
    {
        throw std::logic_error("Outputs cannot be sent until the accessor is initialized");
    }

    this->ScheduleCallback(
        [this, outputPortName, output]()
        {
            this->GetOutputPort(outputPortName)->SendData(output);
        },
        0 /*delayInMilliseconds*/,
        false /*repeat*/);
}

Accessor::Impl::Impl(
    const std::string& name,
    Accessor* container,
    std::function<void(Accessor&)> initializeFunction,
    const std::vector<std::string>& inputPortNames,
    const std::vector<std::string>& connectedOutputPortNames) :
    BaseObject(name),
    m_initialized(false),
    m_container(container),
    m_priority(DefaultAccessorPriority),
    m_initializeFunction(initializeFunction)
{
    this->AddInputPorts(inputPortNames);
    this->AddOutputPorts(connectedOutputPortNames);
}

size_t Accessor::Impl::GetNumberOfInputPorts() const
{
    return this->m_orderedInputPorts.size();
}

size_t Accessor::Impl::GetNumberOfOutputPorts() const
{
    return this->m_orderedOutputPorts.size();
}

std::vector<InputPort*> Accessor::Impl::GetOrderedInputPorts() const
{
    return this->m_orderedInputPorts;
}

std::vector<OutputPort*> Accessor::Impl::GetOrderedOutputPorts() const
{
    return this->m_orderedOutputPorts;
}

bool Accessor::Impl::HasInputPortWithName(const std::string& portName) const
{
    return (this->m_inputPorts.find(portName) != this->m_inputPorts.end());
}

bool Accessor::Impl::HasOutputPortWithName(const std::string& portName) const
{
    return (this->m_outputPorts.find(portName) != this->m_outputPorts.end());
}

void Accessor::Impl::AddOutputPort(const std::string& portName, bool isSpontaneous)
{
    PRINT_VERBOSE("Accessor '%s' is creating a new%s output port \'%s\'", this->GetName().c_str(), isSpontaneous ? " spontaneous" : "", portName.c_str());
    this->ValidatePortName(portName);
    this->m_outputPorts.emplace(portName, std::make_unique<OutputPort>(portName, this, isSpontaneous));
    this->m_orderedOutputPorts.push_back(this->m_outputPorts.at(portName).get());
}

void Accessor::Impl::ValidatePortName(const std::string& portName) const
{
    if (!this->NewPortNameIsValid(portName))
    {
        std::ostringstream exceptionMessage;
        exceptionMessage << "Port name '" << portName << "' is invalid";
        throw std::invalid_argument(exceptionMessage.str());
    }
}