#pragma once

#include <Common/Threading/ThreadId.h>
#include <Common/Assert/Assert.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/Math/Ratio.h>

#if USE_PTHREAD
#include <pthread.h>
#include <thread>
#elif USE_WEB_WORKERS
#include <emscripten/wasm_worker.h>
#else
#include <thread>
#endif

#include <Common/Threading/Sleep.h>
#include <Common/Memory/Containers/FlatString.h>
#include <Common/Memory/Tuple.h>
#include <Common/Memory/CallFunctionWithTuple.h>
#include <Common/Memory/CallMemberFunctionWithTuple.h>

namespace ngine::Threading
{
	inline static constexpr Math::Ratiof LowestPriorityPercentage = 0.f;
	inline static constexpr Math::Ratiof IdlePriorityPercentage = 0.125f;
	inline static constexpr Math::Ratiof LowPriorityPercentage = 0.375f;
	inline static constexpr Math::Ratiof NormalPriorityPercentage = 0.625f;
	inline static constexpr Math::Ratiof HighPriorityPercentage = 0.875f;
	inline static constexpr Math::Ratiof HighestPriorityPercentage = 1.f;

	struct ThreadId;

	struct Thread
	{
#if USE_WEB_WORKERS
		inline static constexpr size StackSize = 16777216;
#endif

		Thread() = default;
		template<typename Function, typename... FunctionArguments>
		explicit Thread(Function&& function, FunctionArguments&&... args)
#if USE_PTHREAD || USE_WEB_WORKERS
		{
			struct Arguments
			{
				Function m_function;
				Tuple<FunctionArguments...> m_arguments;
			};

#if USE_PTHREAD
			using StartThread = void* (*)(void*);
			static StartThread startThread = [](void* pGenericArguments) -> void*
			{
				Arguments* pArguments = reinterpret_cast<Arguments*>(pGenericArguments);
				CallFunctionWithTuple(Move(pArguments->m_function), Move(pArguments->m_arguments));
				delete pArguments;
				return nullptr;
			};
			pthread_create(&m_thread, nullptr, startThread, new Arguments{Forward<Function>(function), Forward<FunctionArguments>(args)...});
#elif USE_WEB_WORKERS
			union ArgumentsUnion
			{
				Arguments* pArguments;
				struct
				{
					int arg1;
					int arg2;
				};
			};

			m_thread = emscripten_malloc_wasm_worker(StackSize);

			using StartThread = void (*)(const int arg1, const int arg2);
			static StartThread startThread = [](const int arg1, const int arg2)
			{
				ArgumentsUnion arguments;
				arguments.arg1 = arg1;
				arguments.arg2 = arg2;
				CallFunctionWithTuple(Move(arguments.pArguments->m_function), Move(arguments.pArguments->m_arguments));
				delete arguments.pArguments;
				return nullptr;
			};
			ArgumentsUnion arguments;
			arguments.pArguments = new Arguments{Forward<Function>(function), Forward<FunctionArguments>(args)...};
			emscripten_wasm_worker_post_function_vii(m_thread, startThread, arguments.arg1, arguments.arg2);
#endif
		}
#else
			: m_thread(function, Forward<FunctionArguments>(args)...)
		{
		}
#endif
		template<typename ObjectType, typename... FunctionArguments>
		explicit Thread(void (ObjectType::*memberFunction)(FunctionArguments...), ObjectType& object, FunctionArguments&&... args)
#if USE_PTHREAD || USE_WEB_WORKERS
		{
			struct Arguments
			{
				void (ObjectType::*m_memberFunction)(FunctionArguments...);
				ObjectType& m_object;
				Tuple<FunctionArguments...> m_arguments;
			};

#if USE_PTHREAD
			using StartThread = void* (*)(void*);
			static StartThread startThread = [](void* pGenericArguments) -> void*
			{
				Arguments* pArguments = reinterpret_cast<Arguments*>(pGenericArguments);
				CallMemberFunctionWithTuple(pArguments->m_object, pArguments->m_memberFunction, Move(pArguments->m_arguments));
				delete pArguments;
				return nullptr;
			};
			pthread_create(&m_thread, nullptr, startThread, new Arguments{memberFunction, object, Forward<FunctionArguments>(args)...});
#elif USE_WEB_WORKERS
			union ArgumentsUnion
			{
				Arguments* pArguments;
				struct
				{
					int arg1;
					int arg2;
				};
			};

			m_thread = emscripten_malloc_wasm_worker(StackSize);

			using StartThread = void (*)(const int arg1, const int arg2);
			static StartThread startThread = [](const int arg1, const int arg2)
			{
				ArgumentsUnion arguments;
				arguments.arg1 = arg1;
				arguments.arg2 = arg2;
				CallMemberFunctionWithTuple(
					arguments.pArguments->m_object,
					arguments.pArguments->m_memberFunction,
					Move(arguments.pArguments->m_arguments)
				);
				delete arguments.pArguments;
			};
			ArgumentsUnion arguments;
			arguments.pArguments = new Arguments{memberFunction, object, Forward<FunctionArguments>(args)...};
			emscripten_wasm_worker_post_function_vii(m_thread, startThread, arguments.arg1, arguments.arg2);
#endif
		}
#else
			: m_thread(memberFunction, &object, Forward<FunctionArguments>(args)...)
		{
		}
#endif

