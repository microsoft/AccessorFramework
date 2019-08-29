// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HostHypervisorImpl.h"
#include <future>
#include <mutex>

HostHypervisor::Impl::Impl()
{
    std::atomic_init<int>(&m_nextHostId, 0);
}

HostHypervisor::Impl::~Impl() = default;

int HostHypervisor::Impl::AddHost(std::unique_ptr<Host> host)
{
    int hostId = this->m_nextHostId++;
    this->m_hosts.emplace(hostId, std::move(host));
    return hostId;
}

void HostHypervisor::Impl::RemoveHost(int hostId)
{
    this->m_hosts.erase(hostId);
}

std::string HostHypervisor::Impl::GetHostName(int hostId) const
{
    return this->m_hosts.at(hostId)->GetName();
}

Host::State HostHypervisor::Impl::GetHostState(int hostId) const
{
    return this->m_hosts.at(hostId)->GetState();
}

void HostHypervisor::Impl::SetupHost(int hostId) const
{
    this->m_hosts.at(hostId)->Setup();
}

void HostHypervisor::Impl::PauseHost(int hostId) const
{
    this->m_hosts.at(hostId)->Pause();
}

void HostHypervisor::Impl::RunHost(int hostId) const
{
    this->m_hosts.at(hostId)->Run();
}

void HostHypervisor::Impl::RemoveAllHosts()
{
    this->m_hosts.clear();
}

std::map<int, std::string> HostHypervisor::Impl::GetHostNames() const
{
    return this->RunMethodOnAllHostsWithResult<std::string>(&HostHypervisor::Impl::GetHostName);
}

std::map<int, Host::State> HostHypervisor::Impl::GetHostStates() const
{
    return this->RunMethodOnAllHostsWithResult<Host::State>(&HostHypervisor::Impl::GetHostState);
}

void HostHypervisor::Impl::SetupHosts() const
{
    this->RunMethodOnAllHosts(&HostHypervisor::Impl::SetupHost);
}

void HostHypervisor::Impl::PauseHosts() const
{
    this->RunMethodOnAllHosts(&HostHypervisor::Impl::PauseHost);
}

void HostHypervisor::Impl::RunHosts() const
{
    this->RunMethodOnAllHosts(&HostHypervisor::Impl::RunHost);
}

void HostHypervisor::Impl::RunHostsOnCurrentThread() const
{
    for (auto it = ++this->m_hosts.begin(); it != this->m_hosts.end(); ++it)
    {
        it->second->Run();
    }

    this->m_hosts.begin()->second->RunOnCurrentThread();
}

void HostHypervisor::Impl::RunMethodOnAllHosts(std::function<void(const HostHypervisor::Impl&, int)> hypervisorMethod) const
{
    std::vector<std::future<void>> tasks;
    for (auto it = this->m_hosts.begin(); it != this->m_hosts.end(); ++it)
    {
        tasks.emplace_back(std::async(
            std::launch::async,
            [this, &hypervisorMethod](int hostId)
            {
                hypervisorMethod(*this, hostId);
            },
            it->first));
    }

    while (!tasks.empty())
    {
        tasks.back().wait();
        tasks.pop_back();
    }
}