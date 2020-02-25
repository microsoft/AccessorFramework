// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef HOST_H
#define HOST_H

#include "Accessor.h"
#include <memory>
#include <string>

// Description
// The host contains and drives the accessor model. It can be thought of as a composite accessor without any input or
// output ports with the ability to set up, run, pause, and tear down the model. It also maintains the model's state and
// passes along any exceptions thrown by the model. It defines an EventListener interface so that other entities can
// subscribe to be notified when the model changes state or throws an exception.
//
// Note: Hosts are not allowed to have ports; calls to inherited Add__Port() methods will throw an exception.
//
class Host : public CompositeAccessor
{
public:
    class Impl;

    enum class State
    {
        NeedsSetup,
        SettingUp,
        ReadyToRun,
        Running,
        Paused,
        Exiting,
        Finished,
        Corrupted
    };

    class EventListener
    {
    public:
        virtual void NotifyOfException(const std::exception& e) = 0;
        virtual void NotifyOfStateChange(Host::State oldState, Host::State newState) = 0;
    };

    ~Host();
    State GetState() const;
    bool EventListenerIsRegistered(int listenerId) const;
    int AddEventListener(std::weak_ptr<EventListener> listener);
    void RemoveEventListener(int listenerId);
    void Setup();
    void Iterate(int numberOfIterations = 1);
    void Pause();
    void Run();
    void RunOnCurrentThread();
    void Exit();

protected:
    Host(const std::string& name);

    // Called during Setup() (base implementation does nothing)
    virtual void AdditionalSetup();

private:
    // Hosts are not allowed to have ports, so we make them private
    // These methods will throw if made public and used by a derived class
    using CompositeAccessor::AddInputPort;
    using CompositeAccessor::AddInputPorts;
    using CompositeAccessor::AddOutputPort;
    using CompositeAccessor::AddOutputPorts;
};

class HostHypervisor
{
public:
    HostHypervisor();
    ~HostHypervisor();

    int AddHost(std::unique_ptr<Host> host);
    void RemoveHost(int hostId);
    std::string GetHostName(int hostId) const;
    Host::State GetHostState(int hostId) const;
    void SetupHost(int hostId) const;
    void PauseHost(int hostId) const;
    void RunHost(int hostId) const;

    void RemoveAllHosts();
    std::map<int, std::string> GetHostNames() const;
    std::map<int, Host::State> GetHostStates() const;
    void SetupHosts() const;
    void PauseHosts() const;
    void RunHosts() const;
    void RunHostsOnCurrentThread() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

#endif // HOST_H
