#pragma once

#include "Identifier.h"
#include "IdentifierMask.h"
#include <Common/Memory/AtomicBitset.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Threading
{
	template<typename IdentifierType>
	struct TRIVIAL_ABI AtomicIdentifierMask : public Threading::AtomicBitset<IdentifierType::MaximumCount>
	{
		using BaseType = Threading::AtomicBitset<IdentifierType::MaximumCount>;
		using BaseType::BaseType;
		using BaseType::operator=;

		FORCE_INLINE AtomicIdentifierMask(const IdentifierType identifier)
			: BaseType()
		{
			Set(identifier);
		}

		AtomicIdentifierMask(const BaseType& other)
			: BaseType(other)
		{
		}
		AtomicIdentifierMask& operator=(const BaseType& other)
		{
			BaseType::operator=(other);
			return *this;
		}
		AtomicIdentifierMask(BaseType&& other)
			: BaseType(Forward<BaseType>(other))
		{
		}
		AtomicIdentifierMask& operator=(BaseType&& other)
		{
			BaseType::operator=(Forward<BaseType>(other));
			return *this;
		}

		FORCE_INLINE constexpr bool Set(const IdentifierType identifier)
		{
			Assert(identifier.GetIndex() != 0);
			return BaseType::Set(identifier.GetFirstValidIndex());
		}
		bool Set(const typename BaseType::BitIndexType position) = delete;
		using BaseType::Set;

		FORCE_INLINE constexpr bool Clear(const IdentifierType identifier)
		{
			Assert(identifier.GetIndex() != 0);
			return BaseType::Clear(identifier.GetFirstValidIndex());
		}
		bool Clear(const typename BaseType::BitIndexType position) = delete;
		using BaseType::Clear;
		using BaseType::ClearAll;

		[[nodiscard]] FORCE_INLINE constexpr bool IsSet(const IdentifierType identifier) const
		{
			Assert(identifier.GetIndex() != 0);
			return BaseType::IsSet(identifier.GetFirstValidIndex());
		}
		[[nodiscard]] FORCE_INLINE constexpr bool IsNotSet(const IdentifierType identifier) const
		{
			Assert(identifier.GetIndex() != 0);
			return BaseType::IsNotSet(identifier.GetFirstValidIndex());
		}
		bool IsSet(const typename BaseType::BitIndexType position) const = delete;
		bool IsNotSet(const typename BaseType::BitIndexType position) const = delete;

		[[nodiscard]] FORCE_INLINE operator IdentifierMask<IdentifierType>() const
		{
			static_assert(sizeof(IdentifierMask<IdentifierType>) == sizeof(AtomicIdentifierMask));
			static_assert(alignof(IdentifierMask<IdentifierType>) == alignof(AtomicIdentifierMask));
			return reinterpret_cast<const IdentifierMask<IdentifierType>&>(*this);
		}
	};
}
