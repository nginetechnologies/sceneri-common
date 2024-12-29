#pragma once

#include <Common/Assert/Assert.h>

#define ENABLE_MATH_ASSERTS PROFILE_BUILD

#if ENABLE_MATH_ASSERTS
#define MathAssert(condition, ...) Assert(condition, ##__VA_ARGS__)
#define MathAssertMessage(condition, ...) AssertMessage(condition, ##__VA_ARGS__)
#else
#define MathAssert(condition, ...)
#define MathAssertMessage(condition, ...)
#endif

#define MathExpect(condition, ...) \
	MathAssert(condition, ##__VA_ARGS__); \
	ASSUME(condition);
