// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef ATOMIC_ACCESSOR_IMPL_H
#define ATOMIC_ACCESSOR_IMPL_H

#include "AccessorImpl.h"

// Description
// The AtomicAccessorImpl implements the public AtomicAccessor interface defined in Accessor.h. In addition, it exposes
// additional functionality for internal use, such as getting an setting the accessor's priority. All atomic accessors
// are given a priority that is used by the Director to help prioritize scheduled callbacks. The priority is derived
// using the causality imperitives implied by the model's port connections; in other words, we use a topological sort of
// the directed graph created by the model's connectivity information. See HostImpl and Director for more details.
//
class AtomicAccessor::Impl : public Accessor::Impl
{
public:
    Impl(
        const std::string& name,
        AtomicAccessor* container,
        const std::vector<std::string>& inputPortNames = {},
        const std::vector<std::string>& connectedOutputPortNames = {},
        const std::vector<std::string>& spontaneousOutputPortNames = {},
        std::map<std::string, std::vector<AtomicAccessor::InputHandler>> inputHandlers = {},
        std::function<void(AtomicAccessor&)> initializeFunction = nullptr,
        std::function<void(AtomicAccessor&)> fireFunction = nullptr);

    // Internal Methods
    bool IsComposite() const override;
    void Initialize() override;
    std::vector<const InputPort*> GetEquivalentPorts(const InputPort* inputPort) const;
    std::vector<const InputPort*> GetInputPortDependencies(const OutputPort* outputPort) const;
    std::vector<const OutputPort*> GetDependentOutputPorts(const InputPort* inputPort) const;
    void SetPriority(int priority);
    void ProcessInputs();

protected:
    // AtomicAccessor Methods
    void AccessorStateDependsOn(const std::string& inputPortName);
    void RemoveDependency(const std::string& inputPortName, const std::string& outputPortName);
    void RemoveDependencies(const std::string& inputPortName, const std::vector<std::string>& outputPortNames);
    void AddSpontaneousOutputPort(const std::string& portName);
    void AddSpontaneousOutputPorts(const std::vector<std::string>& portNames);
    void AddInputHandler(const std::string& inputPortName, AtomicAccessor::InputHandler handler);
    void AddInputHandlers(const std::string& inputPortName, const std::vector<AtomicAccessor::InputHandler>& handlers);

private:
    friend class AtomicAccessor;

    void FindEquivalentPorts(const InputPort* inputPort, std::set<const InputPort*>& equivalentPorts, std::set<const OutputPort*>& dependentPorts) const;
    void InvokeInputHandlers(const std::string& inputPortName);

    AtomicAccessor* const m_container;
    std::map<const InputPort*, std::set<const OutputPort*>> m_forwardPrunedDependencies;
    std::map<const OutputPort*, std::set<const InputPort*>> m_backwardPrunedDependencies;
    std::map<std::string, std::vector<AtomicAccessor::InputHandler>> m_inputHandlers;
    std::function<void(AtomicAccessor&)> m_initializeFunction;
    std::function<void(AtomicAccessor&)> m_fireFunction;
    bool m_initialized;
    bool m_stateDependsOnInputPort;
};

#endif // ATOMIC_ACCESSOR_IMPL_H
