#pragma once

#include <Common/Guid.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/Memory/Bitset.h>
#include <Common/Reflection/Tag.h>
#include <Common/Reflection/Property.h>
#include <Common/Reflection/Function.h>
#include <Common/Reflection/Event.h>
#include <Common/Reflection/Extension.h>
#include <Common/Reflection/TypeInterface.h>
#include <Common/Reflection/TypeDefinition.h>
#include <Common/Reflection/GetType.h>
#include <Common/Function/Function.h>
#include <Common/TypeTraits/Void.h>
#include <Common/TypeTraits/IsAbstract.h>
#include <Common/TypeTraits/IsMoveConstructible.h>
#include <Common/TypeTraits/HasMemberFunction.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/EnumFlagOperators.h>

namespace ngine::Threading
{
	struct JobBatch;
}

namespace ngine::Reflection
{
	struct PropertyInfo;
	struct DynamicPropertyInstance;

	namespace Internal
	{
		template<class T, class U = void>
		struct THasBaseType
		{
			inline static constexpr bool Value = false;
		};

		template<class T>
		struct THasBaseType<T, typename TypeTraits::Void<typename T::BaseType>>
		{
			inline static constexpr bool Value = true;
		};

		template<class T, class U = void>
		struct TNextReflectedBaseType
		{
			using Type = void;
		};

		template<class T>
		struct TNextReflectedBaseType<T, typename TypeTraits::Void<typename T::BaseType>>
		{
			using BaseType = typename T::BaseType;
			using Type = TypeTraits::Select<IsReflected<BaseType>, BaseType, typename TNextReflectedBaseType<BaseType>::Type>;
		};

		template<class T>
		using NextReflectedBaseType = typename TNextReflectedBaseType<T>::Type;

		template<class T, class U = void>
		struct TRootType
		{
			using Type = T;
		};

		template<class T>
		struct TRootType<T, typename TypeTraits::Void<typename T::BaseType>>
		{
			using Type =
				TypeTraits::Select<TypeTraits::IsPointerComparable<T, typename T::BaseType>, typename TRootType<typename T::BaseType>::Type, T>;
		};
	}

	template<typename Type>
	inline static constexpr bool HasBaseType = Internal::THasBaseType<Type>::Value;
	template<typename Type>
	using ParentType = typename Internal::TParentType<Type>::Type;
	template<typename Type>
	using RootType = typename Internal::TRootType<Type>::Type;

	namespace Internal
	{
		template<typename T, T>
		struct TypeCheck;

		HasMemberFunction(ShouldSerialize, bool, Serialization::Writer);

		template<typename Type, typename... Args>
		static auto checkHasCustomDataSerialization(int)
			-> decltype(TypeTraits::DeclareValue<const Type&>().SerializeCustomData(TypeTraits::DeclareValue<Serialization::Writer>(), TypeTraits::DeclareValue<Args>()...), TypeTraits::Select<TypeTraits::IsSame<decltype(&Type::SerializeCustomData), bool (Type::*)(Serialization::Writer, Args...) const>, uint8, uint16>());
		template<typename T, typename... Args>
		static uint16 checkHasCustomDataSerialization(...);

		template<typename Type, typename... Args>
		static auto
		checkHasCustomDataDeserialization(int) -> decltype(TypeTraits::DeclareValue<Type&>().DeserializeCustomData(TypeTraits::DeclareValue<const Optional<Serialization::Reader>>(), TypeTraits::DeclareValue<Args>()...), TypeTraits::Select<TypeTraits::IsSame<decltype(&Type::DeserializeCustomData), void (Type::*)(const Optional<Serialization::Reader>, Args...)>, uint8, uint16>());
		template<typename T, typename... Args>
		static uint16 checkHasCustomDataDeserialization(...);
	}

	template<typename Type>
	inline static constexpr bool HasShouldSerialize = Internal::HasShouldSerialize<Type>;
	template<typename Type, typename... Args>
	inline static constexpr bool HasCustomDataSerialization = sizeof(Internal::checkHasCustomDataSerialization<Type, Args...>(0)) ==
	                                                          sizeof(uint8);
	template<typename Type, typename... Args>
	inline static constexpr bool HasCustomDataDeserialization = sizeof(Internal::checkHasCustomDataDeserialization<Type, Args...>(0)) ==
	                                                            sizeof(uint8);

