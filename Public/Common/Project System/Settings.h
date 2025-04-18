#pragma once

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Memory/Any.h>
#include <Common/Memory/Containers/UnorderedMap.h>

namespace ngine
{
	namespace Internal
	{
		struct SettingsEntry : public Any
		{
			using Any::Any;
			SettingsEntry(Any&& any)
				: Any(Forward<Any>(any))
			{
			}

			bool Serialize(const Serialization::Reader reader);
			bool Serialize(Serialization::Writer writer) const;
		};
	}

	extern template struct UnorderedMap<Guid, Internal::SettingsEntry, Guid::Hash>;

	struct Settings
	{
		bool Serialize(const Serialization::Reader reader);
		bool Serialize(Serialization::Writer writer) const;

		using Entry = Internal::SettingsEntry;

		void EmplaceEntry(const Guid guid, Any&& value)
		{
			m_map.Emplace(Guid(guid), Forward<Any>(value));
		}

		[[nodiscard]] AnyView FindEntry(const Guid guid)
		{
			decltype(m_map)::iterator it = m_map.Find(guid);
			if (it != m_map.end())
			{
				return it->second;
			}
			return {};
		}
		[[nodiscard]] ConstAnyView FindEntry(const Guid guid) const
		{
			decltype(m_map)::const_iterator it = m_map.Find(guid);
			if (it != m_map.end())
			{
				return it->second;
			}
			return {};
		}

		template<typename Type>
		[[nodiscard]] Optional<Type*> FindEntry(const Guid guid)
		{
			return FindEntry(guid).Get<Type>();
		}
		template<typename Type>
		[[nodiscard]] Optional<const Type*> FindEntry(const Guid guid) const
		{
			return FindEntry(guid).Get<Type>();
		}
	protected:
		UnorderedMap<Guid, Entry, Guid::Hash> m_map;
	};
}
