// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "CompositeAccessorImpl.h"
#include "AtomicAccessorImpl.h"
#include "PrintDebug.h"

CompositeAccessor::Impl::Impl(const std::string& name, const std::vector<std::string>& inputPortNames, const std::vector<std::string>& connectedOutputPortNames) :
    Accessor::Impl(name, inputPortNames, connectedOutputPortNames),
    m_reactionRequested(false)
{
}

bool CompositeAccessor::Impl::HasChildWithName(const std::string& childName) const
{
    return (this->m_children.find(childName) != this->m_children.end());
}

Accessor::Impl* CompositeAccessor::Impl::GetChild(const std::string& childName) const
{
    return this->m_children.at(childName)->GetImpl();
}

std::vector<Accessor::Impl*> CompositeAccessor::Impl::GetChildren() const
{
    return this->m_orderedChildren;
}

void CompositeAccessor::Impl::ScheduleReaction(Accessor::Impl* child, int priority)
{
    if (priority == INT_MAX)
    {
        priority = this->GetPriority();
    }

    auto myParent = static_cast<CompositeAccessor::Impl*>(this->GetParent());
    if (myParent != nullptr)
    {
        this->m_childEventQueue.push(child);
        myParent->ScheduleReaction(this, priority);
    }
    else if (!this->m_reactionRequested)
    {
        this->m_reactionRequested = true;
        this->m_childEventQueue.push(child);
        this->GetDirector()->ScheduleCallback(
            [this]() { this->ProcessChildEventQueue(); },
            0 /*delayInMilliseconds*/,
            false /*repeat*/,
            priority);
    }
    else
    {
        this->m_childEventQueue.push(child);
    }
}

void CompositeAccessor::Impl::ProcessChildEventQueue()
{
    while (!this->m_childEventQueue.empty())
    {
        Accessor::Impl* child = this->m_childEventQueue.top();
        this->m_childEventQueue.pop();
        if (child->IsComposite())
        {
            static_cast<CompositeAccessor::Impl*>(child)->ProcessChildEventQueue();
        }
        else
        {
            static_cast<AtomicAccessor::Impl*>(child)->ProcessInputs();
        }
    }

    this->m_reactionRequested = false;
    PRINT_DEBUG("%s has finished reacting to all inputs", this->GetName().c_str());
}

void CompositeAccessor::Impl::ResetPriority()
{
    Accessor::Impl::ResetPriority();
    this->ResetChildrenPriorities();
}

bool CompositeAccessor::Impl::IsComposite() const
{
    return true;
}

void CompositeAccessor::Impl::Initialize()
{
    for (auto child : this->m_orderedChildren)
    {
        child->Initialize();
    }
}

bool CompositeAccessor::Impl::NewChildNameIsValid(const std::string& newChildName) const
{
    // A new child's name cannot be the same as the parent's name or the same as an existing child's name
    return (NameIsValid(newChildName) && newChildName != this->GetName() && !this->HasChildWithName(newChildName));
}

void CompositeAccessor::Impl::AddChild(std::unique_ptr<Accessor> child)
{
    std::string childName = child->GetName();
    if (!this->NewChildNameIsValid(childName))
    {
        throw std::invalid_argument("Child name is invalid");
    }

    child->GetImpl()->SetParent(this);
    this->m_children.emplace(childName, std::move(child));
    this->m_orderedChildren.push_back(this->m_children.at(childName)->GetImpl());
}

void CompositeAccessor::Impl::ConnectMyInputToChildInput(const std::string& myInputPortName, const std::string& childName, const std::string& childInputPortName)
{
    Port::Connect(this->GetInputPort(myInputPortName), this->GetChild(childName)->GetInputPort(childInputPortName));
}

void CompositeAccessor::Impl::ConnectChildOutputToMyOutput(const std::string& childName, const std::string& childOutputPortName, const std::string& myOutputPortName)
{
    Port::Connect(this->GetChild(childName)->GetOutputPort(childOutputPortName), this->GetOutputPort(myOutputPortName));
}

void CompositeAccessor::Impl::ConnectChildren(
    const std::string& sourceChildName,
    const std::string& sourceChildOutputPortName,
    const std::string& destinationChildName,
    const std::string& destinationChildInputPortName)
{
    Port::Connect(
        this->GetChild(sourceChildName)->GetOutputPort(sourceChildOutputPortName),
        this->GetChild(destinationChildName)->GetInputPort(destinationChildInputPortName));
}

void CompositeAccessor::Impl::ResetChildrenPriorities() const
{
    for (auto child : this->m_orderedChildren)
    {
        child->ResetPriority();
    }
}