	namespace Internal
	{
		template<class T, class U = void>
		struct GetTypeInitializer
		{
			using Type = TypeInitializer;
		};

		template<class T>
		struct GetTypeInitializer<T, typename TypeTraits::Void<typename T::Initializer>>
		{
			using Type = typename T::Initializer;
		};

		template<class T, class U = void>
		struct GetDynamicTypeInitializer
		{
			using Type = TypeInitializer;
		};

		template<class T>
		struct GetDynamicTypeInitializer<T, typename TypeTraits::Void<typename T::DynamicInitializer>>
		{
			using Type = typename T::DynamicInitializer;
		};
	}

	template<typename Type>
	using CustomTypeInitializer = typename Internal::GetTypeInitializer<Type>::Type;
	template<typename Type>
	using CustomDynamicTypeInitializer = typename Internal::GetDynamicTypeInitializer<Type>::Type;

	namespace Internal
	{
		template<class T, class U = void>
		struct GetTypeDeserializer
		{
			using Type = TypeDeserializer;
		};

		template<class T>
		struct GetTypeDeserializer<T, typename TypeTraits::Void<typename T::Deserializer>>
		{
			using Type = typename T::Deserializer;
		};
	}

	template<typename Type>
	using CustomTypeDeserializer = typename Internal::GetTypeDeserializer<Type>::Type;
	template<typename Type>
	using CustomDynamicTypeDeserializer = typename Internal::GetTypeDeserializer<Type>::Type;

	template<typename OwnerType, typename DeserializerType>
	inline static constexpr bool CanDeserializeInstance =
		TypeTraits::HasConstructor<OwnerType, DeserializerType&&> || TypeTraits::HasConstructor<OwnerType, const DeserializerType&> ||
		TypeTraits::HasConstructor<OwnerType, const Serialization::Reader, const Reflection::Registry&> ||
		TypeTraits::HasConstructor<OwnerType, const Serialization::Reader> ||
		TypeTraits::HasConstructor<OwnerType, const Serialization::Reader, Threading::JobBatch&> ||
		TypeTraits::IsDefaultConstructible<OwnerType>;

	namespace Internal
	{
		template<class T, class U = void>
		struct GetTypeCloner
		{
			using Type = TypeCloner;
		};

		template<class T>
		struct GetTypeCloner<T, typename TypeTraits::Void<typename T::Cloner>>
		{
			using Type = typename T::Cloner;
		};
	}

	template<typename Type>
	using CustomTypeCloner = typename Internal::GetTypeCloner<Type>::Type;
	template<typename Type>
	using CustomDynamicTypeCloner = typename Internal::GetTypeCloner<Type>::Type;

	template<
		typename OwnerType_,
		typename TagsTuple,
		typename PropertiesTuple,
		typename FunctionsTuple,
		typename EventsTuple,
		typename ExtensionsTuple>
	struct Type final : public TypeInterface
	{
		using OwnerType = OwnerType_;
		using ParentType = ParentType<OwnerType>;

		using Initializer = typename Internal::GetTypeInitializer<OwnerType_>::Type;
		using Deserializer = typename Internal::GetTypeDeserializer<OwnerType_>::Type;
		using Cloner = typename Internal::GetTypeCloner<OwnerType_>::Type;

		HasTypeMemberFunction(OwnerType, OnPropertiesChanged, void, PropertyMask);
		HasTypeMemberFunctionNamed(HasOnPropertiesChangedWithParent, OwnerType, OnPropertiesChanged, void, PropertyMask, ParentType&);

