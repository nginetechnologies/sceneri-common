#pragma once

#include <Common/Platform/StaticUnreachable.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Memory/GetNumericSize.h>
#include <Common/TypeTraits/IsTriviallyDestructible.h>
#include <Common/TypeTraits/WithoutReference.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/TypeTraits/All.h>
#include <Common/TypeTraits/Any.h>
#include <Common/TypeTraits/IsEqualityComparable.h>
#include <Common/TypeTraits/IsBaseOf.h>
#include <Common/Memory/Optional.h>
#include <Common/Memory/AnyView.h>
#include <Common/Memory/AddressOf.h>

namespace ngine
{
	namespace Internal
	{
		template<typename Callback>
		auto Invoke(Callback&& callback)
		{
			return callback();
		}

		template<bool IsTriviallyConstructible, typename... Elements>
		struct VariantStorage;

		template<bool IsTriviallyConstructible>
		struct TRIVIAL_ABI VariantStorage<IsTriviallyConstructible>
		{
			inline static constexpr bool HasValue = false;
			inline static constexpr bool HasNext = false;
			inline static constexpr bool IsTriviallyDestructible = true;

			template<size Index>
			struct GetElementType
			{
				using Type = void;
			};
		};

		template<bool IsTriviallyConstructible, typename ElementType_, typename... ArgumentTypes>
		struct TRIVIAL_ABI VariantStorage<IsTriviallyConstructible, ElementType_, ArgumentTypes...>
		{
			using ElementType = ElementType_;
			using NextElements = VariantStorage<TypeTraits::All<TypeTraits::IsTriviallyDestructible<ArgumentTypes>...>, ArgumentTypes...>;

			inline static constexpr bool HasValue = true;
			inline static constexpr bool HasNext = sizeof...(ArgumentTypes) > 0;
			inline static constexpr bool IsTriviallyDestructible = TypeTraits::All < TypeTraits::IsTriviallyDestructible<ElementType> &&
			                                                       TypeTraits::IsTriviallyDestructible<ArgumentTypes>... > ;

			constexpr VariantStorage()
			{
			}

			constexpr VariantStorage(VariantStorage&& other, const size activeIndex)
			{
				MoveFrom(Forward<VariantStorage>(other), activeIndex);
			}

			constexpr VariantStorage(const VariantStorage& other, const size activeIndex)
			{
				CopyFrom(other, activeIndex);
			}

			constexpr VariantStorage(ElementType&& value)
				: m_value(value)
			{
			}

			template<typename Type, bool HasNextElements = HasNext, typename = EnableIf<HasNextElements>>
			constexpr VariantStorage(Type&& value)
				: m_nextElements(Forward<Type>(value))
			{
				static_assert(HasNext);
				static_assert(!TypeTraits::IsSame<Type, ElementType>);
			}

			constexpr VariantStorage& operator=(ElementType&& value)
			{
				m_value = Forward<ElementType>(value);
				return *this;
			}

			template<typename Type>
			constexpr VariantStorage& operator=(Type&& value)
			{
				static_assert(!TypeTraits::IsSame<Type, ElementType>);
				static_assert(HasNext);
				m_nextElements = Forward<Type>(value);
				return *this;
			}

			constexpr VariantStorage(const ElementType& value)
				: m_value(value)
			{
			}

			template<typename Type>
			constexpr VariantStorage(const Type& value)
				: m_nextElements(value)
			{
				static_assert(HasNext);
				static_assert(!TypeTraits::IsSame<Type, ElementType>);
			}

			constexpr VariantStorage& operator=(const ElementType& value)
			{
				m_value = value;
				return *this;
			}

			template<typename Type>
			constexpr VariantStorage& operator=(const Type& value)
			{
				static_assert(!TypeTraits::IsSame<Type, ElementType>);
				static_assert(HasNext);
				m_nextElements = value;
				return *this;
			}

			~VariantStorage() = default;

			[[nodiscard]] constexpr PURE_STATICS Reflection::TypeDefinition GetTypeDefinition(const size index) const
			{
				switch (index)
				{
					case 0:
						return Reflection::TypeDefinition::Get<ElementType>();
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.GetTypeDefinition(index - 1);
						}
						else
						{
							return Reflection::TypeDefinition{};
						}
					}
				}
			}

