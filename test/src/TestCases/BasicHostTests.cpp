// Copyright(c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <AccessorFramework/Host.h>
#include "../TestClasses/EmptyHost.h"

namespace BasicHostTests
{
    class HostTest : public ::testing::Test
    {
    protected:
        // Runs before each test case
        void SetUp() override
        {
            this->target = std::make_unique<EmptyHost>(this->TargetName);
        }

        // Runs after each test case
        void TearDown() override
        {
            this->target.reset(nullptr);
        }

        std::unique_ptr<EmptyHost> target = nullptr;
        std::string TargetName = "TargetHost";
    };

    TEST_F(HostTest, GetName)
    {
        // Act
        std::string actualTargetName = target->GetName();

        // Assert
        ASSERT_STREQ(TargetName.c_str(), actualTargetName.c_str());
    }

    TEST_F(HostTest, CannotRunWithoutSetup)
    {
        // Act
        Host::State initialState = target->GetState();

        // Assert
        ASSERT_EQ(Host::State::NeedsSetup, initialState);
        ASSERT_THROW(target->Run(), std::logic_error);
        ASSERT_THROW(target->RunOnCurrentThread(), std::logic_error);
        ASSERT_THROW(target->Iterate(1), std::logic_error);
        ASSERT_THROW(target->Pause(), std::logic_error);
    }

    TEST_F(HostTest, SetupEmpty)
    {
        // Act
        target->Setup();
        Host::State stateAfterSetup = target->GetState();
        bool additionalSetupWasCalled = target->AdditionalSetupWasCalled();

        // Assert
        ASSERT_EQ(Host::State::ReadyToRun, stateAfterSetup);
        ASSERT_TRUE(additionalSetupWasCalled);
    }

    TEST_F(HostTest, CanExitWithoutSetup)
    {
        // Act
        Host::State initialState = target->GetState();
        target->Exit();
        Host::State finalState = target->GetState();

        // Assert
        ASSERT_EQ(Host::State::NeedsSetup, initialState);
        ASSERT_EQ(Host::State::Finished, finalState);
    }

    TEST_F(HostTest, CanExitWithoutRunning)
    {
        // Act
        target->Setup();
        Host::State stateAfterSetup = target->GetState();
        target->Exit();
        Host::State finalState = target->GetState();

        // Assert
        ASSERT_EQ(Host::State::ReadyToRun, stateAfterSetup);
        ASSERT_EQ(Host::State::Finished, finalState);
    }
}