		[[nodiscard]] static constexpr EnumFlags<TypeFlags>
		GetDefaultTypeFlags(const PropertiesTuple& properties, const FunctionsTuple& functions)
		{
			// Determine if we have any networked functions
			bool hasNetworkedFunctions{false};
			functions.ForEach(
				[&hasNetworkedFunctions](const auto& function)
				{
					hasNetworkedFunctions |= function.GetFlags().AreAnySet(FunctionFlags::IsNetworkedMask);
				}
			);
			// Determine if we have any networked properties
			bool hasNetworkedProperties{false};
			properties.ForEach(
				[&hasNetworkedProperties](const auto& property)
				{
					hasNetworkedProperties |= property.m_flags.AreAnySet(PropertyFlags::IsNetworkPropagatedMask);
				}
			);

			return (TypeFlags::HasProperties * PropertiesTuple::HasElements) |
			       (TypeFlags::HasInheritedProperties * GetBaseTypeFlags().AreAnySet(TypeFlags::HasProperties | TypeFlags::HasInheritedProperties)
			       ) |
			       TypeFlags::IsAbstract * TypeTraits::IsAbstract<OwnerType> | TypeFlags::HasNetworkedFunctions * hasNetworkedFunctions |
			       TypeFlags::HasNetworkedProperties * hasNetworkedProperties;
		}

		constexpr Type(
			const Guid typeGuid,
			const ConstUnicodeStringView name,
			const ConstUnicodeStringView description,
			const EnumFlags<TypeFlags> flags,
			TagsTuple&& tags,
			PropertiesTuple&& properties,
			FunctionsTuple&& functions,
			EventsTuple&& events,
			ExtensionsTuple&& extensions
		)
			: TypeInterface(
					typeGuid, name, description, flags | GetDefaultTypeFlags(properties, functions), TypeDefinition::Get<OwnerType_>(), GetBaseType()
				)
			, m_tags(Forward<TagsTuple>(tags))
			, m_properties(Forward<PropertiesTuple>(properties))
			, m_functions(Forward<FunctionsTuple>(functions))
			, m_events(Forward<EventsTuple>(events))
			, m_extensions(Forward<ExtensionsTuple>(extensions))
		{
			m_properties.ForEach(
				[propertyIndex = (uint8)0](auto& propertyElement) mutable
				{
					propertyElement.SetPropertyIndex(propertyIndex);
					++propertyIndex;
				}
			);

			m_iteratePropertiesFunction = [](const TypeInterface& typeInterface, IteratePropertiesCallback&& callback)
			{
				const Type& type = static_cast<const Type&>(typeInterface);
				type.IterateProperties(Forward<IteratePropertiesCallback>(callback));
			};
			m_iterateFunctionsFunction = [](const TypeInterface& typeInterface, IterateFunctionsCallback&& callback)
			{
				const Type& type = static_cast<const Type&>(typeInterface);
				type.IterateFunctions(Forward<IterateFunctionsCallback>(callback));
			};
			m_iterateEventsFunction = [](const TypeInterface& typeInterface, IterateEventsCallback&& callback)
			{
				const Type& type = static_cast<const Type&>(typeInterface);
				type.IterateEvents(Forward<IterateEventsCallback>(callback));
			};
		}

		template<typename Callback>
		void IterateProperties(Callback&& callback) const
		{
			m_properties.ForEach(
				[callback = Forward<Callback>(callback)](auto& value)
				{
					callback(value, value.GetDynamicInstance());
				}
			);
		}

		template<typename Callback>
		void IterateFunctions(Callback&& callback) const
		{
			m_functions.ForEach(
				[callback = Forward<Callback>(callback)](auto& value)
				{
					callback(value);
				}
			);
		}

		template<typename Callback>
		void IterateEvents(Callback&& callback) const
		{
			m_events.ForEach(
				[callback = Forward<Callback>(callback)](auto& value)
				{
					callback(value);
				}
			);
		}

		template<typename... Args>
		bool SerializeTypePropertiesInline(
			Serialization::Writer writer, const OwnerType& object, const Optional<const ParentType*> pParent, Args&&... args
		) const;
		bool SerializeTypeProperties(Serialization::Writer writer, const OwnerType& object, const Optional<const ParentType*> pParent) const;
		bool SerializeType(Serialization::Writer writer, const OwnerType& object, const Optional<const ParentType*> pParent) const;
		bool Serialize(Serialization::Writer writer, const OwnerType& object, const Optional<const ParentType*> pParent) const;

