// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef HOST_HYPERVISOR_IMPL_H
#define HOST_HYPERVISOR_IMPL_H

#include "AccessorFramework/Host.h"
#include <atomic>
#include <map>

class HostHypervisor::Impl
{
public:
    Impl();
    ~Impl();

protected:
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
    friend class HostHypervisor;

    void RunMethodOnAllHosts(std::function<void(const HostHypervisor::Impl&, int)> hypervisorMethod) const;

    template<typename T>
    std::map<int, T> RunMethodOnAllHostsWithResult(std::function<T(const HostHypervisor::Impl&, int)> hypervisorMethod) const
    {
        std::map<int, T> results;
        std::mutex resultsMutex;
        std::vector<std::future<void>> tasks;
        for (auto it = this->m_hosts.begin(); it != this->m_hosts.end(); ++it)
        {
            tasks.emplace_back(std::async(
                std::launch::async,
                [this, &hypervisorMethod, &results, &resultsMutex](int hostId)
                {
                    T result = hypervisorMethod(*this, hostId);
                    std::lock_guard<std::mutex> lock(resultsMutex);
                    results.emplace(hostId, result);
                },
                it->first));
        }

        while (!tasks.empty())
        {
            tasks.back().wait();
            tasks.pop_back();
        }

        return results;
    }

    std::atomic_int m_nextHostId;
    std::map<int, std::unique_ptr<Host>> m_hosts;
};

#endif // HOST_HYPERVISOR_IMPL_H
