// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "AccessorFramework/Host.h"
#include "HostImpl.h"
#include "HostHypervisorImpl.h"
#include <thread>

Host::~Host() = default;

Host::State Host::GetState() const
{
    return static_cast<Impl*>(this->GetImpl())->GetState();
}

bool Host::EventListenerIsRegistered(int listenerId) const
{
    return static_cast<Impl*>(this->GetImpl())->EventListenerIsRegistered(listenerId);
}

int Host::AddEventListener(std::weak_ptr<Host::EventListener> listener)
{
    return static_cast<Impl*>(this->GetImpl())->AddEventListener(std::move(listener));
}

void Host::RemoveEventListener(int listenerId)
{
    static_cast<Impl*>(this->GetImpl())->RemoveEventListener(listenerId);
}

void Host::Setup()
{
    static_cast<Impl*>(this->GetImpl())->Setup();
}

void Host::Iterate(int numberOfIterations)
{
    static_cast<Impl*>(this->GetImpl())->Iterate(numberOfIterations);
}

void Host::Pause()
{
    static_cast<Impl*>(this->GetImpl())->Pause();
}

void Host::Run()
{
    static_cast<Impl*>(this->GetImpl())->Run();
}

void Host::RunOnCurrentThread()
{
    static_cast<Impl*>(this->GetImpl())->RunOnCurrentThread();
}

void Host::Exit()
{
    static_cast<Impl*>(this->GetImpl())->Exit();
}

void Host::AdditionalSetup()
{
    // base implementation does nothing
}

Host::Host(const std::string& name) :
    CompositeAccessor(std::make_unique<Impl>(name, this))
{
}

HostHypervisor::HostHypervisor() :
    m_impl(std::make_unique<Impl>())
{
}

HostHypervisor::~HostHypervisor()
{
    this->RemoveAllHosts();
}

int HostHypervisor::AddHost(std::unique_ptr<Host> host)
{
    return this->m_impl->AddHost(std::move(host));
}

void HostHypervisor::RemoveHost(int hostId)
{
    this->m_impl->RemoveHost(hostId);
}

std::string HostHypervisor::GetHostName(int hostId) const
{
    return this->m_impl->GetHostName(hostId);
}

Host::State HostHypervisor::GetHostState(int hostId) const
{
    return this->m_impl->GetHostState(hostId);
}

void HostHypervisor::SetupHost(int hostId) const
{
    this->m_impl->SetupHost(hostId);
}

void HostHypervisor::PauseHost(int hostId) const
{
    this->m_impl->PauseHost(hostId);
}

void HostHypervisor::RunHost(int hostId) const
{
    this->m_impl->RunHost(hostId);
}

void HostHypervisor::RemoveAllHosts()
{
    this->m_impl->RemoveAllHosts();
}

std::map<int, std::string> HostHypervisor::GetHostNames() const
{
    return this->m_impl->GetHostNames();
}

std::map<int, Host::State> HostHypervisor::GetHostStates() const
{
    return this->m_impl->GetHostStates();
}

void HostHypervisor::SetupHosts() const
{
    this->m_impl->SetupHosts();
}

void HostHypervisor::PauseHosts() const
{
    this->m_impl->PauseHosts();
}

void HostHypervisor::RunHosts() const
{
    this->m_impl->RunHosts();
}

void HostHypervisor::RunHostsOnCurrentThread() const
{
    this->m_impl->RunHostsOnCurrentThread();
}