		template<typename... Args>
		bool SerializeTypePropertiesInline(
			const Serialization::Reader objectReader,
			const Optional<Serialization::Reader> typeReader,
			OwnerType& object,
			const Optional<ParentType*> pParent,
			Threading::JobBatch& jobBatchOut,
			Args&&... args
		) const;
		bool SerializeTypeProperties(
			const Serialization::Reader reader, OwnerType& object, const Optional<ParentType*> pParent, Threading::JobBatch& jobBatchOut
		) const;
		bool SerializeType(
			const Serialization::Reader reader, OwnerType& object, const Optional<ParentType*> pParent, Threading::JobBatch& jobBatchOut
		) const;
		bool Serialize(
			const Serialization::Reader reader, OwnerType& object, const Optional<ParentType*> pParent, Threading::JobBatch& jobBatchOut
		) const;

		template<bool CanMove = TypeTraits::IsMoveConstructible<OwnerType>>
		[[nodiscard]] EnableIf<CanMove, OwnerType> DeserializeType(Deserializer&&) const;

		[[nodiscard]] virtual PropertyQueryResult FindProperty(const Guid propertyGuid) const override final
		{
			PropertyQueryResult result;
			m_properties.ForEach(
				[&result, propertyGuid](const auto& property)
				{
					if (property.m_guid == propertyGuid)
					{
						result = {&property, property.GetDynamicInstance()};
						return Memory::CallbackResult::Break;
					}
					return Memory::CallbackResult::Continue;
				}
			);
			return result;
		}

		[[nodiscard]] virtual Optional<const FunctionInfo*> FindFunction(const Guid functionGuid) const override final
		{
			Optional<const FunctionInfo*> pResult;
			m_functions.ForEach(
				[&pResult, functionGuid](const auto& function)
				{
					if (function.GetGuid() == functionGuid)
					{
						pResult = &function;
						return Memory::CallbackResult::Break;
					}
					return Memory::CallbackResult::Continue;
				}
			);
			return pResult;
		}

		[[nodiscard]] virtual Optional<const EventInfo*> FindEvent(const Guid eventGuid) const override final
		{
			Optional<const EventInfo*> pResult;
			m_events.ForEach(
				[&pResult, eventGuid](const auto& event)
				{
					if (event.GetGuid() == eventGuid)
					{
						pResult = &event;
						return Memory::CallbackResult::Break;
					}
					return Memory::CallbackResult::Continue;
				}
			);
			return pResult;
		}

		[[nodiscard]] virtual ArrayView<const Tag::Guid> GetTags() const override final
		{
			using TagArray = Array<const Tag::Guid, TagsTuple::ElementCount>;
			const TagArray& tags = reinterpret_cast<const TagArray&>(m_tags);
			return tags.GetDynamicView();
		}

		[[nodiscard]] virtual Optional<const ExtensionInterface*> FindExtension(const Guid typeGuid) const override final
		{
			return m_extensions.template FindIf<Optional<const ExtensionInterface*>>(
				[typeGuid](const auto& __restrict extension) -> Optional<const ExtensionInterface*>
				{
					if (extension.m_guid == typeGuid)
					{
						return &extension;
					}
					else
					{
						return {};
					}
				}
			);
		}

		inline static constexpr bool HasProperties = PropertiesTuple::ElementCount > 0;
		using PropertiesMask = Bitset<PropertiesTuple::ElementCount>;

		inline static constexpr bool HasFunctions = FunctionsTuple::ElementCount > 0;
		inline static constexpr bool HasEvents = EventsTuple::ElementCount > 0;

		static constexpr auto GetBaseType()
		{
			using BaseType = Internal::NextReflectedBaseType<OwnerType>;
			if constexpr (!TypeTraits::IsSame<BaseType, void>)
			{
				return &GetType<BaseType>();
			}
			else
			{
				return nullptr;
			}
		}

