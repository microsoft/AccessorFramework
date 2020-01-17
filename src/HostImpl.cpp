// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "AtomicAccessorImpl.h"
#include "CompositeAccessorImpl.h"
#include "HostImpl.h"
#include "PrintDebug.h"
#include <cassert>
#include <thread>

static const int UpdateModelPriority = 0;
static const int HostPriority = UpdateModelPriority + 1;

Host::Impl::Impl(const std::string& name, Host* container, std::function<void(Accessor&)> initializeFunction) :
    CompositeAccessor::Impl(name, container, initializeFunction),
    m_state(Host::State::NeedsSetup),
    m_director(std::make_shared<Director>()),
    m_executionCancellationToken(nullptr),
    m_nextListenerId(0)
{
    this->m_priority = HostPriority;
}

Host::Impl::~Impl() = default;

Host::State Host::Impl::GetState() const
{
    return this->m_state.load();
}

bool Host::Impl::EventListenerIsRegistered(int listenerId) const
{
    bool registered = (this->m_listeners.find(listenerId) != this->m_listeners.end());
    return registered;
}

int Host::Impl::AddEventListener(std::weak_ptr<Host::EventListener> listener)
{
    int listenerId = this->m_nextListenerId++;
    this->m_listeners.emplace(listenerId, std::move(listener));
    return listenerId;
}

void Host::Impl::RemoveEventListener(int listenerId)
{
    this->m_listeners.erase(listenerId);
}

void Host::Impl::Setup()
{
    if (this->m_state.load() != Host::State::NeedsSetup)
    {
        throw std::logic_error("Host does not need setup");
    }

    this->SetState(Host::State::SettingUp);
    static_cast<Host*>(this->m_container)->AdditionalSetup();
    this->ComputeAccessorPriorities();
    this->Initialize();
    this->SetState(Host::State::ReadyToRun);
}

void Host::Impl::Iterate(int numberOfIterations)
{
    this->ValidateHostCanRun();
    this->SetState(Host::State::Running);
    this->m_executionCancellationToken = std::make_shared<CancellationToken>();

    try
    {
        this->m_director->Execute(this->m_executionCancellationToken, numberOfIterations);
    }
    catch (const std::exception& e)
    {
        this->m_state.store(Host::State::Corrupted);
        this->NotifyListenersOfException(e);
    }

    this->SetState(Host::State::Paused);
}

void Host::Impl::Pause()
{
    if (this->m_state.load() != Host::State::Running)
    {
        throw std::logic_error("Host is not running");
    }

    this->m_executionCancellationToken->Cancel();
    this->m_executionCancellationToken = nullptr;
    this->SetState(Host::State::Paused);
}

void Host::Impl::Run()
{
    this->ValidateHostCanRun();
    bool retry = false;
    do
    {
        retry = false;
        try
        {
            std::thread(&Host::Impl::RunOnCurrentThread, this).detach();
        }
        catch (const std::system_error& e)
        {
            if (e.code() == std::errc::resource_unavailable_try_again)
            {
                retry = true;
            }
            else
            {
                throw;
            }
        }
    } while (retry);
}

void Host::Impl::RunOnCurrentThread()
{
    this->ValidateHostCanRun();
    this->SetState(Host::State::Running);
    this->m_executionCancellationToken = std::make_shared<CancellationToken>();

    try
    {
        this->m_director->Execute(this->m_executionCancellationToken);
    }
    catch (const std::exception& e)
    {
        this->m_state.store(Host::State::Corrupted);
        this->NotifyListenersOfException(e);
    }

    this->SetState(Host::State::Paused);
}

void Host::Impl::Exit()
{
    this->SetState(Host::State::Exiting);
    if (this->m_executionCancellationToken.get() != nullptr)
    {
        this->m_executionCancellationToken->Cancel();
        this->m_executionCancellationToken = nullptr;
    }

    this->SetState(Host::State::Finished);
}

void Host::Impl::AddInputPort(const std::string& portName)
{
    throw std::logic_error("Hosts are not allowed to have ports");
}

void Host::Impl::AddInputPorts(const std::vector<std::string>& portNames)
{
    throw std::logic_error("Hosts are not allowed to have ports");
}

void Host::Impl::AddOutputPort(const std::string& portName)
{
    throw std::logic_error("Hosts are not allowed to have ports");
}

void Host::Impl::AddOutputPorts(const std::vector<std::string>& portNames)
{
    throw std::logic_error("Hosts are not allowed to have ports");
}

