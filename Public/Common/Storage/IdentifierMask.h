#pragma once

#include "Identifier.h"
#include <Common/Memory/Bitset.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	template<typename IdentifierType>
	struct TRIVIAL_ABI IdentifierMask : public Bitset<IdentifierType::MaximumCount>
	{
		using BaseType = Bitset<IdentifierType::MaximumCount>;
		using BaseType::BaseType;
		using BaseType::operator=;

		FORCE_INLINE IdentifierMask(const IdentifierType identifier)
			: BaseType()
		{
			Set(identifier);
		}

		IdentifierMask(const BaseType& other)
			: BaseType(other)
		{
		}
		IdentifierMask& operator=(const BaseType& other)
		{
			BaseType::operator=(other);
			return *this;
		}
		IdentifierMask(BaseType&& other)
			: BaseType(Forward<BaseType>(other))
		{
		}
		IdentifierMask& operator=(BaseType&& other)
		{
			BaseType::operator=(Forward<BaseType>(other));
			return *this;
		}
		IdentifierMask(const IdentifierMask&) = default;
		IdentifierMask& operator=(const IdentifierMask&) = default;
		IdentifierMask(IdentifierMask&&) = default;
		IdentifierMask& operator=(IdentifierMask&&) = default;

		FORCE_INLINE constexpr void Set(const IdentifierType identifier)
		{
			Assert(identifier.GetIndex() != 0);
			BaseType::Set(identifier.GetFirstValidIndex());
		}
		void Set(const typename BaseType::BitIndexType position) = delete;
		using BaseType::Set;

		FORCE_INLINE constexpr void Toggle(const IdentifierType identifier)
		{
			Assert(identifier.GetIndex() != 0);
			BaseType::Toggle(identifier.GetFirstValidIndex());
		}
		void Toggle(const typename BaseType::BitIndexType position) = delete;
		using BaseType::Toggle;

		FORCE_INLINE constexpr void Clear(const IdentifierType identifier)
		{
			Assert(identifier.GetIndex() != 0);
			BaseType::Clear(identifier.GetFirstValidIndex());
		}
		void Clear(const typename BaseType::BitIndexType position) = delete;
		using BaseType::Clear;

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsSet(const IdentifierType identifier) const
		{
			return BaseType::IsSet(identifier.GetFirstValidIndex());
		}
		bool IsSet(const typename BaseType::BitIndexType position) const = delete;

		// TODO: Override GetSetBitsIterator and return IdentifierType instead
	};
}