		[[nodiscard]] static constexpr EnumFlags<TypeFlags> GetBaseTypeFlags()
		{
			using BaseType = Internal::NextReflectedBaseType<OwnerType>;
			if constexpr (!TypeTraits::IsSame<BaseType, void>)
			{
				return GetType<BaseType>().GetFlags();
			}
			else
			{
				return {};
			}
		}

		template<typename ExtensionType>
		inline static constexpr bool HasExtension = ExtensionsTuple::template ContainsType<ExtensionType>;

		template<typename ExtensionType>
		[[nodiscard]] constexpr const ExtensionType& GetExtension() const
		{
			static_assert(HasExtension<ExtensionType>);
			return m_extensions.template Get<ExtensionsTuple::template FirstIndexOf<ExtensionType>>();
		}

		template<size Index = 0>
		[[nodiscard]] static constexpr bool ContainsExtension(const Guid typeGuid)
		{
			constexpr auto& reflectedType = Reflection::GetType<OwnerType>();
			if constexpr (reflectedType.m_extensions.ElementCount > 0)
			{
				constexpr auto& extension = reflectedType.m_extensions.template Get<Index>();
				if (extension.m_guid == typeGuid)
				{
					return true;
				}
				else if constexpr (Index + 1 < FunctionsTuple::ElementCount)
				{
					return GetExtension<Index + 1>(typeGuid);
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		template<size Index = 0>
		[[nodiscard]] static constexpr auto& GetExtension(const Guid typeGuid)
		{
			constexpr auto& reflectedType = Reflection::GetType<OwnerType>();
			if constexpr (reflectedType.m_extensions.ElementCount > 0)
			{
				constexpr auto& extension = reflectedType.m_extensions.template Get<Index>();
				if (extension.m_guid == typeGuid)
				{
					return extension;
				}
				else if constexpr (Index + 1 < FunctionsTuple::ElementCount)
				{
					return GetExtension<Index + 1>(typeGuid);
				}
				else
				{
					ExpectUnreachable();
				}
			}
			else
			{
				Assert(false, "Attempting to query non-existent extension");
				return reflectedType;
			}
		}

		template<auto Function, size Index = 0>
		[[nodiscard]] static constexpr auto& FindFunction()
		{
			constexpr auto& reflectedType = Reflection::GetType<OwnerType>();
			constexpr auto& function = reflectedType.m_functions.template Get<Index>();
			using ReflectedFunctionType = TypeTraits::WithoutReference<typename FunctionsTuple::template Type<Index>>;

			using FunctionType = TypeTraits::WithoutReference<TypeTraits::WithoutConst<decltype(Function)>>;
			using OtherFunctionType = typename ReflectedFunctionType::FunctionType;

			if constexpr (TypeTraits::IsSame<FunctionType, OtherFunctionType>)
			{
				if constexpr (function.GetFunction() == Function)
				{
					return function;
				}
				else if constexpr (Index + 1 < FunctionsTuple::ElementCount)
				{
					return FindFunction<Function, Index + 1>();
				}
				else
				{
					ExpectUnreachable();
				}
			}
			else if constexpr (Index + 1 < FunctionsTuple::ElementCount)
			{
				return FindFunction<Function, Index + 1>();
			}
			else
			{
				ExpectUnreachable();
			}
		}

		template<auto Property, size Index = 0>
		[[nodiscard]] static constexpr auto& FindProperty()
		{
			constexpr auto& reflectedType = Reflection::GetType<OwnerType>();
			constexpr auto& property = reflectedType.m_properties.template Get<Index>();
			using ReflectedPropertyType = TypeTraits::WithoutReference<typename PropertiesTuple::template Type<Index>>;

			if constexpr (TypeTraits::IsMemberVariable<decltype(Property)>)
			{
				if constexpr (ReflectedPropertyType::IsMemberVariable)
				{
					using PropertyType = TypeTraits::WithoutReference<TypeTraits::WithoutConst<decltype(Property)>>;
					using OtherPropertyType = typename ReflectedPropertyType::MemberVariablePointer;

					if constexpr (TypeTraits::IsSame<PropertyType, OtherPropertyType>)
					{
						if constexpr (property.GetMemberVariablePointer() == Property)
						{
							return property;
						}
						else if constexpr (Index + 1 < PropertiesTuple::ElementCount)
						{
							return FindProperty<Property, Index + 1>();
						}
						else
						{
							ExpectUnreachable();
						}
					}
					else if constexpr (Index + 1 < PropertiesTuple::ElementCount)
					{
						return FindProperty<Property, Index + 1>();
					}
					else
					{
						ExpectUnreachable();
					}
				}
				else if constexpr (Index + 1 < PropertiesTuple::ElementCount)
				{
					return FindProperty<Property, Index + 1>();
				}
				else
				{
					ExpectUnreachable();
				}
			}
			else if constexpr (TypeTraits::IsMemberFunction<decltype(Property)>)
			{
				if constexpr (!ReflectedPropertyType::IsMemberVariable)
				{
					using PropertyFunctionType = TypeTraits::WithoutReference<TypeTraits::WithoutConst<decltype(Property)>>;
					using SetterType = typename ReflectedPropertyType::SetterType;
					using GetterType = typename ReflectedPropertyType::GetterType;

					if constexpr (TypeTraits::IsSame<PropertyFunctionType, SetterType>)
					{
						if constexpr (property.GetSetterFunction() == Property)
						{
							return property;
						}
						else if constexpr (Index + 1 < PropertiesTuple::ElementCount)
						{
							return FindProperty<Property, Index + 1>();
						}
						else
						{
							ExpectUnreachable();
						}
					}
					else if constexpr (TypeTraits::IsSame<PropertyFunctionType, GetterType>)
					{
						if constexpr (property.GetGetterFunction() == Property)
						{
							return property;
						}
						else if constexpr (Index + 1 < PropertiesTuple::ElementCount)
						{
							return FindProperty<Property, Index + 1>();
						}
						else
						{
							ExpectUnreachable();
						}
					}
					else if constexpr (Index + 1 < PropertiesTuple::ElementCount)
					{
						return FindProperty<Property, Index + 1>();
					}
					else
					{
						ExpectUnreachable();
					}
				}
				else if constexpr (Index + 1 < PropertiesTuple::ElementCount)
				{
					return FindProperty<Property, Index + 1>();
				}
				else
				{
					ExpectUnreachable();
				}
			}
			else
			{
				ExpectUnreachable();
			}
		}

		TagsTuple m_tags;
		PropertiesTuple m_properties;
		FunctionsTuple m_functions;
		EventsTuple m_events;
		ExtensionsTuple m_extensions;
	};

	template<
		typename OwnerType,
		typename... TagArgs,
		typename... PropertyArgs,
		typename... FunctionArgs,
		typename... EventArgs,
		typename... ExtensionsArgs>
	constexpr Type<
		OwnerType,
		Tags<TagArgs...>,
		Properties<PropertyArgs...>,
		Functions<FunctionArgs...>,
		Events<EventArgs...>,
		Extensions<ExtensionsArgs...>>
	Reflect(
		const Guid typeGuid,
		const ConstUnicodeStringView name,
		const EnumFlags<TypeFlags> flags = {},
		Tags<TagArgs...>&& tags = {},
		Properties<PropertyArgs...>&& properties = {},
		Functions<FunctionArgs...>&& functions = {},
		Events<EventArgs...>&& events = {},
		Extensions<ExtensionsArgs...>&& extensions = {}
	)
	{
		return Type<
			OwnerType,
			Tags<TagArgs...>,
			Properties<PropertyArgs...>,
			Functions<FunctionArgs...>,
			Events<EventArgs...>,
			Extensions<ExtensionsArgs...>>(
			typeGuid,
			name,
			MAKE_UNICODE_LITERAL(""),
			flags,
			Forward<Tags<TagArgs...>>(tags),
			Forward<Properties<PropertyArgs...>>(properties),
			Forward<Functions<FunctionArgs...>>(functions),
			Forward<Events<EventArgs...>>(events),
			Forward<Extensions<ExtensionsArgs...>>(extensions)
		);
	}
};
