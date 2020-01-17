// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef HOST_IMPL_H
#define HOST_IMPL_H

#include "AccessorFramework/Host.h"
#include "CancellationToken.h"
#include "CompositeAccessorImpl.h"
#include "Director.h"
#include <atomic>
#include <map>
#include <vector>

// Description
// The HostImpl implements the public Host interface defined in Host.h. In addition, it exposes additional functionality
// for internal use, such as a public method to access the contained model. The HostImpl is also responsible for
// assigning priorities to the atomic accessors in the model. To do this, it follows the connections between accessor
// ports, calculating the depth of each port to quantify causal dependencies. At the same time, it also checks the model
// for causal loops; if there is a cyclic connection such that liveness cannot be established, the HostImpl will throw.
// The depth of an input port is defined as the maximum depth of all input ports in the same equivalence class. The
// source depth of an input port is the depth of its source port plus one, or 0 if there is no source. The depth of an
// output port is defined as the maximum depths of all input ports it depends on, or 0 if it does not depend on any input
// ports (i.e. is a spontaneous output port).
//
// For more information, see "Causality Interfaces for Actor Networks" (Zhou and Lee)
// http://www.eecs.berkeley.edu/Pubs/TechRpts/2006/EECS-2006-148.html
//
class Host::Impl : public CompositeAccessor::Impl
{
public:
    Impl(const std::string& name, Host* container, std::function<void(Accessor&)> initializeFunction);
    ~Impl();
    void ResetPriority() override;
    std::shared_ptr<Director> GetDirector() const override;

protected:
    // Host Methods
    Host::State GetState() const;
    bool EventListenerIsRegistered(int listenerId) const;
    int AddEventListener(std::weak_ptr<Host::EventListener> listener);
    void RemoveEventListener(int listenerId);
    void Setup();
    void Iterate(int numberOfIterations = 1);
    void Pause();
    void Run();
    void RunOnCurrentThread();
    void Exit();

    // Hosts are not allowed to have ports; these methods will throw
    void AddInputPort(const std::string& portName) final;
    void AddInputPorts(const std::vector<std::string>& portNames) final;
    void AddOutputPort(const std::string& portName) final;
    void AddOutputPorts(const std::vector<std::string>& portNames) final;

    void ChildrenChanged() final;

private:
    friend class Host;

    // Internal Methods
    void ValidateHostCanRun() const;
    void SetState(Host::State newState);
    void ComputeAccessorPriorities(bool updateCallbacks = false);
    int ComputeCompositeAccessorDepth(
        CompositeAccessor::Impl* compositeAccessor,
        std::map<const Port*, int>& portDepths,
        std::map<int, std::vector<Accessor::Impl*>>& accessorDepths);
    int ComputeAtomicAccessorDepth(
        AtomicAccessor::Impl* atomicAccessor,
        std::map<const Port*, int>& portDepths,
        std::map<int, std::vector<Accessor::Impl*>>& accessorDepths);
    void ComputeAtomicAccessorInputPortDepth(
        const InputPort* inputPort,
        std::map<const Port*, int>& portDepths,
        std::set<const InputPort*>& visitedInputPorts,
        std::set<const OutputPort*>& visitedOutputPorts);
    void ComputeAtomicAccessorOutputPortDepth(
        const OutputPort* outputPort,
        std::map<const Port*, int>& portDepths,
        std::set<const InputPort*>& visitedInputPorts,
        std::set<const OutputPort*>& visitedOutputPorts);

    void NotifyListenersOfException(const std::exception& e);
    void NotifyListenersOfStateChange(Host::State oldState, Host::State newState);

    std::atomic<Host::State> m_state;
    std::shared_ptr<Director> m_director;
    std::shared_ptr<CancellationToken> m_executionCancellationToken;
    std::map<int, std::weak_ptr<Host::EventListener>> m_listeners;
    int m_nextListenerId;

    static const OutputPort* GetSourceOutputPort(const InputPort* inputPort);
};

#endif // HOST_IMPL_H
