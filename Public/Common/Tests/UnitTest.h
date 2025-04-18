#pragma once

#include <Common/Assert/Assert.h>

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

namespace ngine::Tests
{
	struct UnitTest : public testing::Test
	{
#if ENABLE_ASSERTS
		inline static int AssertIdentifier{0};
		static void EnableAssertDetection()
		{
			Internal::AssertEvents::GetInstance().AddAssertListener(
				[](const char* file, const uint32 lineNumber, [[maybe_unused]] const bool isFirstTime, const char* message, void*)
				{
					GTEST_MESSAGE_AT_(file, lineNumber, message, ::testing::TestPartResult::kNonFatalFailure);
				},
				&AssertIdentifier
			);
		}

		static void DisableAssertDetection()
		{
			Internal::AssertEvents::GetInstance().RemoveAssertListener(&AssertIdentifier);
		}

		static void SetUpTestSuite()
		{
			EnableAssertDetection();
		}

		static void TearDownTestSuite()
		{
			DisableAssertDetection();
		}
#endif

		// Per-test setup
		void SetUp() override
		{
		}

		// Per-test cleanup
		void TearDown() override
		{
		}
	};

#define UNIT_TEST(category, name) \
	GTEST_TEST_(category, name, ngine::Tests::UnitTest, ::testing::internal::GetTypeId<ngine::Tests::UnitTest>())
}
