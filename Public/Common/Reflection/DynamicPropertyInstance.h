#pragma once

#include <Common/Function/Function.h>
#include <Common/Memory/Any.h>
#include <Common/Memory/ReferenceWrapper.h>

namespace ngine::Reflection
{
	struct PropertyOwner;

	struct DynamicPropertyInstance
	{
		using Getter = ngine::Function<Any(PropertyOwner& owner, const Optional<PropertyOwner*> pParent), 8>;
		using Setter = ngine::Function<void(PropertyOwner& owner, const Optional<PropertyOwner*> pParent, Any&& newValue), 8>;

		DynamicPropertyInstance()
			: m_getter(
					[](PropertyOwner&, [[maybe_unused]] const Optional<PropertyOwner*> pParent)
					{
						return Any();
					}
				)
			, m_setter(
					[](PropertyOwner&, [[maybe_unused]] const Optional<PropertyOwner*> pParent, Any&&)
					{
					}
				)
		{
		}

		DynamicPropertyInstance(Getter&& getter, Setter&& setter)
			: m_getter(Forward<Getter>(getter))
			, m_setter(Forward<Setter>(setter))
		{
		}

		Any GetValue(const PropertyOwner& owner, const Optional<const PropertyOwner*> pParent) const
		{
			return m_getter(const_cast<PropertyOwner&>(owner), const_cast<PropertyOwner*>(pParent.Get()));
		}

		[[nodiscard]] Any GetSharedValue(
			const ArrayView<const ReferenceWrapper<PropertyOwner>> owners, const ArrayView<const Optional<PropertyOwner*>> pParents
		) const
		{
			Assert(owners.HasElements());
			Any value = GetValue(owners[0], pParents[0]);
			for (uint32 index = 1, count = owners.GetSize(); index < count; ++index)
			{
				if (value != GetValue(owners[index], pParents[index]))
				{
					return Invalid;
				}
			}
			return value;
		}

		void SetValue(PropertyOwner& owner, const Optional<PropertyOwner*> pParent, Any&& newValue) const
		{
			m_setter(owner, pParent, Forward<Any>(newValue));
		}

		void SetValue(
			const ArrayView<const ReferenceWrapper<PropertyOwner>> owners,
			const ArrayView<const Optional<PropertyOwner*>> pParents,
			Any&& newValue
		) const
		{
			for (uint32 index = 0, count = owners.GetSize(); index < count; ++index)
			{
				SetValue(owners[index], pParents[index], Any(newValue));
			}
		}

		[[nodiscard]] bool IsValid() const
		{
			return m_getter.IsValid() & m_setter.IsValid();
		}
	private:
		Getter m_getter;
		Setter m_setter;
	};
}