void Host::Impl::ChildrenChanged()
{
    this->m_priority = UpdateModelPriority;
    this->ScheduleCallback(
        [this]()
        {
            PRINT_DEBUG("%s is updating the model", this->GetName().c_str());
            this->ComputeAccessorPriorities(true /*updateCallbacks*/);
            for (auto child : this->GetChildren())
            {
                if (!(child->IsInitialized()))
                {
                    child->Initialize();
                }
            }
        },
        0 /*delayInMilliseconds*/,
        false /*repeat*/
    );

    this->m_priority = HostPriority;
}

void Host::Impl::ResetPriority()
{
    this->m_priority = HostPriority;
    this->ResetChildrenPriorities();
}

std::shared_ptr<Director> Host::Impl::GetDirector() const
{
    return this->m_director->shared_from_this();
}

void Host::Impl::ValidateHostCanRun() const
{
    if (this->m_state.load() == Host::State::Running)
    {
        throw std::logic_error("Host is already running");
    }
    else if (this->m_state.load() != Host::State::ReadyToRun && this->m_state.load() != Host::State::Paused)
    {
        throw std::logic_error("Host is not in a runnable state");
    }
}

void Host::Impl::SetState(Host::State newState)
{
    Host::State oldState = this->m_state.exchange(newState);
    if (oldState != newState)
    {
        this->NotifyListenersOfStateChange(oldState, newState);
    }
}

void Host::Impl::ComputeAccessorPriorities(bool updateCallbacks)
{
    std::map<int, std::vector<Accessor::Impl*>> accessorDepths{};
    std::map<const Port*, int> portDepths{};
    this->ComputeCompositeAccessorDepth(this, portDepths, accessorDepths);
    int priority = HostPriority;
    for (auto entry : accessorDepths)
    {
        priority = std::max(priority, entry.first);
        for (auto accessor : entry.second)
        {
            if (updateCallbacks)
            {
                int oldPriority = accessor->GetPriority();
                this->m_director->HandlePriorityUpdate(oldPriority, priority);
            }
            
            accessor->SetPriority(priority);
            ++priority;
        }
    }
}

int Host::Impl::ComputeCompositeAccessorDepth(CompositeAccessor::Impl* compositeAccessor, std::map<const Port*, int>& portDepths, std::map<int, std::vector<Accessor::Impl*>>& accessorDepths)
{
    int minChildDepth = INT_MAX;
    for (auto child : compositeAccessor->GetChildren())
    {
        int childDepth = 0;
        if (child->IsComposite())
        {
            childDepth = this->ComputeCompositeAccessorDepth(static_cast<CompositeAccessor::Impl*>(child), portDepths, accessorDepths);
        }
        else
        {
            childDepth = this->ComputeAtomicAccessorDepth(static_cast<AtomicAccessor::Impl*>(child), portDepths, accessorDepths);
        }

        minChildDepth = std::min(minChildDepth, childDepth);
    }

    int accessorDepth = minChildDepth;
    (accessorDepths[accessorDepth]).insert((accessorDepths[accessorDepth]).begin(), compositeAccessor);
    return accessorDepth;
}

int Host::Impl::ComputeAtomicAccessorDepth(AtomicAccessor::Impl* atomicAccessor, std::map<const Port*, int>& portDepths, std::map<int, std::vector<Accessor::Impl*>>& accessorDepths)
{
    int maximumInputDepth = 0;
    for (auto inputPort : atomicAccessor->GetInputPorts())
    {
        if (portDepths.find(inputPort) == portDepths.end())
        {
            std::set<const InputPort*> visitedInputPorts{};
            std::set<const OutputPort*> visitedOutputPorts{};
            this->ComputeAtomicAccessorInputPortDepth(inputPort, portDepths, visitedInputPorts, visitedOutputPorts);
        }

        if (portDepths.at(inputPort) > maximumInputDepth)
        {
            maximumInputDepth = portDepths.at(inputPort);
        }
    }

    int minimumOutputDepth = INT_MAX;
    for (auto outputPort : atomicAccessor->GetOutputPorts())
    {
        if (portDepths.find(outputPort) == portDepths.end())
        {
            std::set<const InputPort*> visitedInputPorts{};
            std::set<const OutputPort*> visitedOutputPorts{};
            this->ComputeAtomicAccessorOutputPortDepth(outputPort, portDepths, visitedInputPorts, visitedOutputPorts);
        }

        if (portDepths.at(outputPort) < minimumOutputDepth)
        {
            minimumOutputDepth = portDepths.at(outputPort);
        }
    }

    int accessorDepth = (atomicAccessor->HasOutputPorts() ? minimumOutputDepth : maximumInputDepth);
    accessorDepths[accessorDepth].push_back(atomicAccessor);
    return accessorDepth;
}