			[[nodiscard]] constexpr PURE_STATICS bool Equals(const VariantStorage& other, const size index) const
			{
				switch (index)
				{
					case 0:
					{
						if constexpr (TypeTraits::IsEqualityComparable<const ElementType, const ElementType>)
						{
							return m_value == other.m_value;
						}
						else
						{
							return false;
						}
					}
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.Equals(other.m_nextElements, index - 1);
						}
						else
						{
							return false;
						}
					}
				}
			}

			PUSH_CLANG_WARNINGS
			DISABLE_CLANG_WARNING("-Wreturn-type")
			template<typename Callback>
			constexpr auto Visit(Callback&& callback, const size index)
			{
				switch (index)
				{
					case 0:
						return callback(m_value);
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.Visit(Forward<Callback>(callback), index - 1);
						}
					}
				}
			}
			template<typename Callback>
			constexpr auto Visit(Callback&& callback, const size index) const
			{
				switch (index)
				{
					case 0:
						return callback(m_value);
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.Visit(Forward<Callback>(callback), index - 1);
						}
					}
				}
			}
			template<typename Callback, typename... Callbacks>
			constexpr auto Visit(const size index, Callback&& callback, Callbacks&&... callbacks)
			{
				switch (index)
				{
					case 0:
						return callback(m_value);
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.Visit(index - 1, Forward<Callbacks>(callbacks)...);
						}
						else
						{
							return Internal::Invoke(Forward<Callbacks>(callbacks)...);
						}
					}
				}
			}
			template<typename Callback, typename... Callbacks>
			constexpr auto Visit(const size index, Callback&& callback, Callbacks&&... callbacks) const
			{
				switch (index)
				{
					case 0:
						return callback(m_value);
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.Visit(index - 1, Forward<Callbacks>(callbacks)...);
						}
						else
						{
							return Internal::Invoke(Forward<Callbacks>(callbacks)...);
						}
					}
				}
			}
			POP_CLANG_WARNINGS

			template<size Index>
			struct GetElementType
			{
				using Type = TypeTraits::Select<Index == 0, ElementType, typename NextElements::template GetElementType<Index - 1>::Type>;
			};

			template<size Index>
			using Type = typename GetElementType<Index>::Type;

			template<size Index>
			[[nodiscard]] PURE_STATICS constexpr Type<Index>& GetAt()
			{
				if constexpr (Index == 0)
				{
					return m_value;
				}
				else if constexpr (HasNext)
				{
					return m_nextElements.template GetAt<Index - 1>();
				}
				else
				{
					static_unreachable("Invalid index!");
				}
			}

			template<size Index>
			[[nodiscard]] PURE_STATICS constexpr const Type<Index>& GetAt() const
			{
				if constexpr (Index == 0)
				{
					return m_value;
				}
				else if constexpr (HasNext)
				{
					static_assert(Index > 0);
					return m_nextElements.template GetAt<Index - 1>();
				}
				else
				{
					static_unreachable("Invalid index!");
				}
			}

			[[nodiscard]] PURE_STATICS constexpr AnyView GetAt(const size index) LIFETIME_BOUND
			{
				switch (index)
				{
					case 0:
						return m_value;
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.GetAt(index - 1);
						}
						else
						{
							return {};
						}
					}
				}
			}

			[[nodiscard]] PURE_STATICS constexpr ConstAnyView GetAt(const size index) const LIFETIME_BOUND
			{
				switch (index)
				{
					case 0:
						return m_value;
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.GetAt(index - 1);
						}
						else
						{
							return {};
						}
					}
				}
			}

			template<typename Type>
			[[nodiscard]] PURE_STATICS static constexpr size GetFirstIndex()
			{
				if constexpr (TypeTraits::IsSame<ElementType, Type>)
				{
					return 0;
				}
				else if constexpr (HasNext)
				{
					return NextElements::template GetFirstIndex<Type>() + 1;
				}
				else
				{
					static_unreachable("Type was not present in variant!");
					return 0;
				}
			}

			template<typename Type>
			[[nodiscard]] PURE_STATICS static constexpr bool ContainsType()
			{
				if constexpr (TypeTraits::IsSame<ElementType, Type>)
				{
					return true;
				}
				else if constexpr (HasNext)
				{
					return NextElements::template ContainsType<Type>();
				}
				else
				{
					return false;
				}
			}

			constexpr void MoveFrom(VariantStorage&& other, const size activeIndex)
			{
				switch (activeIndex)
				{
					case 0:
						new (Memory::GetAddressOf(m_value)) ElementType(Move(other.m_value));
						break;
					default:
					{
						if constexpr (HasNext)
						{
							m_nextElements.MoveFrom(Move(other.m_nextElements), activeIndex - 1);
						}
					}
				}
			}

			constexpr void CopyFrom(const VariantStorage& other, const size activeIndex)
			{
				switch (activeIndex)
				{
					case 0:
						new (Memory::GetAddressOf(m_value)) ElementType(other.m_value);
						break;
					default:
					{
						if constexpr (HasNext)
						{
							m_nextElements.CopyFrom(other.m_nextElements, activeIndex - 1);
						}
					}
				}
			}

			template<typename Type>
			inline static constexpr size FirstIndex = GetFirstIndex<Type>();

			union
			{
				ElementType m_value;
				NextElements m_nextElements;
			};
		};

		template<typename ElementType_, typename... ArgumentTypes>
		struct TRIVIAL_ABI VariantStorage<false, ElementType_, ArgumentTypes...>
		{
			using ElementType = ElementType_;
			using NextElements = VariantStorage<TypeTraits::All<TypeTraits::IsTriviallyDestructible<ArgumentTypes>...>, ArgumentTypes...>;

			inline static constexpr bool HasValue = true;
			inline static constexpr bool HasNext = sizeof...(ArgumentTypes) > 0;
			inline static constexpr bool IsTriviallyDestructible = TypeTraits::All < TypeTraits::IsTriviallyDestructible<ElementType> &&
			                                                       TypeTraits::IsTriviallyDestructible<ArgumentTypes>... > ;

			constexpr VariantStorage()
			{
			}

			constexpr VariantStorage(VariantStorage&& other, const size activeIndex)
			{
				MoveFrom(Forward<VariantStorage>(other), activeIndex);
			}

			constexpr VariantStorage(const VariantStorage& other, const size activeIndex)
			{
				CopyFrom(other, activeIndex);
			}

			constexpr VariantStorage(ElementType&& value)
				: m_value(Forward<ElementType>(value))
			{
			}

			template<typename Type, bool HasNextElements = HasNext, typename = EnableIf<HasNextElements>>
			constexpr VariantStorage(Type&& value)
				: m_nextElements(Forward<Type>(value))
			{
				static_assert(HasNext);
				static_assert(!TypeTraits::IsSame<Type, ElementType>);
			}

			constexpr VariantStorage& operator=(ElementType&& value)
			{
				m_value = Forward<ElementType>(value);
				return *this;
			}

			template<typename Type>
			constexpr VariantStorage& operator=(Type&& value)
			{
				static_assert(!TypeTraits::IsSame<Type, ElementType>);
				static_assert(HasNext);
				m_nextElements = Forward<Type>(value);
				return *this;
			}

			constexpr VariantStorage(const ElementType& value)
				: m_value(value)
			{
			}

			template<typename Type>
			constexpr VariantStorage(const Type& value)
				: m_nextElements(value)
			{
				static_assert(HasNext);
				static_assert(!TypeTraits::IsSame<Type, ElementType>);
			}

			constexpr VariantStorage& operator=(const ElementType& value)
			{
				m_value = value;
				return *this;
			}

			template<typename Type>
			constexpr VariantStorage& operator=(const Type& value)
			{
				static_assert(!TypeTraits::IsSame<Type, ElementType>);
				static_assert(HasNext);
				m_nextElements = value;
				return *this;
			}

			~VariantStorage()
			{
			}

			[[nodiscard]] PURE_STATICS constexpr Reflection::TypeDefinition GetTypeDefinition(const size index) const
			{
				switch (index)
				{
					case 0:
						return Reflection::TypeDefinition::Get<ElementType>();
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.GetTypeDefinition(index - 1);
						}
						else
						{
							return Reflection::TypeDefinition{};
						}
					}
				}
			}

			[[nodiscard]] PURE_STATICS constexpr bool Equals(const VariantStorage& other, const size index) const
			{
				switch (index)
				{
					case 0:
					{
						if constexpr (TypeTraits::IsEqualityComparable<const ElementType, const ElementType>)
						{
							return bool{m_value == other.m_value};
						}
						else
						{
							return false;
						}
					}
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.Equals(other.m_nextElements, index - 1);
						}
						else
						{
							return false;
						}
					}
				}
			}

			PUSH_CLANG_WARNINGS
			DISABLE_CLANG_WARNING("-Wreturn-type")
			template<typename Callback>
			constexpr auto Visit(Callback&& callback, const size index)
			{
				switch (index)
				{
					case 0:
						return callback(m_value);
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.Visit(Forward<Callback>(callback), index - 1);
						}
					}
				}
			}
			template<typename Callback>
			constexpr auto Visit(Callback&& callback, const size index) const
			{
				switch (index)
				{
					case 0:
						return callback(m_value);
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.Visit(Forward<Callback>(callback), index - 1);
						}
					}
				}
			}
			template<typename Callback, typename... Callbacks>
			constexpr auto Visit(const size index, Callback&& callback, Callbacks&&... callbacks)
			{
				switch (index)
				{
					case 0:
						return callback(m_value);
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.Visit(index - 1, Forward<Callbacks>(callbacks)...);
						}
						else
						{
							return Internal::Invoke(Forward<Callbacks>(callbacks)...);
						}
					}
				}
			}
			template<typename Callback, typename... Callbacks>
			constexpr auto Visit(const size index, Callback&& callback, Callbacks&&... callbacks) const
			{
				switch (index)
				{
					case 0:
						return callback(m_value);
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.Visit(index - 1, Forward<Callbacks>(callbacks)...);
						}
						else
						{
							return Internal::Invoke(Forward<Callbacks>(callbacks)...);
						}
					}
				}
			}
			POP_CLANG_WARNINGS

			template<size Index>
			struct GetElementType
			{
				using Type = TypeTraits::Select<Index == 0, ElementType, typename NextElements::template GetElementType<Index - 1>::Type>;
			};

			template<size Index>
			using Type = typename GetElementType<Index>::Type;

			template<size Index>
			[[nodiscard]] PURE_STATICS constexpr Type<Index>& GetAt()
			{
				if constexpr (Index == 0)
				{
					return m_value;
				}
				else if constexpr (HasNext)
				{
					return m_nextElements.template GetAt<Index - 1>();
				}
				else
				{
					static_unreachable("Invalid index!");
				}
			}

			template<size Index>
			[[nodiscard]] PURE_STATICS constexpr const Type<Index>& GetAt() const
			{
				if constexpr (Index == 0)
				{
					return m_value;
				}
				else if constexpr (HasNext)
				{
					static_assert(Index > 0);
					return m_nextElements.template GetAt<Index - 1>();
				}
				else
				{
					static_unreachable("Invalid index!");
				}
			}

			[[nodiscard]] PURE_STATICS constexpr AnyView GetAt(const size index) LIFETIME_BOUND
			{
				switch (index)
				{
					case 0:
						return m_value;
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.GetAt(index - 1);
						}
						else
						{
							return {};
						}
					}
				}
			}

			[[nodiscard]] PURE_STATICS constexpr ConstAnyView GetAt(const size index) const LIFETIME_BOUND
			{
				switch (index)
				{
					case 0:
						return m_value;
					default:
					{
						if constexpr (HasNext)
						{
							return m_nextElements.GetAt(index - 1);
						}
						else
						{
							return {};
						}
					}
				}
			}

			template<typename Type>
			[[nodiscard]] PURE_STATICS static constexpr size GetFirstIndex()
			{
				if constexpr (TypeTraits::IsSame<ElementType, Type>)
				{
					return 0;
				}
				else if constexpr (HasNext)
				{
					return NextElements::template GetFirstIndex<Type>() + 1;
				}
				else
				{
					static_unreachable("Type was not present in variant!");
					return 0;
				}
			}

			template<typename Type>
			[[nodiscard]] PURE_STATICS static constexpr bool ContainsType()
			{
				if constexpr (TypeTraits::IsSame<ElementType, Type>)
				{
					return true;
				}
				else if constexpr (HasNext)
				{
					return NextElements::template ContainsType<Type>();
				}
				else
				{
					return false;
				}
			}

			constexpr void MoveFrom(VariantStorage&& other, const size activeIndex)
			{
				switch (activeIndex)
				{
					case 0:
						new (Memory::GetAddressOf(m_value)) ElementType(Move(other.m_value));
						break;
					default:
					{
						if constexpr (HasNext)
						{
							m_nextElements.MoveFrom(Move(other.m_nextElements), activeIndex - 1);
						}
					}
				}
			}

			constexpr void CopyFrom(const VariantStorage& other, const size activeIndex)
			{
				switch (activeIndex)
				{
					case 0:
						new (Memory::GetAddressOf(m_value)) ElementType(other.m_value);
						break;
					default:
					{
						if constexpr (HasNext)
						{
							m_nextElements.CopyFrom(other.m_nextElements, activeIndex - 1);
						}
					}
				}
			}

			template<typename Type>
			inline static constexpr size FirstIndex = GetFirstIndex<Type>();

			union
			{
				ElementType m_value;
				NextElements m_nextElements;
			};
		};

		template<typename VariantType, bool IsTriviallyDestructible>
		struct VariantBase
		{
		};

		template<typename VariantType>
		struct VariantBase<VariantType, false>
		{
			~VariantBase()
			{
				static_cast<VariantType&>(*this).Destroy();
			}
		};
	}

	template<typename... ArgumentTypes>
	struct TRIVIAL_ABI Variant
		: private Internal::VariantBase<
				Variant<ArgumentTypes...>,
				Internal::VariantStorage<TypeTraits::All<TypeTraits::IsTriviallyDestructible<ArgumentTypes>...>, ArgumentTypes...>::
					IsTriviallyDestructible>
	{
		inline static constexpr bool IsEqualityComparable = TypeTraits::Any<TypeTraits::IsEqualityComparable<ArgumentTypes, ArgumentTypes>...>;
		using StorageType = Internal::VariantStorage<TypeTraits::All<TypeTraits::IsTriviallyDestructible<ArgumentTypes>...>, ArgumentTypes...>;
		using BaseType = Internal::VariantBase<Variant<ArgumentTypes...>, StorageType::IsTriviallyDestructible>;
		using IndexType = Memory::NumericSize<sizeof...(ArgumentTypes)>;
		inline static constexpr IndexType Size = sizeof...(ArgumentTypes);
		inline static constexpr bool IsTriviallyDestructible = StorageType::IsTriviallyDestructible;

		constexpr Variant()
		{
		}
		constexpr Variant(Variant&& other)
			: m_activeIndex(other.m_activeIndex)
			, m_storage(Move(other.m_storage), m_activeIndex)
		{
			other.m_activeIndex = InvalidIndex;
		}
		constexpr Variant& operator=(Variant&& other)
		{
			m_activeIndex = other.m_activeIndex;
			other.m_activeIndex = InvalidIndex;
			m_storage.MoveFrom(Move(other.m_storage), m_activeIndex);
			return *this;
		}
		constexpr Variant(const Variant& other)
			: m_activeIndex(other.m_activeIndex)
			, m_storage(other.m_storage, m_activeIndex)
		{
		}
		constexpr Variant(Variant& other)
			: m_activeIndex(other.m_activeIndex)
			, m_storage(other.m_storage, m_activeIndex)
		{
		}
		constexpr Variant& operator=(const Variant& other)
		{
			m_activeIndex = other.m_activeIndex;
			m_storage.CopyFrom(other.m_storage, m_activeIndex);
			return *this;
		}
		constexpr Variant& operator=(Variant& other)
		{
			m_activeIndex = other.m_activeIndex;
			m_storage.CopyFrom(other.m_storage, m_activeIndex);
			return *this;
		}
		template<
			typename Type,
			typename VariantStorageType = StorageType,
			typename = EnableIf<VariantStorageType::template ContainsType<Type>()>>
		constexpr Variant(Type&& value)
			: m_activeIndex((IndexType)StorageType::template GetFirstIndex<Type>())
			, m_storage(Forward<Type>(value))
		{
		}
		template<
			typename Type,
			typename VariantStorageType = StorageType,
			typename = EnableIf<VariantStorageType::template ContainsType<Type>()>>
		constexpr Variant& operator=(Type&& value)
		{
			Destroy();
			m_activeIndex = (IndexType)StorageType::template GetFirstIndex<Type>();
			new (Memory::GetAddressOf(m_storage)) StorageType(Forward<Type>(value));
			return *this;
		}
		template<
			typename Type,
			typename VariantStorageType = StorageType,
			typename = EnableIf<VariantStorageType::template ContainsType<Type>()>>
		constexpr Variant(const Type& value)
			: m_activeIndex((IndexType)StorageType::template GetFirstIndex<Type>())
			, m_storage(value)
		{
		}
		template<
			typename Type,
			typename VariantStorageType = StorageType,
			typename = EnableIf<VariantStorageType::template ContainsType<Type>()>>
		constexpr Variant& operator=(const Type& value)
		{
			Destroy();
			m_activeIndex = (IndexType)StorageType::template GetFirstIndex<Type>();
			new (Memory::GetAddressOf(m_storage)) StorageType(Type(value));
			return *this;
		}
		template<
			typename Type,
			typename VariantStorageType = StorageType,
			typename = EnableIf<VariantStorageType::template ContainsType<Type>()>>
		constexpr Variant(Type& value)
			: m_activeIndex((IndexType)StorageType::template GetFirstIndex<Type>())
			, m_storage(static_cast<const Type&>(value))
		{
		}
		template<
			typename Type,
			typename VariantStorageType = StorageType,
			typename = EnableIf<VariantStorageType::template ContainsType<Type>()>>
		constexpr Variant& operator=(Type& value)
		{
			Destroy();
			m_activeIndex = (IndexType)StorageType::template GetFirstIndex<Type>();
			new (Memory::GetAddressOf(m_storage)) StorageType(Type(value));
			return *this;
		}

		template<typename Callback>
		constexpr auto Visit(Callback&& callback)
		{
			return m_storage.Visit(Forward<Callback>(callback), m_activeIndex);
		}
		template<typename Callback>
		constexpr auto Visit(Callback&& callback) const
		{
			return m_storage.Visit(Forward<Callback>(callback), m_activeIndex);
		}

		template<typename... Callbacks>
		constexpr auto Visit(Callbacks&&... callbacks)
		{
			static_assert(sizeof...(Callbacks) == Size + 1);
			return m_storage.Visit(m_activeIndex, Forward<Callbacks>(callbacks)...);
		}
		template<typename... Callbacks>
		constexpr auto Visit(Callbacks&&... callbacks) const
		{
			static_assert(sizeof...(Callbacks) == Size + 1);
			return m_storage.Visit(m_activeIndex, Forward<Callbacks>(callbacks)...);
		}

		template<typename Type>
		inline static constexpr IndexType TypeIndex = StorageType::template FirstIndex<Type>;

		template<size Index>
		using TypeAtIndex = typename StorageType::template Type<Index>;

		template<typename Type>
		[[nodiscard]] PURE_STATICS static constexpr bool ContainsType()
		{
			return StorageType::template ContainsType<Type>();
		}

		template<IndexType Index>
		[[nodiscard]] PURE_STATICS constexpr Optional<TypeAtIndex<Index>*> GetAt() LIFETIME_BOUND
		{
			using Type = TypeAtIndex<Index>;
			Type& element = m_storage.template GetAt<Index>();
			return Optional<Type*>{Memory::GetAddressOf(element), Index == m_activeIndex};
		}

		template<IndexType Index>
		[[nodiscard]] PURE_STATICS constexpr Optional<const TypeAtIndex<Index>*> GetAt() const LIFETIME_BOUND
		{
			using Type = TypeAtIndex<Index>;
			const Type& element = m_storage.template GetAt<Index>();
			return Optional<const Type*>{Memory::GetAddressOf(element), Index == m_activeIndex};
		}

		auto& GetStorage() const LIFETIME_BOUND
		{
			return m_storage;
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS constexpr Optional<Type*> Get() LIFETIME_BOUND
		{
			constexpr IndexType index = TypeIndex<Type>;
			Type& element = m_storage.template GetAt<index>();
			return Optional<Type*>(Memory::GetAddressOf(element), index == m_activeIndex);
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS constexpr Optional<const Type*> Get() const LIFETIME_BOUND
		{
			constexpr IndexType index = TypeIndex<Type>;
			const Type& element = m_storage.template GetAt<index>();
			return Optional<const Type*>(Memory::GetAddressOf(element), index == m_activeIndex);
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS constexpr Type& GetExpected() LIFETIME_BOUND
		{
			constexpr IndexType index = TypeIndex<Type>;
			Expect(index == m_activeIndex);
			return m_storage.template GetAt<index>();
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS constexpr const Type& GetExpected() const LIFETIME_BOUND
		{
			constexpr IndexType index = TypeIndex<Type>;
			Expect(index == m_activeIndex);
			return m_storage.template GetAt<index>();
		}

		template<typename Type, typename... Args>
		[[nodiscard]] constexpr EnableIf<TypeTraits::HasConstructor<Type, Args...>, Type&> GetOrEmplace(Args&&... args)
		{
			constexpr IndexType index = TypeIndex<Type>;
			if (index != m_activeIndex)
			{
				*this = Type(Forward<Args>(args)...);
			}
			return m_storage.template GetAt<index>();
		}

		[[nodiscard]] PURE_STATICS constexpr AnyView Get() LIFETIME_BOUND
		{
			return m_storage.GetAt(m_activeIndex);
		}

		[[nodiscard]] PURE_STATICS constexpr ConstAnyView Get() const LIFETIME_BOUND
		{
			return m_storage.GetAt(m_activeIndex);
		}

		template<IndexType Index>
		[[nodiscard]] PURE_STATICS constexpr bool Is() const
		{
			return Index == m_activeIndex;
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS constexpr bool Is() const
		{
			return Is<StorageType::template FirstIndex<Type>>();
		}

		template<typename Type>
		[[nodiscard]] PURE_STATICS constexpr bool Implements() const
		{
			return m_storage.Visit(
				[](auto& value)
				{
					using ValueType = TypeTraits::WithoutReference<decltype(value)>;
					if constexpr (TypeTraits::IsBaseOf<Type, ValueType> || TypeTraits::IsSame<Type, ValueType>)
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			);
		}

		[[nodiscard]] PURE_STATICS constexpr Reflection::TypeDefinition GetActiveType() const
		{
			return m_storage.GetTypeDefinition(m_activeIndex);
		}

		inline static constexpr IndexType InvalidIndex = Size;
		[[nodiscard]] PURE_STATICS constexpr IndexType GetActiveIndex() const
		{
			return m_activeIndex;
		}

		[[nodiscard]] PURE_STATICS constexpr bool HasValue() const
		{
			return m_activeIndex != InvalidIndex;
		}

		template<bool IsComparable = IsEqualityComparable, typename = EnableIf<IsComparable>>
		[[nodiscard]] PURE_STATICS constexpr bool operator==(const Variant& other) const
		{
			return m_activeIndex == other.m_activeIndex && (m_activeIndex == InvalidIndex || m_storage.Equals(other.m_storage, m_activeIndex));
		}
		template<bool IsComparable = IsEqualityComparable, typename = EnableIf<IsComparable>>
		[[nodiscard]] PURE_STATICS constexpr bool operator!=(const Variant& other) const
		{
			return !operator==(other);
		}
	protected:
		friend BaseType;

		constexpr void Destroy()
		{
			if constexpr (!IsTriviallyDestructible)
			{
				if (m_activeIndex != InvalidIndex)
				{
					Visit(
						[](auto& value)
						{
							using ValueType = TypeTraits::WithoutReference<decltype(value)>;
							value.~ValueType();
						}
					);
				}
			}
			m_activeIndex = InvalidIndex;
		}
	protected:
		IndexType m_activeIndex = InvalidIndex;
		StorageType m_storage;
	};
}
