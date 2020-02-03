// Copyright(c) Microsoft Corporation.
// Licensed under the MIT License.

#include <cmath>
#include <thread>
#include <gtest/gtest.h>
#include <AccessorFramework/Accessor.h>
#include <AccessorFramework/Host.h>
#include "../TestClasses/SumVerifierHost.h"

namespace SumVerifierTests
{
    class SumVerifierTest : public ::testing::Test
    {
    protected:
        // Runs before each test case
        void SetUp() override
        {
            this->latestSum = std::make_shared<int>(0);
            this->error = std::make_shared<bool>(false);
            this->target = std::make_unique<SumVerifierHost>(this->TargetName, this->latestSum, this->error);
        }

        // Runs after each test case
        void TearDown() override
        {
            this->target.reset(nullptr);
            this->latestSum.reset();
            this->error.reset();
        }

        std::string TargetName = "TargetHost";
        std::unique_ptr<SumVerifierHost> target = nullptr;
        std::shared_ptr<int> latestSum = nullptr;
        std::shared_ptr<bool> error = nullptr;
    };

    TEST_F(SumVerifierTest, SumVerifier_Iterate)
    {
        // Arrange
        int numberOfIterations = 5;
        int expectedSum = (numberOfIterations - 1) * 2;

        // Act
        target->Setup();
        target->Iterate(5);
        target->Exit();

        // Assert
        ASSERT_FALSE(*error);
        ASSERT_EQ(expectedSum, *latestSum);
    }

    TEST_F(SumVerifierTest, SumVerifier_Run)
    {
        using namespace std::chrono_literals;

        // Arrange
        auto sleepInterval = 5.5s;
        int expectedSum = (std::floor(sleepInterval.count()) - 1) * 2;

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