#pragma once

#include <future>
#include <Common/Threading/Thread.h>

#define TEST_TIMEOUT_BEGIN \
	std::promise<bool> promisedFinished; \
	auto futureResult = promisedFinished.get_future(); \
	Threading::Thread thread([&](std::promise<bool>& finished) \
	{

#define TEST_TIMEOUT_FAIL_END(X) \
	finished.set_value(true); \
	}, std::ref(promisedFinished)); \
	EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(X)) != std::future_status::timeout); \
	if (!thread.Join()) \
		thread.ForceKill();

#define TEST_TIMEOUT_SUCCESS_END(X) \
	finished.set_value(true); \
	}, std::ref(promisedFinished)); \
	EXPECT_FALSE(futureResult.wait_for(std::chrono::milliseconds(X)) != std::future_status::timeout); \
	if (!thread.Join()) \
		thread.ForceKill();
