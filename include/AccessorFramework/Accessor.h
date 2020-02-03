// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef ACCESSOR_H
#define ACCESSOR_H

#include "Event.h"
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

// Description
// An accessor is an actor that wraps a (possibly remote) device or service in an actor interface. An accessor can
// possess input ports and connected output ports (i.e. output ports that are causally dependent on, or are connected to,
// an input port on the same accessor). All accessors are able to connect their own input ports to their own output ports
// (i.e. feedforward) and their own output ports to their own input ports (i.e. feedback). All accessors can also
// schedule callbacks, or functions, that either occur as soon as possible or at a later scheduled time. Lastly, all
// accessors and their ports are given names. Port names are unique to their accessor; no two ports on a given accessor
// can have the same name.
//
// The Accessor class has two subtypes: AtomicAccessor and CompositeAccessor. In addition to input and connected output
// ports, an atomic accessor can also possess spontaneous output ports, or output ports that do not depend on input from
// any input port. For example, an output port that sends out a sensor reading every 5 seconds is a spontaneous output
// port. An atomic accessor can also contain code that handles, or reacts to, input on its input ports. Input handlers
// are functions that react to input on a specific input port. A single input port can be associated with multiple input
// handlers. Atomic accessors also have a Fire() function that is invoked once regardless of which input port receives
// input or how many input ports recieve input. This invocation occurs after all input handlers have been called.
// Composite accessors do NOT possess spontaneous output ports or any input handling logic. Instead, composite accessors
// contain child accessors, which can be atomic, composite, or both. Composites are responsible for connecting its
// children to itself and to each other and for enforcing name uniquness. A child accessor cannot have the same name as
// its parent, and no two accessors with the same parent can have the same name. Without any input handling logic,
// composites are purely containers for their children. This allows the Accessor Framework to be more modular by hiding
// layered subnetworks in the model behind composites.
//
class Accessor
{
public:
    class Impl;

    virtual ~Accessor();
    std::string GetName() const;
    Impl* GetImpl() const;
    static bool NameIsValid(const std::string& name);

protected:
    Accessor(std::unique_ptr<Impl> impl);

    // Called once during host setup (base implementation does nothing)
    virtual void Initialize();

    // Schedules a new callback using the deterministic temporal semantics.
    // A callback identifier is returned that can be used to clear the callback.
    int ScheduleCallback(std::function<void()> callback, int delayInMilliseconds, bool repeat);

    // Clears the callback with the given ID
    void ClearScheduledCallback(int callbackId);

    // Port names cannot be empty, and an accessor can have only one port with a given name
    bool NewPortNameIsValid(const std::string& newPortName) const;

    void AddInputPort(const std::string& portName);
    void AddInputPorts(const std::vector<std::string>& portNames);
    void AddOutputPort(const std::string& portName);
    void AddOutputPorts(const std::vector<std::string>& portNames);

    void ConnectMyInputToMyOutput(const std::string& myInputPortName, const std::string& myOutputPortName);
    void ConnectMyOutputToMyInput(const std::string& myOutputPortName, const std::string& myInputPortName);

    // Get the latest input on an input port
    IEvent* GetLatestInput(const std::string& inputPortName) const;

    // Send an event via an output port
    void SendOutput(const std::string& outputPortName, std::shared_ptr<IEvent> output);

private:
    std::unique_ptr<Impl> m_impl;
};

class CompositeAccessor : public Accessor
{
public:
    class Impl;

    CompositeAccessor(
        const std::string& name,
        const std::vector<std::string>& inputPortNames = {},
        const std::vector<std::string>& outputPortNames = {});

protected:
    CompositeAccessor(std::unique_ptr<Impl> impl);

    // Child names cannot be empty or the same as the parent's name, and a parent can have only one child with a given name
    bool NewChildNameIsValid(const std::string& newChildName) const;
    void AddChild(std::unique_ptr<Accessor> child);
    void ConnectMyInputToChildInput(const std::string& myInputPortName, const std::string& childName, const std::string& childInputPortName);
    void ConnectChildOutputToMyOutput(const std::string& childName, const std::string& childOutputPortName, const std::string& myOutputPortName);
    void ConnectChildren(
        const std::string& sourceChildName,
        const std::string& sourceChildOutputPortName,
        const std::string& destinationChildName,
        const std::string& destinationChildInputPortName);
    void ChildrenChanged(); // Call after children/connections are added or removed at runtime
};

class AtomicAccessor : public Accessor
{
public:
    class Impl;

    using InputHandler = std::function<void(IEvent* /*input*/)>;

    AtomicAccessor(
        const std::string& name,
        const std::vector<std::string>& inputPortNames = {},
        const std::vector<std::string>& outputPortNames = {},
        const std::vector<std::string>& spontaneousOutputPortNames = {},
        const std::map<std::string, std::vector<InputHandler>>& inputHandlers = {});

protected:
    // Declares an input port that changes this accessor's state
    void AccessorStateDependsOn(const std::string& inputPortName);

    // Removes a direct causal dependency between an input port and an output port on this accessor
    void RemoveDependency(const std::string& inputPortName, const std::string& outputPortName);
    void RemoveDependencies(const std::string& inputPortName, const std::vector<std::string>& outputPortNames);

    // Add an output port that does not depend on input from any input port (i.e. generates outputs spontaneously)
    void AddSpontaneousOutputPort(const std::string& portName);
    void AddSpontaneousOutputPorts(const std::vector<std::string>& portNames);

    // Register a function that is called when an input port receives an input
    void AddInputHandler(const std::string& inputPortName, InputHandler handler);
    void AddInputHandlers(const std::string& inputPortName, const std::vector<InputHandler>& handlers);

    // Called once per reaction (base implementation does nothing)
    virtual void Fire();
};

#endif //ACCESSOR_H
