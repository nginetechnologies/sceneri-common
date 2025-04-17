#pragma once

#include <Common/System/SystemType.h>
#include <Common/TypeTraits/Void.h>
#include <Common/Memory/Optional.h>
#include <Common/Memory/Containers/Array.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>
#include <Common/Guid.h>

namespace ngine
{
	struct Application;
	struct Engine;
	struct Plugin;
}

namespace ngine::System
{
	namespace Internal
	{
		template<class T, class U = void>
		struct THasSystemType
		{
			inline static constexpr bool Value = false;
		};

		template<class T>
		struct THasSystemType<T, typename TypeTraits::Void<decltype(T::SystemType)>>
		{
			inline static constexpr bool Value = true;
		};

		template<class T, class U = void>
		struct THasPluginGuid
		{
			inline static constexpr bool Value = false;
		};

		template<class T>
		struct THasPluginGuid<T, typename TypeTraits::Void<decltype(T::Guid)>>
		{
			inline static constexpr bool Value = true;
		};
	}

	template<typename Type>
	inline static constexpr bool HasSystemType = Internal::THasSystemType<Type>::Value;

	template<typename Type>
	inline static constexpr bool HasPluginGuid = Internal::THasPluginGuid<Type>::Value;

	struct Query final
	{
		Query();
		Query(const Query&) = delete;
		Query(Query&&) = delete;
		Query& operator=(const Query&) = delete;
		Query& operator=(Query&&) = delete;
		~Query();

		[[nodiscard]] static Query& GetInstance();

		template<typename Type>
		void RegisterSystem(Type& system)
		{
			Assert(m_systems[(uint8)Type::SystemType] == nullptr);
			m_systems[(uint8)Type::SystemType] = &system;
		}
		template<typename Type>
		void DeregisterSystem()
		{
			Assert(m_systems[(uint8)Type::SystemType] != nullptr);
			m_systems[(uint8)Type::SystemType] = nullptr;
		}

		template<typename Type>
		[[nodiscard]] FORCE_INLINE Type& QuerySystem()
		{
			static_assert(HasSystemType<Type>, "Every system that should be queryable needs to provide a static system type.");
			Assert(m_systems[(uint8)Type::SystemType] != nullptr);

			return *reinterpret_cast<Type*>(GetSystem(Type::SystemType));
		}

		template<typename Type>
		[[nodiscard]] FORCE_INLINE const Type& QuerySystem() const
		{
			static_assert(HasSystemType<Type>, "Every system that should be queryable needs to provide a static system type.");
			Assert(m_systems[(uint8)Type::SystemType] != nullptr);

			return *reinterpret_cast<const Type*>(GetSystem(Type::SystemType));
		}

		template<typename Type>
		[[nodiscard]] FORCE_INLINE Optional<Type*> FindPlugin()
		{
			static_assert(HasPluginGuid<Type>, "Every plugin has to provide a guid for identification.");

			return static_cast<Type*>(GetPluginByGuid(Type::Guid).Get());
		}

		template<typename Type>
		[[nodiscard]] FORCE_INLINE Optional<Type*> FindSystem()
		{
			return reinterpret_cast<Type*>(GetSystem(Type::SystemType));
		}
	protected:
		friend Engine;
		friend Application;
	private:
		[[nodiscard]] void* GetSystem(const System::Type system)
		{
			return m_systems[(uint8)system];
		}
		[[nodiscard]] Optional<Plugin*> GetPluginByGuid(Guid guid);

		Array<void*, (uint8)System::Type::Count> m_systems{Memory::InitializeAll, nullptr};
	};

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS static Type& Get()
	{
		return Query::GetInstance().QuerySystem<Type>();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS static Optional<Type*> Find()
	{
		return Query::GetInstance().FindSystem<Type>();
	}

	template<typename Type>
	[[nodiscard]] FORCE_INLINE PURE_STATICS static Optional<Type*> FindPlugin()
	{
		return Query::GetInstance().FindPlugin<Type>();
	}
}
