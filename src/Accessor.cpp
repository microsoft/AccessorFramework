// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "AccessorFramework/Accessor.h"
#include "AccessorImpl.h"
#include "AtomicAccessorImpl.h"
#include "CompositeAccessorImpl.h"
#include "PrintDebug.h"
#include <algorithm>

Accessor::~Accessor() = default;

std::string Accessor::GetName() const
{
    return this->m_impl->GetName();
}

Accessor::Impl* Accessor::GetImpl() const
{
    return this->m_impl.get();
}

bool Accessor::NameIsValid(const std::string& name)
{
    return Accessor::Impl::NameIsValid(name);
}

Accessor::Accessor(std::unique_ptr<Accessor::Impl> impl) :
    m_impl(std::move(impl))
{
}

void Accessor::Initialize()
{
    // base implementation does nothing
}

int Accessor::ScheduleCallback(std::function<void()> callback, int delayInMilliseconds, bool repeat)
{
    return this->m_impl->ScheduleCallback(callback, delayInMilliseconds, repeat);
}

void Accessor::ClearScheduledCallback(int callbackId)
{
    this->m_impl->ClearScheduledCallback(callbackId);
}

void Accessor::ClearAllScheduledCallbacks()
{
    this->m_impl->ClearAllScheduledCallbacks();
}

bool Accessor::NewPortNameIsValid(const std::string& newPortName) const
{
    return this->m_impl->NewPortNameIsValid(newPortName);
}

void Accessor::AddInputPort(const std::string& portName)
{
    this->m_impl->AddInputPort(portName);
}

void Accessor::AddInputPorts(const std::vector<std::string>& portNames)
{
    this->m_impl->AddInputPorts(portNames);
}
void Accessor::AddOutputPort(const std::string& portName)
{
    this->m_impl->AddOutputPort(portName);
}

void Accessor::AddOutputPorts(const std::vector<std::string>& portNames)
{
    this->m_impl->AddOutputPorts(portNames);
}

void Accessor::ConnectMyInputToMyOutput(const std::string& myInputPortName, const std::string& myOutputPortName)
{
    this->m_impl->ConnectMyInputToMyOutput(myInputPortName, myOutputPortName);
}

void Accessor::ConnectMyOutputToMyInput(const std::string& myOutputPortName, const std::string& myInputPortName)
{
    this->m_impl->ConnectMyOutputToMyInput(myOutputPortName, myInputPortName);
}

IEvent* Accessor::GetLatestInput(const std::string& inputPortName) const
{
    return this->m_impl->GetLatestInput(inputPortName);
}

void Accessor::SendOutput(const std::string& outputPortName, std::shared_ptr<IEvent> output)
{
    this->m_impl->SendOutput(outputPortName, output);
}

CompositeAccessor::CompositeAccessor(
    const std::string& name,
    const std::vector<std::string>& inputPortNames,
    const std::vector<std::string>& outputPortNames)
    : Accessor(std::make_unique<CompositeAccessor::Impl>(name, this, &CompositeAccessor::Initialize, inputPortNames, outputPortNames))
{
}

CompositeAccessor::CompositeAccessor(std::unique_ptr<CompositeAccessor::Impl> impl) :
    Accessor(std::move(impl))
{
}

bool CompositeAccessor::NewChildNameIsValid(const std::string& newChildName) const
{
    return static_cast<CompositeAccessor::Impl*>(this->GetImpl())->NewChildNameIsValid(newChildName);
}

void CompositeAccessor::AddChild(std::unique_ptr<Accessor> child)
{
    static_cast<CompositeAccessor::Impl*>(this->GetImpl())->AddChild(std::move(child));
}

void CompositeAccessor::RemoveChild(const std::string& childName)
{
    static_cast<CompositeAccessor::Impl*>(this->GetImpl())->RemoveChild(childName);
}

void CompositeAccessor::RemoveAllChildren()
{
    static_cast<CompositeAccessor::Impl*>(this->GetImpl())->RemoveAllChildren();
}

void CompositeAccessor::ConnectMyInputToChildInput(const std::string& myInputPortName, const std::string& childName, const std::string& childInputPortName)
{
    static_cast<CompositeAccessor::Impl*>(this->GetImpl())->ConnectMyInputToChildInput(myInputPortName, childName, childInputPortName);
}

void CompositeAccessor::ConnectChildOutputToMyOutput(const std::string& childName, const std::string& childOutputPortName, const std::string& myOutputPortName)
{
    static_cast<CompositeAccessor::Impl*>(this->GetImpl())->ConnectChildOutputToMyOutput(childName, childOutputPortName, myOutputPortName);
}

void CompositeAccessor::ConnectChildren(
    const std::string& sourceChildName,
    const std::string& sourceChildOutputPortName,
    const std::string& destinationChildName,
    const std::string& destinationChildInputPortName)
{
    return static_cast<CompositeAccessor::Impl*>(this->GetImpl())->ConnectChildren(sourceChildName, sourceChildOutputPortName, destinationChildName, destinationChildInputPortName);
}

void CompositeAccessor::ChildrenChanged()
{
    static_cast<CompositeAccessor::Impl*>(this->GetImpl())->ChildrenChanged();
}

AtomicAccessor::AtomicAccessor(
    const std::string& name,
    const std::vector<std::string>& inputPortNames,
    const std::vector<std::string>& outputPortNames,
    const std::vector<std::string>& spontaneousOutputPortNames,
    const std::map<std::string, std::vector<InputHandler>>& inputHandlers)
    : Accessor(std::make_unique<AtomicAccessor::Impl>(name, this, &AtomicAccessor::Initialize, inputPortNames, outputPortNames, spontaneousOutputPortNames, inputHandlers, &AtomicAccessor::Fire))
{
}

void AtomicAccessor::AccessorStateDependsOn(const std::string& inputPortName)
{
    static_cast<AtomicAccessor::Impl*>(this->GetImpl())->AccessorStateDependsOn(inputPortName);
}

void AtomicAccessor::RemoveDependency(const std::string& inputPortName, const std::string& outputPortName)
{
    static_cast<AtomicAccessor::Impl*>(this->GetImpl())->RemoveDependency(inputPortName, outputPortName);
}

void AtomicAccessor::RemoveDependencies(const std::string& inputPortName, const std::vector<std::string>& outputPortNames)
{
    static_cast<AtomicAccessor::Impl*>(this->GetImpl())->RemoveDependencies(inputPortName, outputPortNames);
}

void AtomicAccessor::AddSpontaneousOutputPort(const std::string& portName)
{
    static_cast<AtomicAccessor::Impl*>(this->GetImpl())->AddSpontaneousOutputPort(portName);
}

void AtomicAccessor::AddSpontaneousOutputPorts(const std::vector<std::string>& portNames)
{
    static_cast<AtomicAccessor::Impl*>(this->GetImpl())->AddSpontaneousOutputPorts(portNames);
}

void AtomicAccessor::AddInputHandler(const std::string& inputPortName, InputHandler handler)
{
    static_cast<AtomicAccessor::Impl*>(this->GetImpl())->AddInputHandler(inputPortName, handler);
}

void AtomicAccessor::AddInputHandlers(const std::string& inputPortName, const std::vector<InputHandler>& handlers)
{
    static_cast<AtomicAccessor::Impl*>(this->GetImpl())->AddInputHandlers(inputPortName, handlers);
}

void AtomicAccessor::Fire()
{
    // base implementation does nothing
}