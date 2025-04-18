#pragma once

#include "Common/Platform/Assume.h"
#include "Common/Platform/Unreachable.h"
#include "Common/Platform/NoDebug.h"

#if ENABLE_ASSERTS
#include "Common/Platform/IsDebuggerAttached.h"
#include "Common/Platform/IsConstantEvaluated.h"
#include "Common/Platform/IsConstant.h"
#include "Common/Platform/Likely.h"
#include "Common/Memory/Forward.h"
#include "Validate.h"
#include <Common/Threading/Atomics/CompareExchangeStrong.h>
#endif

namespace ngine
{
	namespace Internal
	{
		inline bool FailedConstantEvaluatedAssert()
		{
			return true;
		}

#if ENABLE_ASSERTS
		struct AssertEvents
		{
			using EventFunction =
				void (*)(const char* file, const uint32 lineNumber, const bool isFirstTime, const char* message, void* pUserData);

			~AssertEvents();

			void NO_INLINE UNLIKELY_ERROR_SECTION COLD_FUNCTION NO_DEBUG
			OnAssert(const char* file, const uint32 lineNumber, const bool isFirstTime);
			void NO_INLINE UNLIKELY_ERROR_SECTION COLD_FUNCTION NO_DEBUG
			OnAssert(const char* file, const uint32 lineNumber, const bool isFirstTime, const char* message);
			void AddAssertListener(EventFunction&& function, void* pUserData);
			void RemoveAssertListener(void* pUserData);

			[[nodiscard]] NO_DEBUG static AssertEvents& GetInstance();
		protected:
			struct Listener
			{
				EventFunction function;
				void* pUserData;
			};
			Listener* m_pListeners{nullptr};
			uint32 m_count{0};
			uint32 m_capacity{0};
		};
#endif
	}

#if DEBUG_BUILD && ENABLE_ASSERTS
#define Assert(condition, ...) \
	{ \
		auto checkFirstTime = []() \
		{ \
			static bool wasHit = false; \
			bool expected = false; \
			return ngine::Threading::Atomics::CompareExchangeStrong(wasHit, expected, true); \
		}; \
		if (ngine::IsConstantEvaluated()) \
		{ \
			(static_cast<bool>(condition) ? true : ngine::Internal::FailedConstantEvaluatedAssert()); \
		} \
		/*else if constexpr (IS_CONSTANT(condition)) \
		{ \
		  if (!(condition)) \
		  { \
		    static_unreachable(); \
		  } \
		}*/ \
		else if (UNLIKELY(!static_cast<bool>((void)0, condition))) \
		{ \
			const bool isFirstTime = checkFirstTime(); \
			if (isFirstTime) \
			{ \
				BreakIfDebuggerIsAttached(); \
			} \
			ngine::Internal::AssertEvents::GetInstance().OnAssert(__FILE__, __LINE__, isFirstTime, ##__VA_ARGS__); \
		} \
	}
#elif ENABLE_ASSERTS
#define Assert(condition, ...) \
	{ \
		if (ngine::IsConstantEvaluated()) \
		{ \
			(static_cast<bool>(condition) ? true : ngine::Internal::FailedConstantEvaluatedAssert()); \
		} \
		else if (UNLIKELY(!static_cast<bool>((void)0, condition))) \
		{ \
			const bool isFirstTime = COLD_ERROR_LOGIC( \
				[]() \
				{ \
					static bool wasHit = false; \
					bool expected = false; \
					const bool isFirstTime = ngine::Threading::Atomics::CompareExchangeStrong(wasHit, expected, true); \
					if (isFirstTime) \
					{ \
						BreakIfDebuggerIsAttached(); \
					} \
					return isFirstTime; \
				} \
			); \
			ngine::Internal::AssertEvents::GetInstance().OnAssert(__FILE__, __LINE__, isFirstTime, ##__VA_ARGS__); \
		} \
	}
#else
#define Assert(...)
#endif

#if DEBUG_BUILD && ENABLE_ASSERTS
#define AssertMessage(condition, ...) \
	{ \
		auto checkFirstTime = []() \
		{ \
			static bool wasHit = false; \
			bool expected = false; \
			return ngine::Threading::Atomics::CompareExchangeStrong(wasHit, expected, true); \
		}; \
		if (ngine::IsConstantEvaluated()) \
		{ \
			(static_cast<bool>(condition) ? true : ngine::Internal::FailedConstantEvaluatedAssert()); \
		} \
		/*else if constexpr (IS_CONSTANT(condition)) \
		{ \
		  if (!(condition)) \
		  { \
		    static_unreachable(); \
		  } \
		}*/ \
		else if (UNLIKELY(!static_cast<bool>((void)0, condition))) \
		{ \
			const bool isFirstTime = checkFirstTime(); \
			if (isFirstTime) \
			{ \
				BreakIfDebuggerIsAttached(); \
			} \
			ngine::Internal::AssertEvents::GetInstance() \
				.OnAssert(__FILE__, __LINE__, isFirstTime, String().Format(__VA_ARGS__).GetZeroTerminated()); \
		} \
	}
#elif ENABLE_ASSERTS
#define AssertMessage(condition, ...) \
	{ \
		if (ngine::IsConstantEvaluated()) \
		{ \
			(static_cast<bool>(condition) ? true : ngine::Internal::FailedConstantEvaluatedAssert()); \
		} \
		else if (UNLIKELY(!static_cast<bool>((void)0, condition))) \
		{ \
			const bool isFirstTime = COLD_ERROR_LOGIC( \
				[]() \
				{ \
					static bool wasHit = false; \
					bool expected = false; \
					const bool isFirstTime = ngine::Threading::Atomics::CompareExchangeStrong(wasHit, expected, true); \
					if (isFirstTime) \
					{ \
						BreakIfDebuggerIsAttached(); \
					} \
					return isFirstTime; \
				} \
			); \
			ngine::Internal::AssertEvents::GetInstance() \
				.OnAssert(__FILE__, __LINE__, isFirstTime, String().Format(__VA_ARGS__).GetZeroTerminated()); \
		} \
	}
#else
#define AssertMessage(...)
#endif

#if ENABLE_ASSERTS
#define InternalAssertConstant(x) ((ngine::IsConstantEvaluated() && x) || ngine::Internal::FailedConstantEvaluatedAssert())
#define InternalAssert(x, ...) \
	(LIKELY(x) || ([file = __FILE__, line = __LINE__]() NO_INLINE UNLIKELY_ERROR_SECTION COLD_FUNCTION { \
		static bool wasHit = false; \
		bool expected = false; \
		bool isFirstTime = ngine::Threading::Atomics::CompareExchangeStrong(wasHit, expected, true); \
		ngine::Internal::AssertEvents::GetInstance().OnAssert(file, line, isFirstTime, ##__VA_ARGS__); \
		return !isFirstTime; \
	}() && (!IsDebuggerAttached() || BreakIntoDebugger())))

#define Ensure(x, ...) \
	const bool _cond = (x); \
	InternalAssertConstant(_cond) || InternalAssert(_cond), LIKELY(_cond)
#else
#define Ensure(x, ...) LIKELY(x)
#endif

// Used to indicate that a code block is not yet implemented / intended to be hit
#define NotImplemented(...) Assert(false, ##__VA_ARGS__)
// Used to indicate that a code block is not supported / intended to be hit
#define NotSupported(...) Assert(false, ##__VA_ARGS__)

#define Expect(condition, ...) \
	Assert(condition, ##__VA_ARGS__); \
	ASSUME(condition);

#define ExpectUnreachable(...) \
	Assert(false, ##__VA_ARGS__); \
	UNREACHABLE;
}
