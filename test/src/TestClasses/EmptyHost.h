// Copyright(c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef EMPTYHOST_H
#define EMPTYHOST_H

#include <AccessorFramework/Host.h>

class EmptyHost : public Host
{
public:
    EmptyHost(const std::string& name) :
        Host(name)
    {
    }

    bool AdditionalSetupWasCalled()
    {
        return this->m_additionalSetupCalled;
    }

protected:
    void AdditionalSetup() override
    {
        this->m_additionalSetupCalled = true;
    }

private:
    bool m_additionalSetupCalled = false;
};

#endif // EMPTYHOST_H