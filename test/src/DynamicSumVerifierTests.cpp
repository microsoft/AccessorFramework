// Copyright(c) Microsoft Corporation.
// Licensed under the MIT License.

#include <thread>
#include <gtest/gtest.h>
#include <AccessorFramework/Accessor.h>
#include <AccessorFramework/Host.h>
#include "../TestClasses/DynamicSumVerifierHost.h"

namespace DynamicSumVerifierTests
{
    class DynamicSumVerifierTest : public ::testing::Test
    {
    protected:
        // Runs before each test case
        void SetUp() override
        {
            this->latestSum = std::make_shared<int>(0);
            this->error = std::make_shared<bool>(false);
            this->target = std::make_unique<DynamicSumVerifierHost>(this->TargetName, this->latestSum, this->error);
        }

        // Runs after each test case
        void TearDown() override
        {
            this->target.reset(nullptr);
        }

        std::unique_ptr<DynamicSumVerifierHost> target = nullptr;
        std::string TargetName = "TargetHost";
        std::shared_ptr<int> latestSum = nullptr;
        std::shared_ptr<bool> error = nullptr;
    };

    TEST_F(DynamicSumVerifierTest, SetUpModel)
    {
        // Act
        target->Setup();
        Host::State stateAfterSetup = target->GetState();

        // Assert
        ASSERT_EQ(Host::State::ReadyToRun, stateAfterSetup);
    }

    TEST_F(DynamicSumVerifierTest, Iterate)
    {
        /*
        Events:
           Add: host adds another spontaneous counter to the model (should trigger an update at the beginning of the next round)
           Update: host updates the model (recalculates priorities & initializes new actors)
           Fire: spontaneous counters output their latest counts
        Expected Sequence:
           Round 0 (initialization): Add
           Rount 1: Update --> Add
           Round 2: Update --> Add --> Fire (0)
           Round 3: Update --> Add --> Fire (0 + 1)
           Round 4: Update --> Add --> Fire (0 + 1 + 2)
           Round 5: Update --> Add --> Fire (0 + 1 + 2 + 3)
           ...
           Round N: Update --> Add --> Fire (0 + 1 + 2 + 3 + ... + N) = 0.5(N-1)(N-2)
        */

        // Arrange
        int numberOfIterations = 5;
        int expectedSum = ((numberOfIterations - 1) * (numberOfIterations - 2)) / 2;

        // Act
        target->Setup();
        target->Iterate(5);

        // Assert
        ASSERT_FALSE(*error);
        ASSERT_EQ(expectedSum, *latestSum);
    }

    TEST_F(DynamicSumVerifierTest, Run)
    {
        using namespace std::chrono_literals;

        // Arrange
        auto sleepInterval = 5.5s;
        int expectedNumberOfIterations = floor(sleepInterval.count());
        int expectedSum = ((expectedNumberOfIterations - 1) * (expectedNumberOfIterations - 2)) / 2;

        // Act
        target->Setup();
        target->Run();
        std::this_thread::sleep_for(sleepInterval);
        target->Exit();

        // Assert
        ASSERT_FALSE(*error);
        ASSERT_EQ(expectedSum, *latestSum);
    }
}