		~Thread()
		{
#if USE_PTHREAD
			pthread_join(m_thread, nullptr);
#elif USE_WEB_WORKERS
			Assert(!IsRunningOnThread(), "Worker cannot terminate itself!");
			emscripten_terminate_wasm_worker(m_thread);
#else
			if (m_thread.joinable())
			{
				m_thread.join();
			}
#endif
		}

		template<typename ObjectType, typename... FunctionArguments>
		void Start(void (ObjectType::*memberFunction)(FunctionArguments...), ObjectType& object, FunctionArguments&&... args)
		{
#if USE_PTHREAD || USE_WEB_WORKERS
			struct Arguments
			{
				void (ObjectType::*m_memberFunction)(FunctionArguments...);
				ObjectType& m_object;
				Tuple<FunctionArguments...> m_arguments;
			};
#endif

#if USE_PTHREAD
			using StartThread = void* (*)(void*);
			static StartThread startThread = [](void* pGenericArguments) -> void*
			{
				Arguments* pArguments = reinterpret_cast<Arguments*>(pGenericArguments);
				CallMemberFunctionWithTuple(pArguments->m_object, pArguments->m_memberFunction, Move(pArguments->m_arguments));
				delete pArguments;
				return nullptr;
			};
			pthread_create(&m_thread, nullptr, startThread, new Arguments{memberFunction, object, Forward<FunctionArguments>(args)...});
#elif USE_WEB_WORKERS
			union ArgumentsUnion
			{
				Arguments* pArguments;
				struct
				{
					int arg1;
					int arg2;
				};
			};

			m_thread = emscripten_malloc_wasm_worker(StackSize);

			using StartThread = void (*)(const int arg1, const int arg2);
			static StartThread startThread = [](const int arg1, const int arg2)
			{
				ArgumentsUnion arguments;
				arguments.arg1 = arg1;
				arguments.arg2 = arg2;
				CallMemberFunctionWithTuple(
					arguments.pArguments->m_object,
					arguments.pArguments->m_memberFunction,
					Move(arguments.pArguments->m_arguments)
				);
				delete arguments.pArguments;
			};
			ArgumentsUnion arguments;
			arguments.pArguments = new Arguments{memberFunction, object, Forward<FunctionArguments>(args)...};
			emscripten_wasm_worker_post_function_vii(m_thread, startThread, arguments.arg1, arguments.arg2);
#else
			m_thread = std::thread(memberFunction, &object, Forward<FunctionArguments>(args)...);
#endif
		}

		template<typename Function, typename... FunctionArguments>
		void Start(Function&& function, FunctionArguments&&... args)
		{
#if USE_PTHREAD || USE_WEB_WORKERS
			struct Arguments
			{
				Function m_function;
				Tuple<FunctionArguments...> m_arguments;
			};
#endif

#if USE_PTHREAD
			using StartThread = void* (*)(void*);
			static StartThread startThread = [](void* pGenericArguments) -> void*
			{
				Arguments* pArguments = reinterpret_cast<Arguments*>(pGenericArguments);
				CallFunctionWithTuple(Move(pArguments->m_function), Move(pArguments->m_arguments));
				delete pArguments;
				return nullptr;
			};
			pthread_create(&m_thread, nullptr, startThread, new Arguments{Forward<Function>(function), Forward<FunctionArguments>(args)...});
#elif USE_WEB_WORKERS
			union ArgumentsUnion
			{
				Arguments* pArguments;
				struct
				{
					int arg1;
					int arg2;
				};
			};

			m_thread = emscripten_malloc_wasm_worker(StackSize);

			using StartThread = void (*)(const int arg1, const int arg2);
			static StartThread startThread = [](const int arg1, const int arg2)
			{
				ArgumentsUnion arguments;
				arguments.arg1 = arg1;
				arguments.arg2 = arg2;
				CallFunctionWithTuple(Move(arguments.pArguments->m_function), Move(arguments.pArguments->m_arguments));
				delete arguments.pArguments;
			};
			ArgumentsUnion arguments;
			arguments.pArguments = new Arguments{Forward<Function>(function), Forward<FunctionArguments>(args)...};
			emscripten_wasm_worker_post_function_vii(m_thread, startThread, arguments.arg1, arguments.arg2);
#else
			m_thread = std::thread(function, Forward<FunctionArguments>(args)...);
#endif
		}

