#pragma once

#define static_unreachable(...) \
	enum FalseType \
	{ \
		False \
	}; \
	static_assert(False, ##__VA_ARGS__);
