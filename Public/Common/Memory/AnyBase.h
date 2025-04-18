#pragma once

#include <Common/Memory/ForwardDeclarations/AnyBase.h>
#include <Common/Memory/AnyView.h>
#include <Common/Reflection/TypeDefinition.h>

namespace ngine
{
	template<typename AllocatorType_>
	struct TAny
	{
	private:
		using AllocatorType = AllocatorType_;

		template<typename Type>
		struct TypeHelper
		{
			inline static constexpr size MaxOffset = alignof(Type) > alignof(typename AllocatorType::ElementType)
			                                           ? (alignof(Type) - alignof(typename AllocatorType::ElementType))
			                                           : 0;
			inline static constexpr size RequiredSize = sizeof(Type) + MaxOffset;

			template<typename ElementType = Type, typename = EnableIf<TypeTraits::IsDefaultConstructible<ElementType>>>
			static void DefaultConstructIntoStorage(AllocatorType& storage)
			{
				if (storage.GetCapacity() < RequiredSize)
				{
					Assert(storage.GetTheoreticalCapacity() > RequiredSize, "Type does not fit into storage!");
					storage.Allocate(RequiredSize);
				}

				uintptr pData = reinterpret_cast<uintptr>(storage.GetData());
				if constexpr (MaxOffset > 0)
				{
					pData += pData % alignof(Type);
				}

				new (reinterpret_cast<Type*>(pData)) Type();
			}

			template<typename... Arguments>
			static void EmplaceConstructIntoStorage(AllocatorType& storage, Arguments&&... arguments)
			{
				if (storage.GetCapacity() < RequiredSize)
				{
					Assert(storage.GetTheoreticalCapacity() > RequiredSize, "Type does not fit into storage!");
					storage.Allocate(RequiredSize);
				}

				uintptr pData = reinterpret_cast<uintptr>(storage.GetData());
				if constexpr (MaxOffset > 0)
				{
					pData += pData % alignof(Type);
				}

				new (reinterpret_cast<Type*>(pData)) Type(Forward<Arguments>(arguments)...);
			}

			template<typename ElementType = Type, typename = EnableIf<TypeTraits::IsMoveConstructible<ElementType>>>
			static void EmplaceIntoStorage(AllocatorType& storage, Type&& value)
			{
				if (storage.GetCapacity() < RequiredSize)
				{
					Assert(storage.GetTheoreticalCapacity() > RequiredSize, "Type does not fit into storage!");
					storage.Allocate(RequiredSize);
				}

				uintptr pData = reinterpret_cast<uintptr>(storage.GetData());
				if constexpr (MaxOffset > 0)
				{
					pData += pData % alignof(Type);
				}

				new (reinterpret_cast<Type*>(pData)) Type(Forward<Type>(value));
			}