		void InitializeExternallyCreatedFromThread()
		{
#if USE_PTHREAD
			m_thread = pthread_self();
#elif USE_WEB_WORKERS
			m_thread = emscripten_wasm_worker_self_id();
#endif
		}

		bool Join()
		{
#if USE_PTHREAD
			return pthread_join(m_thread, nullptr) == 0;
#elif USE_WEB_WORKERS
			Assert(false, "Not supported");
#else
			if (m_thread.joinable())
			{
				m_thread.join();
				return true;
			}
#endif

			return false;
		}

		void Detach()
		{
#if USE_PTHREAD
			pthread_detach(m_thread);
#elif USE_WEB_WORKERS
			Assert(false, "Not supported");
#else
			m_thread.detach();
#endif
		}

		void ForceKill();

#if USE_PTHREAD
		[[nodiscard]] pthread_t GetThreadHandle() const
		{
			return m_thread;
		}
#elif USE_WEB_WORKERS
		[[nodiscard]] int GetThreadHandle() const
		{
			return m_thread;
		}
#else
		[[nodiscard]] void* GetThreadHandle() const
		{
			return m_thread.native_handle();
		}
#endif
		[[nodiscard]] PURE_STATICS ThreadId GetThreadId() const;
		[[nodiscard]] bool IsRunningOnThread() const
		{
			return ThreadId::GetCurrent() == GetThreadId();
		}

		void SetThreadName(const ConstNativeZeroTerminatedStringView name);
		static void SetAffinityMask(const ThreadId id, const uint64 mask);
		void SetAffinityMask(const uint64 mask);

		enum class Priority : uint8
		{
			//! Represents tasks that need to be done immediately in order to provide a nice user experience. Use it for UI updates, event
			//! handling and small workloads that require low latency.
			UserInteractive,
			//! Represents tasks that are initiated from the UI and can be performed asynchronously. It should be used when the user is waiting
			//! for immediate results, and for tasks required to continue user interaction.
			UserInitiated,
			Default,
			//! Represents long-running tasks, typically with a user-visible progress indicator. Use it for computations, I/O, networking,
			//! continous data feeds and similar tasks. This class is designed to be energy efficient.
			UserVisibleBackground,
			//! Represents tasks that the user is not directly aware of. Use it for prefetching, maintenance, and other tasks that don’t require
			//! user interaction and aren’t time-sensitive.
			Background
		};
	protected:
		void Sleep(const unsigned long milliseconds)
		{
			Assert(IsRunningOnThread());

			Threading::Sleep(milliseconds);
		}

		void SetPriority(const Priority priority, const float ratio);
		[[nodiscard]] Priority GetPriority() const
		{
			return m_priority;
		}
		[[nodiscard]] float GetPriorityRatio() const
		{
			return m_priorityRatio;
		}
	protected:
#if USE_PTHREAD
		pthread_t m_thread;
#elif USE_WEB_WORKERS
		int m_thread;
#else
		mutable std::thread m_thread;
#endif
		Priority m_priority = Priority::Default;
		float m_priorityRatio = 1.f;
	};

	template<typename JobType>
	struct ThreadWithRunMember : private Thread
	{
		void Start(const ConstNativeStringView name)
		{
			Thread::Start(&ThreadWithRunMember::RunInternal, *this, FlatNativeString<48>(name));
		}

		void RunInternal(const FlatNativeString<48> name)
		{
			SetThreadName(name.GetZeroTerminated());

			static_cast<JobType*>(this)->Run();
		}

		using Priority = Thread::Priority;

		using Thread::InitializeExternallyCreatedFromThread;
		using Thread::Join;
		using Thread::Detach;
		using Thread::SetPriority;
		using Thread::GetPriority;
		using Thread::GetPriorityRatio;
		using Thread::GetThreadHandle;
		using Thread::GetThreadId;
		using Thread::SetAffinityMask;
		using Thread::IsRunningOnThread;
	};
}
