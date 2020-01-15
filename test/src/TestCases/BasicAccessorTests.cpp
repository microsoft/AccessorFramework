#include <gtest/gtest.h>
#include <AccessorFramework/Accessor.h>

namespace BasicAccessorTests
{
    // A name cannot be empty, cannot contain periods, and cannot contain whitespace
    TEST(Accessor_NameIsValidTests, ValidName)
    {
        // Arrange
        const std::string targetName = "TargetName";

        // Act
        bool nameIsValid = Accessor::NameIsValid(targetName);

        // Assert
        ASSERT_TRUE(nameIsValid);
    }

    TEST(Accessor_NameIsValidTests, EmptyName)
    {
        // Arrange
        const std::string targetName = "";

        // Act
        bool nameIsValid = Accessor::NameIsValid(targetName);

        // Assert
        ASSERT_FALSE(nameIsValid);
    }

    TEST(Accessor_NameIsValidTests, NameWithPeriods)
    {
        // Arrange
        const std::string targetName = "Target.Name";

        // Act
        bool nameIsValid = Accessor::NameIsValid(targetName);

        // Assert
        ASSERT_FALSE(nameIsValid);
    }

    TEST(Accessor_NameIsValidTests, NameWithWhitespace)
    {
        // Arrange
        const std::string targetName = "Target Name";

        // Act
        bool nameIsValid = Accessor::NameIsValid(targetName);

        // Assert
        ASSERT_FALSE(nameIsValid);
    }

    TEST(Accessor_GetNameTests, ValidName)
    {
        // Arrange
        const std::string expectedTargetName = "TargetName";
        AtomicAccessor target(expectedTargetName);

        // Act
        std::string actualTargetName = target.GetName();

        // Assert
        ASSERT_EQ(expectedTargetName, actualTargetName);
    }

    TEST(Accessor_GetImplTests, NotNull)
    {
        // Arrange
        const std::string targetName = "TargetName";
        AtomicAccessor target(targetName);

        // Act
        Accessor::Impl* targetImpl = target.GetImpl();

        // Assert
        ASSERT_NE(nullptr, targetImpl);
    }
}