void Host::Impl::ComputeAtomicAccessorInputPortDepth(const InputPort* inputPort, std::map<const Port*, int>& portDepths, std::set<const InputPort*>& visitedInputPorts, std::set<const OutputPort*>& visitedOutputPorts)
{
    int depth = 0;
    auto equivalentPorts = static_cast<AtomicAccessor::Impl*>(inputPort->GetOwner())->GetEquivalentPorts(inputPort);
    for (auto equivalentPort : equivalentPorts)
    {
        visitedInputPorts.insert(equivalentPort);
        if (equivalentPort->IsConnectedToSource())
        {
            const OutputPort* sourceOutputPort = GetSourceOutputPort(equivalentPort);
            if (sourceOutputPort == nullptr)
            {
                // not connected to source
                continue;
            }

            if (portDepths.find(sourceOutputPort) == portDepths.end())
            {
                if (visitedOutputPorts.find(sourceOutputPort) != visitedOutputPorts.end())
                {
                    std::ostringstream exceptionMessage;
                    exceptionMessage << "Detected causality loop involving port " << sourceOutputPort->GetFullName();
                    throw std::logic_error(exceptionMessage.str());
                }
                else
                {
                    this->ComputeAtomicAccessorOutputPortDepth(sourceOutputPort, portDepths, visitedInputPorts, visitedOutputPorts);
                }
            }

            int newDepth = portDepths.at(sourceOutputPort) + 1;
            if (depth < newDepth)
            {
                depth = newDepth;
            }
        }
    }

    for (auto equivalentPort : equivalentPorts)
    {
        PRINT_VERBOSE("Input port '%s' is now priority %d", equivalentPort->GetFullName().c_str(), depth);
        portDepths[equivalentPort] = depth;
    }
}

void Host::Impl::ComputeAtomicAccessorOutputPortDepth(const OutputPort* outputPort, std::map<const Port*, int>& portDepths, std::set<const InputPort*>& visitedInputPorts, std::set<const OutputPort*>& visitedOutputPorts)
{
    visitedOutputPorts.insert(outputPort);
    int depth = 0;
    std::vector<const InputPort*> inputPortDependencies = static_cast<AtomicAccessor::Impl*>(outputPort->GetOwner())->GetInputPortDependencies(outputPort);
    for (auto inputPort : inputPortDependencies)
    {
        if (portDepths.find(inputPort) == portDepths.end())
        {
            if (visitedInputPorts.find(inputPort) != visitedInputPorts.end())
            {
                std::ostringstream exceptionMessage;
                exceptionMessage << "Detected causality loop involving port " << inputPort->GetFullName();
                throw std::logic_error(exceptionMessage.str().c_str());
            }
            else
            {
                this->ComputeAtomicAccessorInputPortDepth(inputPort, portDepths, visitedInputPorts, visitedOutputPorts);
            }
        }

        if (depth < portDepths.at(inputPort))
        {
            depth = portDepths.at(inputPort);
        }
    }

    PRINT_VERBOSE("Output port '%s' is now priority %d", outputPort->GetFullName().c_str(), depth);
    portDepths[outputPort] = depth;
}

void Host::Impl::NotifyListenersOfException(const std::exception& e)
{
    auto it = this->m_listeners.begin();
    while (it != this->m_listeners.end())
    {
        if (auto strongListener = it->second.lock())
        {
            try
            {
                strongListener->NotifyOfException(e);
                ++it;
            }
            catch (const std::exception&)
            {
                this->m_listeners.erase(it);
                continue;
            }
        }
        else
        {
            this->m_listeners.erase(it);
        }
    }
}

void Host::Impl::NotifyListenersOfStateChange(Host::State oldState, Host::State newState)
{
    auto it = this->m_listeners.begin();
    while (it != this->m_listeners.end())
    {
        if (auto strongListener = it->second.lock())
        {
            try
            {
                strongListener->NotifyOfStateChange(oldState, newState);
                ++it;
            }
            catch (std::exception)
            {
                this->m_listeners.erase(it);
                continue;
            }
        }
        else
        {
            this->m_listeners.erase(it);
        }
    }
}

const OutputPort* Host::Impl::GetSourceOutputPort(const InputPort* inputPort)
{
    const Port* sourcePort = inputPort->GetSource();
    while (sourcePort->GetOwner()->IsComposite())
    {
        if (!sourcePort->IsConnectedToSource())
        {
            return nullptr;
        }

        sourcePort = sourcePort->GetSource();
    }

    return static_cast<const OutputPort*>(sourcePort);
}