#pragma once

#include <Common/Memory/ForwardDeclarations/AnyView.h>
#include <Common/Memory/Optional.h>
#include <Common/Reflection/TypeDefinition.h>
#include <Common/Memory/Invalid.h>
#include <Common/Memory/AddressOf.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	struct TRIVIAL_ABI AnyView
	{
		AnyView() = default;
		AnyView(InvalidType)
			: AnyView()
		{
		}
		AnyView(void* pData, const Reflection::TypeDefinition typeDefinition)
			: m_pData(pData)
			, m_typeDefinition(typeDefinition)
		{
		}
		template<typename Type, typename = EnableIf<!TypeTraits::IsBaseOf<AnyView, Type>>>
		AnyView(const Type& value LIFETIME_BOUND)
			: m_pData(const_cast<void*>(static_cast<const void*>(Memory::GetAddressOf(value))))
			, m_typeDefinition(Reflection::TypeDefinition::Get<Type>())
		{
		}
		template<typename Type, typename = EnableIf<!TypeTraits::IsBaseOf<AnyView, Type>>>
		AnyView(Type& value LIFETIME_BOUND)
			: m_pData(Memory::GetAddressOf(value))
			, m_typeDefinition(Reflection::TypeDefinition::Get<Type>())
		{
		}
		template<typename Type>
		AnyView(Type&&) = delete;
		AnyView(AnyView&& other) = default;
		AnyView(const AnyView& other LIFETIME_BOUND)
			: m_pData(other.m_pData)
			, m_typeDefinition(other.m_typeDefinition)
		{
		}
		AnyView(AnyView& other LIFETIME_BOUND)
			: AnyView(const_cast<const AnyView&>(other))
		{
		}
		AnyView& operator=(AnyView&& other) = default;
		AnyView& operator=(const AnyView& other LIFETIME_BOUND)
		{
			m_pData = other.m_pData;
			m_typeDefinition = other.m_typeDefinition;
			return *this;
		}
		AnyView& operator=(AnyView& other LIFETIME_BOUND)
		{
			return operator=(const_cast<const AnyView&>(other));
		}
		template<typename Type, typename = EnableIf<!TypeTraits::IsBaseOf<AnyView, Type>>>
		AnyView& operator=(const Type& value LIFETIME_BOUND)
		{
			m_pData = const_cast<void*>(static_cast<const void*>(Memory::GetAddressOf(value)));
			m_typeDefinition = Reflection::TypeDefinition::Get<Type>();
			return *this;
		}
		template<typename Type, typename = EnableIf<!TypeTraits::IsBaseOf<AnyView, Type>>>
		AnyView& operator=(Type& value LIFETIME_BOUND)
		{
			m_pData = Memory::GetAddressOf(value);
			m_typeDefinition = Reflection::TypeDefinition::Get<Type>();
			return *this;
		}
		template<typename Type>
		AnyView& operator=(Type&& other) = delete;

		template<typename Type>
		[[nodiscard]] PURE_STATICS Optional<Type*> Get()
		{
			if (Is<Type>())
			{
				// TODO: If Type is const Type, and stored type is non-cosnt, allow
				return static_cast<Type*>(m_pData);
			}
			return Invalid;
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS Optional<const Type*> Get() const
		{
			if (Is<Type>())
			{
				return static_cast<const Type*>(m_pData);
			}
			return Invalid;
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS Type& GetExpected()
		{
			Assert(Is<Type>());
			return *reinterpret_cast<Type*>(m_pData);
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS const Type& GetExpected() const
		{
			Assert(Is<Type>());
			return *reinterpret_cast<const Type*>(m_pData);
		}

		[[nodiscard]] PURE_STATICS void* GetData()
		{
			return m_pData;
		}

		[[nodiscard]] PURE_STATICS const void* GetData() const
		{
			return m_pData;
		}

		[[nodiscard]] PURE_STATICS ByteView GetByteView()
		{
			return m_typeDefinition.GetByteViewAtAddress(m_pData);
		}

		[[nodiscard]] PURE_STATICS ConstByteView GetByteView() const
		{
			return m_typeDefinition.GetByteViewAtAddress(m_pData);
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS constexpr bool Is() const
		{
			return m_typeDefinition == Reflection::TypeDefinition::Get<Type>();
		}

		[[nodiscard]] PURE_STATICS Reflection::TypeDefinition GetTypeDefinition() const
		{
			return m_typeDefinition;
		}

		[[nodiscard]] PURE_STATICS bool operator==(const AnyView& other) const
		{
			return m_typeDefinition == other.GetTypeDefinition() &&
			       (GetData() == other.GetData() || m_typeDefinition.AreReferencesEqual(m_pData, other.m_pData));
		}

		[[nodiscard]] PURE_STATICS bool operator!=(const AnyView& other) const
		{
			return !operator==(other);
		}

		[[nodiscard]] PURE_STATICS bool IsValid() const
		{
			return m_typeDefinition.IsValid();
		}

		[[nodiscard]] PURE_STATICS bool IsInvalid() const
		{
			return m_typeDefinition.IsInvalid();
		}

		[[nodiscard]] PURE_STATICS explicit operator bool() const
		{
			return IsValid();
		}
	private:
		void* m_pData = nullptr;
		Reflection::TypeDefinition m_typeDefinition;
	};

	struct ConstAnyView : protected AnyView
	{
		ConstAnyView(const AnyView view)
			: AnyView(view)
		{
		}
		ConstAnyView(const void* pData, const Reflection::TypeDefinition typeDefinition)
			: AnyView(const_cast<void*>(pData), typeDefinition)
		{
		}
		ConstAnyView(const ConstAnyView& other)
			: AnyView(static_cast<const AnyView&>(other))
		{
		}
		ConstAnyView& operator=(const ConstAnyView& other)
		{
			AnyView::operator=(static_cast<const AnyView&>(other));
			return *this;
		}
		ConstAnyView(ConstAnyView& other)
			: AnyView(static_cast<AnyView&>(other))
		{
		}
		ConstAnyView& operator=(ConstAnyView& other)
		{
			AnyView::operator=(static_cast<AnyView&>(other));
			return *this;
		}
		ConstAnyView(ConstAnyView&& other)
			: AnyView(Forward<AnyView>(other))
		{
		}
		ConstAnyView& operator=(ConstAnyView&& other)
		{
			AnyView::operator=(Forward<AnyView>(other));
			return *this;
		}
		using AnyView::AnyView;
		using AnyView::operator=;

		template<typename Type>
		[[nodiscard]] PURE_STATICS Optional<const Type*> Get() const
		{
			return AnyView::Get<Type>();
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS const Type& GetExpected() const
		{
			return AnyView::GetExpected<Type>();
		}

		[[nodiscard]] PURE_STATICS ConstByteView GetByteView() const
		{
			return AnyView::GetByteView();
		}

		using AnyView::Is;
		using AnyView::GetTypeDefinition;

		using AnyView::operator==;
		using AnyView::operator!=;

		[[nodiscard]] PURE_STATICS bool operator==(const ConstAnyView& other) const
		{
			return AnyView::operator==(other);
		}
		[[nodiscard]] PURE_STATICS bool operator!=(const ConstAnyView& other) const
		{
			return !operator==(other);
		}

		using AnyView::IsValid;
		using AnyView::IsInvalid;
		using AnyView::operator bool;

		[[nodiscard]] PURE_STATICS const void* GetData() const
		{
			return AnyView::GetData();
		}
	};
}
