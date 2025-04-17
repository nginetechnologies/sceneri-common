#pragma once

#include <Common/Platform/StaticUnreachable.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Memory/Containers/StringView.h>

namespace ngine::Platform
{
	enum class Environment : uint8
	{
		//! Development environment usually from a desktop machine
		//! This is from a local machine where a developer is making changes to the engine itself
		InternalDevelopment,
		// Development environment from a desktop machine with a local lootlocker instance running
		// This is from a local machine where a developer is making changes to the engine and the backend
		LocalBackendDevelopment,
		//! Staging environment running internal pre-release Sceneri builds
		//! This will usually be from TestFlight and similar automated CI builds
		InternalStaging,
		//! Staging environment running external pre-release Sceneri builds
		//! Release candidates and such will run here.
		Staging,
		//! Full production environment where all users can access Sceneri
		Live
	};

	[[nodiscard]] inline static constexpr Environment GetEnvironment(const ConstStringView environment)
	{
		if (environment == "internal_development")
		{
			return Environment::InternalDevelopment;
		}
		else if (environment == "local_backend_development")
		{
			return Environment::LocalBackendDevelopment;
		}
		else if (environment == "internal_staging")
		{
			return Environment::InternalStaging;
		}
		else if (environment == "staging")
		{
			return Environment::Staging;
		}
		else if (environment == "live")
		{
			return Environment::Live;
		}
		else
		{
			return Environment::Live;
		}
	}

	[[nodiscard]] inline static constexpr ConstStringView GetEnvironmentString(const Environment environment)
	{
		switch (environment)
		{
			case Environment::InternalDevelopment:
				return "internal_development";
			case Environment::LocalBackendDevelopment:
				return "local_backend_development";
			case Environment::InternalStaging:
				return "internal_staging";
			case Environment::Staging:
				return "staging";
			case Environment::Live:
				return "live";
		}
		ExpectUnreachable();
	}

	inline static constexpr Environment DefaultEnvironment = GetEnvironment(TARGET_ENVIRONMENT);

	namespace Internal
	{
		inline static constexpr bool CanSwitchEnvironmentAtRuntime = DefaultEnvironment == Environment::InternalDevelopment ||
		                                                             DefaultEnvironment == Environment::InternalStaging;
		[[nodiscard]] extern Environment& GetCurrentEnvironment();
	}

	using Internal::CanSwitchEnvironmentAtRuntime;

	[[nodiscard]] inline static Environment GetEnvironment()
	{
		if constexpr (Internal::CanSwitchEnvironmentAtRuntime)
		{
			return Internal::GetCurrentEnvironment();
		}
		else
		{
			return DefaultEnvironment;
		}
	}

	inline static void SwitchEnvironment(const Environment environment)
	{
		if constexpr (CanSwitchEnvironmentAtRuntime)
		{
			Internal::GetCurrentEnvironment() = environment;
		}
	}
}
