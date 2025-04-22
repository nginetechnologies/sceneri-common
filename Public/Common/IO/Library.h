#pragma once

#include <Common/Memory/Containers/ZeroTerminatedStringView.h>
#include <Common/IO/ForwardDeclarations/ZeroTerminatedPathView.h>
#include <Common/IO/PathView.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::IO
{
	struct Path;

	struct TRIVIAL_ABI LibraryView
	{
		inline static constexpr bool IsDirectory = PLATFORM_APPLE;

#if PLATFORM_WINDOWS
		inline static constexpr PathView FileNamePrefix = MAKE_PATH("");
		inline static constexpr PathView FileNamePostfix = MAKE_PATH(".dll");
		inline static constexpr PathView ExecutablePostfix = MAKE_PATH(".exe");
		inline static constexpr PathView ApplicationPostfix = ExecutablePostfix;
#elif PLATFORM_APPLE
		inline static constexpr PathView FileNamePrefix = MAKE_PATH("");
		inline static constexpr PathView FileNamePostfix = MAKE_PATH(".bundle");
		inline static constexpr PathView ExecutablePostfix = MAKE_PATH("");
		inline static constexpr PathView ApplicationPostfix = MAKE_PATH(".app");
#else
		inline static constexpr PathView FileNamePrefix = MAKE_PATH("lib");
		inline static constexpr PathView FileNamePostfix = MAKE_PATH(".so");
		inline static constexpr PathView ExecutablePostfix = MAKE_PATH("");
		inline static constexpr PathView ApplicationPostfix = MAKE_PATH("");
#endif

		LibraryView() = default;
		LibraryView(const LibraryView&) = default;
		LibraryView& operator=(const LibraryView&) = default;
		LibraryView(LibraryView&& other) noexcept
			: m_pModuleHandle(other.m_pModuleHandle)
		{
			other.m_pModuleHandle = nullptr;
		}
		LibraryView& operator=(LibraryView&& other) noexcept
		{
			m_pModuleHandle = other.m_pModuleHandle;
			other.m_pModuleHandle = nullptr;
			return *this;
		}

		template<typename Procedure>
		Procedure GetProcedureAddress(const ZeroTerminatedStringView name) const
		{
			return reinterpret_cast<Procedure>(GetProcedureAddressInternal(name));
		}

		[[nodiscard]] bool IsValid() const
		{
			return m_pModuleHandle != nullptr;
		}
	protected:
		LibraryView(void* pModuleHandle)
			: m_pModuleHandle(pModuleHandle)
		{
		}

		void* GetProcedureAddressInternal(const ZeroTerminatedStringView name) const LIFETIME_BOUND;
	protected:
		void* m_pModuleHandle = nullptr;
	};

	struct TRIVIAL_ABI Library : public LibraryView
	{
		Library()
		{
		}
		Library(const IO::ConstZeroTerminatedPathView path);
		Library(const Library&) = delete;
		Library& operator=(const Library&) = delete;
		Library(Library&& other) noexcept = default;
		Library& operator=(Library&& other) noexcept = default;
		~Library();
	};
}