			template<typename ElementType = Type, typename = EnableIf<TypeTraits::IsCopyConstructible<ElementType>>>
			static void CopyIntoStorage(AllocatorType& storage, const Type& value)
			{
				if (storage.GetCapacity() < RequiredSize)
				{
					Assert(storage.GetTheoreticalCapacity() > RequiredSize, "Type does not fit into storage!");
					storage.Allocate(RequiredSize);
				}

				uintptr pData = reinterpret_cast<uintptr>(storage.GetData());
				if constexpr (MaxOffset > 0)
				{
					pData += pData % alignof(Type);
				}

				new (reinterpret_cast<Type*>(pData)) Type(value);
			}
		};
	public:
		TAny() = default;
		TAny(InvalidType)
			: TAny()
		{
		}
		TAny(const void* pData, const Reflection::TypeDefinition typeDefinition)
			: m_storage(Memory::Reserve, typeDefinition.GetSize())
			, m_typeDefinition(typeDefinition)
		{
			[[maybe_unused]] const uint32 result =
				m_typeDefinition.TryCopyConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity(), pData);
			Assert(result == 0);
		}
		template<typename Type, typename = EnableIf<TypeTraits::IsMoveConstructible<Type>>>
		TAny(Type&& value)
			: m_typeDefinition(Reflection::TypeDefinition::Get<Type>())
		{
			TypeHelper<Type>::EmplaceIntoStorage(m_storage, Forward<Type>(value));
		}
		template<typename Type, typename = EnableIf<TypeTraits::IsCopyConstructible<Type>>>
		TAny(const Type& value)
			: m_typeDefinition(Reflection::TypeDefinition::Get<Type>())
		{
			TypeHelper<Type>::CopyIntoStorage(m_storage, value);
		}
		template<typename Type, typename = EnableIf<TypeTraits::IsCopyConstructible<Type>>>
		TAny(Type& value)
			: m_typeDefinition(Reflection::TypeDefinition::Get<Type>())
		{
			TypeHelper<Type>::CopyIntoStorage(m_storage, value);
		}
		TAny(Memory::UninitializedType, const Reflection::TypeDefinition typeDefinition)
			: m_storage(Memory::Reserve, typeDefinition.GetSize())
			, m_typeDefinition(typeDefinition)
		{
		}
		TAny(Memory::DefaultConstructType, const Reflection::TypeDefinition typeDefinition)
			: m_storage(Memory::Reserve, typeDefinition.GetSize())
		{
			uint32 result = typeDefinition.TryDefaultConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity());
			switch (result)
			{
				case 0: // Success
					m_typeDefinition = typeDefinition;
					break;
				case Math::NumericLimits<uint32>::Max: // Copy not supported
					m_typeDefinition = Reflection::TypeDefinition();
					break;
				default: // Insufficient capacity
					ExpectUnreachable();
					break;
			}
		}
		TAny(TAny&& other)
		{
			if (other.m_storage.IsDynamicallyStored())
			{
				m_storage = Move(other.m_storage);
				m_typeDefinition = other.m_typeDefinition;
			}
			else
			{
				m_storage.Allocate(other.m_typeDefinition.GetSize());
				uint32 result =
					other.m_typeDefinition.TryMoveConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity(), other.m_storage.GetData());
				switch (result)
				{
					case 0: // Successfully moved
						m_typeDefinition = other.m_typeDefinition;
						break;
					case Math::NumericLimits<uint32>::Max: // Move not supported
						m_typeDefinition = Reflection::TypeDefinition();
						other.Destroy();
						break;
					default: // Insufficient capacity
						ExpectUnreachable() break;
				}
			}
			other.m_typeDefinition = Reflection::TypeDefinition();
		}
		TAny(const TAny& other)
			: m_storage(Memory::Reserve, other.m_typeDefinition.GetSize())
		{
			uint32 result =
				other.m_typeDefinition.TryCopyConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity(), other.m_storage.GetData());
			switch (result)
			{
				case 0: // Successfully copied
					m_typeDefinition = other.m_typeDefinition;
					break;
				case Math::NumericLimits<uint32>::Max: // Copy not supported
					m_typeDefinition = Reflection::TypeDefinition();
					break;
				default: // Insufficient capacity
					ExpectUnreachable();
					break;
			}
		}
		TAny(TAny& other)
			: TAny(const_cast<const TAny&>(other))
		{
		}
		TAny& operator=(TAny&& other)
		{
			Destroy();

			if (other.m_storage.IsDynamicallyStored())
			{
				m_storage = Move(other.m_storage);
				m_typeDefinition = other.m_typeDefinition;
			}
			else
			{
				uint32 result =
					other.m_typeDefinition.TryMoveConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity(), other.m_storage.GetData());
				switch (result)
				{
					case 0: // Successfully moved
						m_typeDefinition = other.m_typeDefinition;
						break;
					case Math::NumericLimits<uint32>::Max: // Move not supported
						m_typeDefinition = Reflection::TypeDefinition();
						other.Destroy();
						break;
					default: // Insufficient capacity
					{
						m_storage.Allocate(result);
						result =
							other.m_typeDefinition.TryMoveConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity(), other.m_storage.GetData());
						Assert(result == 0);
						m_typeDefinition = other.m_typeDefinition;
					}
					break;
				}
			}
			other.m_typeDefinition = Reflection::TypeDefinition();

			return *this;
		}
		TAny& operator=(const TAny& other)
		{
			Destroy();
			uint32 result =
				other.m_typeDefinition.TryCopyConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity(), other.m_storage.GetData());
			switch (result)
			{
				case 0: // Successfully copied
					m_typeDefinition = other.m_typeDefinition;
					break;
				case Math::NumericLimits<uint32>::Max: // Copy not supported
					m_typeDefinition = Reflection::TypeDefinition();
					break;
				default: // Insufficient capacity
				{
					m_storage.Allocate(result);
					result =
						other.m_typeDefinition.TryCopyConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity(), other.m_storage.GetData());
					Assert(result == 0);
					m_typeDefinition = other.m_typeDefinition;
				}
				break;
			}
			return *this;
		}
		TAny& operator=(TAny& other)
		{
			return operator=(const_cast<const TAny&>(other));
		}
		TAny(const ConstAnyView view)
			: m_storage(Memory::Reserve, view.GetTypeDefinition().GetSize())
		{
			uint32 result = view.GetTypeDefinition().TryCopyConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity(), view.GetData());
			switch (result)
			{
				case 0: // Successfully copied
					m_typeDefinition = view.GetTypeDefinition();
					break;
				case Math::NumericLimits<uint32>::Max: // Copy not supported
					m_typeDefinition = Reflection::TypeDefinition();
					break;
				default: // Insufficient capacity
					ExpectUnreachable();
					break;
			}
		}
		TAny(const AnyView view)
			: TAny((ConstAnyView)view)
		{
		}
		TAny& operator=(const ConstAnyView view)
		{
			Destroy();
			uint32 result = view.GetTypeDefinition().TryCopyConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity(), view.GetData());
			switch (result)
			{
				case 0: // Successfully copied
					m_typeDefinition = view.GetTypeDefinition();
					break;
				case Math::NumericLimits<uint32>::Max: // Copy not supported
					m_typeDefinition = Reflection::TypeDefinition();
					break;
				default: // Insufficient capacity
				{
					m_storage.Allocate(result);
					result = view.GetTypeDefinition().TryCopyConstructStoredObject(m_storage.GetData(), m_storage.GetCapacity(), view.GetData());
					Assert(result == 0);
					m_typeDefinition = view.GetTypeDefinition();
				}
				break;
			}
			return *this;
		}
		TAny& operator=(const AnyView view)
		{
			return operator=((ConstAnyView)view);
		}
		template<typename Type, typename = EnableIf<TypeTraits::IsMoveAssignable<Type>>>
		TAny& operator=(Type&& value)
		{
			Destroy();
			TypeHelper<Type>::EmplaceIntoStorage(m_storage, Forward<Type>(value));
			m_typeDefinition = Reflection::TypeDefinition::Get<Type>();
			return *this;
		}
		template<typename Type, typename = EnableIf<TypeTraits::IsCopyAssignable<Type>>>
		TAny& operator=(const Type& value)
		{
			Destroy();
			TypeHelper<Type>::CopyIntoStorage(m_storage, value);
			m_typeDefinition = Reflection::TypeDefinition::Get<Type>();
			return *this;
		}
		template<typename Type, typename = EnableIf<TypeTraits::IsCopyAssignable<Type>>>
		TAny& operator=(Type& value)
		{
			Destroy();
			TypeHelper<Type>::CopyIntoStorage(m_storage, value);
			m_typeDefinition = Reflection::TypeDefinition::Get<Type>();
			return *this;
		}
		~TAny()
		{
			Destroy();
		}

		template<typename Type, typename... Args>
		[[nodiscard]] static TAny Make(const Args&... args)
		{
			TAny any;
			TypeHelper<Type>::EmplaceConstructIntoStorage(any.m_storage, args...);
			any.m_typeDefinition = Reflection::TypeDefinition::Get<Type>();
			return any;
		}
		template<typename Type, typename... Args>
		[[nodiscard]] static TAny Make(Args&&... args)
		{
			TAny any;
			TypeHelper<Type>::EmplaceConstructIntoStorage(any.m_storage, Forward<Args>(args)...);
			any.m_typeDefinition = Reflection::TypeDefinition::Get<Type>();
			return any;
		}

		template<typename Type, typename = EnableIf<TypeTraits::IsDefaultConstructible<Type>>>
		[[nodiscard]] static TAny Make()
		{
			TAny any;
			TypeHelper<Type>::DefaultConstructIntoStorage(any.m_storage);
			any.m_typeDefinition = Reflection::TypeDefinition::Get<Type>();
			return any;
		}

		[[nodiscard]] PURE_STATICS void* GetData() LIFETIME_BOUND
		{
			return m_typeDefinition.GetAlignedObject(m_storage.GetData());
		}

		[[nodiscard]] PURE_STATICS const void* GetData() const LIFETIME_BOUND
		{
			return m_typeDefinition.GetAlignedObject(const_cast<typename AllocatorType::ElementType*>(m_storage.GetData()));
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS Optional<Type*> Get() LIFETIME_BOUND
		{
			if (Is<Type>())
			{
				return static_cast<Type*>(GetData());
			}
			return Invalid;
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS Optional<const Type*> Get() const LIFETIME_BOUND
		{
			if (Is<Type>())
			{
				return static_cast<const Type*>(GetData());
			}
			return Invalid;
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS Type& GetExpected() LIFETIME_BOUND
		{
			Assert(Is<Type>());
			uintptr address = reinterpret_cast<uintptr>(m_storage.GetData());
			address += address % alignof(Type);
			return *reinterpret_cast<Type*>(address);
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS const Type& GetExpected() const LIFETIME_BOUND
		{
			Assert(Is<Type>());
			uintptr address = reinterpret_cast<uintptr>(m_storage.GetData());
			address += address % alignof(Type);
			return *reinterpret_cast<const Type*>(address);
		}

		template<typename Type>
		[[nodiscard]] Type StealExpected() LIFETIME_BOUND
		{
			Assert(Is<Type>());
			uintptr address = reinterpret_cast<uintptr>(m_storage.GetData());
			address += address % alignof(Type);
			return Move(*reinterpret_cast<Type*>(address));
		}

		[[nodiscard]] PURE_STATICS ByteView GetByteView() LIFETIME_BOUND
		{
			return m_typeDefinition.GetByteViewAtAddress(m_storage.GetData());
		}

		[[nodiscard]] PURE_STATICS ConstByteView GetByteView() const LIFETIME_BOUND
		{
			return m_typeDefinition.GetByteViewAtAddress(const_cast<ByteType*>(m_storage.GetData()));
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS constexpr bool Is() const
		{
			return m_typeDefinition.Is<Type>();
		}

		[[nodiscard]] PURE_STATICS bool operator==(const TAny& other) const
		{
			return m_typeDefinition == other.m_typeDefinition &&
			       m_typeDefinition.AreStoredObjectsEqual(m_storage.GetData(), other.m_storage.GetData());
		}

		[[nodiscard]] PURE_STATICS bool operator!=(const TAny& other) const
		{
			return !operator==(other);
		}

		[[nodiscard]] PURE_STATICS bool IsValid() const
		{
			return m_typeDefinition.IsValid();
		}

		[[nodiscard]] PURE_STATICS explicit operator bool() const
		{
			return IsValid();
		}

		[[nodiscard]] PURE_STATICS bool IsInvalid() const
		{
			return m_typeDefinition.IsInvalid();
		}

		[[nodiscard]] PURE_STATICS Reflection::TypeDefinition GetTypeDefinition() const
		{
			return m_typeDefinition;
		}

		operator AnyView() const = delete;

		[[nodiscard]] PURE_STATICS operator AnyView() LIFETIME_BOUND
		{
			return AnyView(reinterpret_cast<void*>(m_typeDefinition.GetAlignedObject(m_storage.GetData())), m_typeDefinition);
		}

		[[nodiscard]] PURE_STATICS operator ConstAnyView() LIFETIME_BOUND
		{
			return ConstAnyView(reinterpret_cast<void*>(m_typeDefinition.GetAlignedObject(m_storage.GetData())), m_typeDefinition);
		}

		[[nodiscard]] PURE_STATICS operator ConstAnyView() const LIFETIME_BOUND
		{
			return ConstAnyView(
				reinterpret_cast<void*>(m_typeDefinition.GetAlignedObject(const_cast<typename AllocatorType::ElementType*>(m_storage.GetData()))),
				m_typeDefinition
			);
		}

		void MoveIntoView(AnyView view)
		{
			Assert(view.GetTypeDefinition() == m_typeDefinition);
			m_typeDefinition.MoveConstructAt(view.GetData(), GetData());
			m_typeDefinition = Reflection::TypeDefinition();
		}

		void CopyIntoView(AnyView view)
		{
			Assert(view.GetTypeDefinition() == m_typeDefinition);
			m_typeDefinition.CopyConstructAt(view.GetData(), GetData());
		}

		bool Serialize(const Serialization::Reader);
		bool Serialize(Serialization::Writer) const;
	private:
		void Destroy()
		{
			if (m_typeDefinition.IsValid())
			{
				m_typeDefinition.DestroyUnalignedObject(m_storage.GetData());
				m_typeDefinition = Reflection::TypeDefinition();
			}
			else
			{
				Assert(!m_storage.HasAnyCapacity());
			}
		}
	private:
		AllocatorType m_storage;
		Reflection::TypeDefinition m_typeDefinition;
	};
}
