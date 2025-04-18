#pragma once

#include <Common/Memory/Tuple.h>
#include <Common/Memory/Bitset.h>
#include <Common/Memory/GetNumericSize.h>
#include <Common/TypeTraits/IsMoveConstructible.h>
#include <Common/TypeTraits/IsCopyConstructible.h>
#include <Common/TypeTraits/IsMoveAssignable.h>
#include <Common/TypeTraits/IsCopyAssignable.h>
#include <Common/TypeTraits/IsMemberFunction.h>
#include <Common/TypeTraits/IsMemberVariable.h>
#include <Common/TypeTraits/HasMemberFunction.h>
#include <Common/TypeTraits/HasFunctionCallOperator.h>
#include <Common/TypeTraits/IsInvocable.h>
#include <Common/TypeTraits/IsReference.h>
#include <Common/TypeTraits/ReturnType.h>
#include <Common/TypeTraits/MemberOwnerType.h>
#include <Common/TypeTraits/MemberType.h>
#include <Common/TypeTraits/GetParameterTypes.h>

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

#include <Common/Reflection/TypeDefinition.h>
#include <Common/Reflection/DynamicPropertyInstance.h>
#include <Common/Reflection/PropertyInfo.h>
#include <Common/Reflection/IsReflected.h>
#include <Common/Reflection/GetType.h>

namespace ngine::Threading
{
	struct JobBatch;
}

namespace ngine::Reflection
{
	inline static constexpr uint8 MaximumPropertyCount = 255;
	using PropertyMask = Bitset<MaximumPropertyCount>;
	using PropertyIndex = Memory::NumericSize<MaximumPropertyCount>;

	namespace Internal
	{
		template<class T, class U = void>
		struct TParentType
		{
			using Type = T;
		};

		template<class T>
		struct TParentType<T, typename TypeTraits::Void<typename T::ParentType>>
		{
			using Type = typename T::ParentType;
		};

		using DummyFunction = void (*)();

		template<typename OwnerType, typename Type_, typename OnChangeCallback>
		struct MemberVariablePointerProperty : public PropertyInfo
		{
			using Type = Type_;
			using ParentType = typename TParentType<OwnerType>::Type;
			using MemberVariablePointer = Type OwnerType::*;
			inline static constexpr bool IsMemberVariable = true;

			HasMemberVariable(Type, TypeGuid);
			inline static constexpr auto TypeGuid = []()
			{
				if constexpr (Reflection::IsReflected<Type>)
				{
					return Reflection::GetTypeGuid<Type>();
				}
				else if constexpr (HasTypeGuid)
				{
					return Type::TypeGuid;
				}
				else
				{
					return Guid();
				}
			}();

			constexpr MemberVariablePointerProperty(
				const ConstUnicodeStringView displayName,
				const ConstStringView name,
				const Guid guid,
				const ConstUnicodeStringView categoryDisplayName,
				MemberVariablePointer memberPointer,
				const OnChangeCallback onChangeCallback,
				const PropertyFlags flags
			)
				: PropertyInfo{displayName, name, guid, categoryDisplayName, flags, TypeGuid, TypeDefinition::Get<Type>()}
				, m_variablePointer(memberPointer)
				, m_onChangeCallback(onChangeCallback)
			{
			}

			constexpr void SetPropertyIndex(const PropertyIndex index)
			{
				m_propertyIndex = index;
			}
			[[nodiscard]] constexpr PropertyIndex GetPropertyIndex() const
			{
				return m_propertyIndex;
			}

			[[nodiscard]] constexpr MemberVariablePointer GetMemberVariablePointer() const
			{
				return m_variablePointer;
			}

			using ReturnType = Type&;
			[[nodiscard]] constexpr ReturnType GetValue(OwnerType& owner, [[maybe_unused]] const Optional<ParentType*> pParent = Invalid) const
			{
				return owner.*m_variablePointer;
			}
			using ConstReturnType = const Type&;
			[[nodiscard]] constexpr ConstReturnType
			GetValue(const OwnerType& owner, [[maybe_unused]] const Optional<const ParentType*> pParent = Invalid) const
			{
				return owner.*m_variablePointer;
			}

			//! Sets the value of this property, and notifies any listeners of the change
			inline constexpr void SetValueSingle(OwnerType& owner, Optional<ParentType*> pParent, Type&& newValue) const
			{
				owner.*m_variablePointer = Forward<Type>(newValue);

				OnChangedValueSingle(owner, pParent);
			}

			//! Sets the value of the property, and notifies the owner if it had opted to listen to this specific property changing
			//! Note that this does _not_ notify the owner's OnPropertiesChanged(mask) callback. This should be done separately after all
			//! properties have been changed.
			inline constexpr void SetValueBatched(OwnerType& owner, Optional<ParentType*> pParent, Type&& newValue) const
			{
				owner.*m_variablePointer = Forward<Type>(newValue);

				OnChangedValueBatched(owner, pParent);
			}

			[[nodiscard]] inline DynamicPropertyInstance GetDynamicInstance() const
			{
				if constexpr (TypeTraits::IsBaseOf<PropertyOwner, OwnerType>)
				{
					return DynamicPropertyInstance{
						[this](PropertyOwner& owner, [[maybe_unused]] const Optional<PropertyOwner*> pParent) -> Any
						{
							return GetValue(static_cast<OwnerType&>(owner));
						},
						[this](PropertyOwner& owner, const Optional<PropertyOwner*> pParent, Any&& anyNewValue)
						{
							Type& newValue = anyNewValue.GetExpected<Type>();
							SetValueSingle(static_cast<OwnerType&>(owner), static_cast<ParentType*>(pParent.Get()), Move(newValue));
						}
					};
				}
				else
				{
					return {};
				}
			}

			template<typename... Args>
			bool Serialize(
				const Serialization::Reader objectReader,
				const Serialization::Reader typeReader,
				OwnerType& owner,
				const Optional<ParentType*> pParent,
				Threading::JobBatch& jobBatchOut,
				Args&&... args
			) const;
			template<typename... Args>
			bool Serialize(Serialization::Writer writer, const OwnerType& owner, const Optional<const ParentType*> pParent, Args&&... args) const;
		protected:
			HasTypeMemberFunction(OwnerType, OnPropertiesChanged, void, PropertyMask);
			HasTypeMemberFunctionNamed(HasOnPropertiesChangedWithParent, OwnerType, OnPropertiesChanged, void, PropertyMask, ParentType&);

			inline void OnChangedValueBatched(OwnerType& owner, const Optional<ParentType*> pParent) const
			{
				if constexpr (TypeTraits::IsMemberFunction<OnChangeCallback> || (TypeTraits::IsMemberVariable<OnChangeCallback> && TypeTraits::HasFunctionCallOperator<TypeTraits::MemberType<OnChangeCallback>>))
				{
					if constexpr (TypeTraits::IsInvocable<OnChangeCallback, void, ParentType&> || TypeTraits::IsInvocable<TypeTraits::MemberType<OnChangeCallback>, void, ParentType&>)
					{
						(owner.*m_onChangeCallback)(*pParent);
					}
					else if constexpr (TypeTraits::IsInvocable<OnChangeCallback, void> || TypeTraits::IsInvocable<TypeTraits::MemberType<OnChangeCallback>, void>)
					{
						(owner.*m_onChangeCallback)();
					}
					else
					{
						static_unreachable("Invalid property member on change callback provided");
					}
				}
				else if (m_onChangeCallback != nullptr)
				{
					if constexpr (TypeTraits::IsInvocable<OnChangeCallback, void, ParentType&>)
					{
						m_onChangeCallback(*pParent);
					}
					else if constexpr (TypeTraits::IsInvocable<OnChangeCallback, void>)
					{
						m_onChangeCallback();
					}
					else
					{
						static_unreachable("Invalid property on change callback provided");
					}
				}
			}

			inline void OnChangedValueSingle(OwnerType& owner, const Optional<ParentType*> pParent) const
			{
				OnChangedValueBatched(owner, pParent);

				if constexpr (HasOnPropertiesChangedWithParent)
				{
					PropertyMask propertyMask;
					propertyMask.Set(1 << m_propertyIndex);
					owner.OnPropertiesChanged(propertyMask, *pParent);
				}
				else if constexpr (HasOnPropertiesChanged)
				{
					PropertyMask propertyMask;
					propertyMask.Set(PropertyMask::BitIndexType(1 << m_propertyIndex));
					owner.OnPropertiesChanged(propertyMask);
				}
			}
		protected:
			const MemberVariablePointer m_variablePointer;
			const OnChangeCallback m_onChangeCallback;
			PropertyIndex m_propertyIndex = 0;
		};

		template<typename OwnerType, typename Type_, typename Setter, typename Getter, typename SetFromDeserializationCallback>
		struct SetterGetterProperty : public PropertyInfo
		{
			using ParentType = typename TParentType<OwnerType>::Type;
			using Type = Type_;
			using MutableType = TypeTraits::WithoutConst<Type>;
			using ReturnType = TypeTraits::ReturnType<Getter>;
			using ConstReturnType = ReturnType;
			using MutableReturnType = TypeTraits::WithoutConst<ReturnType>;
			using SetterType = Setter;
			using GetterType = Getter;
			inline static constexpr bool IsMemberVariable = false;

			HasMemberVariable(Type, TypeGuid);
			inline static constexpr auto TypeGuid = []()
			{
				static_assert(
					Reflection::IsReflected<Type> || HasTypeGuid,
					"Property type has no type guid! Missing include or type wasn't reflected?"
				);

				if constexpr (Reflection::IsReflected<Type>)
				{
					return Reflection::GetTypeGuid<Type>();
				}
				else if constexpr (HasTypeGuid)
				{
					return Type::TypeGuid;
				}
				else
				{
					return Guid();
				}
			}();

			static constexpr auto GetDeserializationGetter(const Setter setter, SetFromDeserializationCallback&& deserializationSetter)
			{
				if constexpr (TypeTraits::IsSame<Setter, SetFromDeserializationCallback>)
				{
					return setter;
				}
				else
				{
					return Forward<SetFromDeserializationCallback>(deserializationSetter);
				}
			}

			constexpr SetterGetterProperty(
				const ConstUnicodeStringView displayName,
				const ConstStringView name,
				const Guid guid,
				const ConstUnicodeStringView categoryDisplayName,
				Setter&& setter,
				Getter&& getter,
				SetFromDeserializationCallback&& deserializationSetter,
				const PropertyFlags flags
			)
				: PropertyInfo{displayName, name, guid, categoryDisplayName, flags, TypeGuid, TypeDefinition::Get<TypeTraits::WithoutConst<Type>>()}
				, m_setter(Forward<Setter>(setter))
				, m_getter(Forward<Getter>(getter))
				, m_deserializationSetter(GetDeserializationGetter(m_setter, Forward<SetFromDeserializationCallback>(deserializationSetter)))
			{
			}

			constexpr void SetPropertyIndex(const PropertyIndex index)
			{
				m_propertyIndex = index;
			}
			[[nodiscard]] constexpr PropertyIndex GetPropertyIndex() const
			{
				return m_propertyIndex;
			}

			[[nodiscard]] constexpr SetterType GetSetterFunction() const
			{
				return m_setter;
			}
			[[nodiscard]] constexpr GetterType GetGetterFunction() const
			{
				return m_getter;
			}

			[[nodiscard]] ReturnType
			GetDeserializationTarget([[maybe_unused]] OwnerType& owner, [[maybe_unused]] const Optional<ParentType*> pParent) const
			{
				if constexpr (TypeTraits::IsDefaultConstructible<ReturnType>)
				{
					return ReturnType();
				}
				else
				{
					return GetValue(owner, pParent);
				}
			}

			[[nodiscard]] constexpr ReturnType GetValueInternal(OwnerType& owner, [[maybe_unused]] const Optional<ParentType*> pParent) const
			{
				if constexpr (TypeTraits::IsMemberFunction<Getter>)
				{
					if constexpr (TypeTraits::IsInvocable<Getter, ReturnType, ParentType&> || TypeTraits::IsInvocable<TypeTraits::MemberType<Getter>, ReturnType, ParentType&>)
					{
						return (owner.*m_getter)(*pParent);
					}
					else if constexpr (TypeTraits::IsInvocable<Getter, ReturnType> || TypeTraits::IsInvocable<TypeTraits::MemberType<Getter>, ReturnType>)
					{
						return (owner.*m_getter)();
					}
					else
					{
						static_unreachable("Invalid property getter member function was provided!");
					}
				}
				else if constexpr (TypeTraits::IsInvocable<Getter, ReturnType, ParentType&>)
				{
					return m_getter(owner, *pParent);
				}
				else if constexpr (TypeTraits::IsInvocable<Getter, ReturnType>)
				{
					return m_getter(owner);
				}
				else
				{
					static_unreachable("Invalid property getter member function was provided!");
				}
			}

			[[nodiscard]] constexpr ReturnType GetValue(OwnerType& owner, const Optional<ParentType*> pParent) const
			{
				return GetValueInternal(owner, pParent);
			}
			[[nodiscard]] constexpr ReturnType GetValue(const OwnerType& owner, const Optional<const ParentType*> pParent) const
			{
				return GetValueInternal(const_cast<OwnerType&>(owner), const_cast<ParentType*>(pParent.Get()));
			}

			HasTypeMemberFunction(OwnerType, OnPropertiesChanged, void, PropertyMask);
			HasTypeMemberFunctionNamed(HasOnPropertiesChangedWithParent, OwnerType, OnPropertiesChanged, void, PropertyMask, ParentType&);

			constexpr void SetValueBatched(OwnerType& owner, const Optional<ParentType*> pParent, MutableType&& newValue) const
			{
				// Allow this to return a job that needs to be queued
				if constexpr (TypeTraits::IsMemberFunction<Setter>)
				{
					if constexpr (TypeTraits::IsInvocable<Setter, void, ParentType&, MutableType> || TypeTraits::IsInvocable<Setter, void, ParentType&, MutableType&&>)
					{
						(owner.*m_setter)(*pParent, Forward<MutableType>(newValue));
					}
					else if constexpr (TypeTraits::IsInvocable<Setter, void, MutableType> || TypeTraits::IsInvocable<TypeTraits::MemberType<Setter>, void, MutableType>)
					{
						(owner.*m_setter)(Forward<MutableType>(newValue));
					}
					else
					{
						static_unreachable("Invalid setter member function signature provided by property!");
					}
				}
				else
				{
					if constexpr (TypeTraits::IsInvocable<Setter, void, OwnerType&, ParentType&, MutableType&&>)
					{
						m_setter(owner, *pParent, Forward<MutableType>(newValue));
					}
					else if constexpr (TypeTraits::IsInvocable<Setter, void, OwnerType&, MutableType&&>)
					{
						m_setter(owner, Forward<MutableType>(newValue));
					}
					else
					{
						static_unreachable("Invalid setter function signature provided by property!");
					}
				}
			}

			void SetValueSingle(OwnerType& owner, const Optional<ParentType*> pParent, MutableType&& newValue) const
			{
				SetValueBatched(owner, pParent, Forward<MutableType>(newValue));

				if constexpr (HasOnPropertiesChangedWithParent)
				{
					PropertyMask propertyMask;
					propertyMask.Set(1 << m_propertyIndex);
					owner.OnPropertiesChanged(propertyMask, *pParent);
				}
				else if constexpr (HasOnPropertiesChanged)
				{
					PropertyMask propertyMask;
					propertyMask.Set(1 << m_propertyIndex);
					owner.OnPropertiesChanged(propertyMask);
				}
			}

			[[nodiscard]] auto SetDeserializedValue(
				OwnerType& owner,
				const Optional<ParentType*> pParent,
				MutableType&& newValue,
				const Serialization::Reader objectReader,
				const Serialization::Reader typeReader
			) const;

			[[nodiscard]] DynamicPropertyInstance GetDynamicInstance() const
			{
				if constexpr (TypeTraits::IsBaseOf<PropertyOwner, OwnerType>)
				{
					return DynamicPropertyInstance{
						[this](PropertyOwner& owner, const Optional<PropertyOwner*> pParent) -> Any
						{
							if constexpr (TypeTraits::IsConst<ReturnType>)
							{
								return MutableReturnType(GetValue(static_cast<OwnerType&>(owner), static_cast<ParentType*>(pParent.Get())));
							}
							else
							{
								return GetValue(static_cast<OwnerType&>(owner), static_cast<ParentType*>(pParent.Get()));
							}
						},
						[this](PropertyOwner& owner, const Optional<PropertyOwner*> pParent, Any&& anyNewValue)
						{
							MutableType& newValue = anyNewValue.GetExpected<MutableType>();
							SetValueSingle(static_cast<OwnerType&>(owner), static_cast<ParentType*>(pParent.Get()), Move(newValue));
						}
					};
				}
				else
				{
					return {};
				}
			}

			bool Serialize(
				const Serialization::Reader objectReader,
				const Serialization::Reader typeReader,
				OwnerType& owner,
				const Optional<ParentType*> pParent,
				Threading::JobBatch& jobBatchOut
			) const;
			bool Serialize(Serialization::Writer writer, const OwnerType& owner, const Optional<const ParentType*> pParent) const;
		private:
			const Setter m_setter;
			const Getter m_getter;
			const SetFromDeserializationCallback m_deserializationSetter;
			PropertyIndex m_propertyIndex = 0;
		};
	}

	template<typename OwnerType, typename Type, typename OnChangeCallback = Internal::DummyFunction>
	constexpr auto MakeProperty(
		const ConstUnicodeStringView displayName,
		const ConstStringView name,
		const Guid guid,
		const ConstUnicodeStringView categoryDisplayName,
		const PropertyFlags flags,
		Type OwnerType::*memberPointer,
		const OnChangeCallback onChangeCallback = nullptr
	)
	{
		return Internal::MemberVariablePointerProperty<OwnerType, Type, OnChangeCallback>(
			displayName,
			name,
			guid,
			categoryDisplayName,
			memberPointer,
			onChangeCallback,
			flags
		);
	}

	template<typename OwnerType, typename Type, typename OnChangeCallback = Internal::DummyFunction>
	constexpr auto MakeProperty(
		const ConstUnicodeStringView displayName,
		const ConstStringView name,
		const Guid guid,
		const ConstUnicodeStringView categoryDisplayName,
		Type OwnerType::*memberPointer,
		const OnChangeCallback onChangeCallback = nullptr
	)
	{
		return MakeProperty<OwnerType, Type, OnChangeCallback>(
			displayName,
			name,
			guid,
			categoryDisplayName,
			PropertyFlags{},
			memberPointer,
			onChangeCallback
		);
	}

	template<typename OwnerType, typename Type, typename OnChangeCallback = Internal::DummyFunction>
	struct Property : public Internal::MemberVariablePointerProperty<OwnerType, Type, OnChangeCallback>
	{
		using BaseType = Internal::MemberVariablePointerProperty<OwnerType, Type, OnChangeCallback>;

		constexpr Property(
			const ConstUnicodeStringView displayName,
			const ConstStringView name,
			const Guid guid,
			const ConstUnicodeStringView categoryDisplayName,
			const PropertyFlags flags,
			Type OwnerType::*memberPointer,
			const OnChangeCallback onChangeCallback = nullptr
		)
			: BaseType(displayName, name, guid, categoryDisplayName, memberPointer, onChangeCallback, flags)
		{
		}
	};

	template<typename Getter, typename Setter, typename SetFromDeserializationCallback = Internal::DummyFunction>
	constexpr auto MakeDynamicProperty(
		const ConstUnicodeStringView displayName,
		const ConstStringView name,
		const Guid guid,
		const ConstUnicodeStringView categoryDisplayName,
		const PropertyFlags flags,
		Setter&& setter,
		Getter&& getter,
		SetFromDeserializationCallback&& deserializationSetter = nullptr
	)
	{
		using Type = TypeTraits::WithoutReference<TypeTraits::ReturnType<Getter>>;

		if constexpr (TypeTraits::IsMemberFunction<Setter>)
		{
			using OwnerType = TypeTraits::MemberOwnerType<Setter>;
			return Internal::SetterGetterProperty<OwnerType, Type, Setter, Getter, SetFromDeserializationCallback>(
				displayName,
				name,
				guid,
				categoryDisplayName,
				Forward<Setter>(setter),
				Forward<Getter>(getter),
				Forward<SetFromDeserializationCallback>(deserializationSetter),
				flags
			);
		}
		else
		{
			using ArgumentTypes = TypeTraits::GetParameterTypes<Setter>;
			using OwnerType = TypeTraits::WithoutReference<typename ArgumentTypes::template Type<0>>;

			return Internal::SetterGetterProperty<OwnerType, Type, Setter, Getter, SetFromDeserializationCallback>(
							 displayName,
							 name,
							 guid,
							 categoryDisplayName,
							 Forward<Setter>(setter),
							 Forward<Getter>(getter)
						 ),
			       Forward<SetFromDeserializationCallback>(deserializationSetter), flags;
		}
	}

	template<typename Getter, typename Setter, typename SetFromDeserializationCallback = Internal::DummyFunction>
	constexpr auto MakeDynamicProperty(
		const ConstUnicodeStringView displayName,
		const ConstStringView name,
		const Guid guid,
		const ConstUnicodeStringView categoryDisplayName,
		Setter&& setter,
		Getter&& getter,
		SetFromDeserializationCallback&& deserializationSetter = nullptr,
		const EnumFlags<PropertyFlags> propertyFlags = {}
	)
	{
		return MakeDynamicProperty<Getter, Setter, SetFromDeserializationCallback>(
			displayName,
			name,
			guid,
			categoryDisplayName,
			propertyFlags.GetFlags(),
			Forward<Setter>(setter),
			Forward<Getter>(getter),
			Forward<SetFromDeserializationCallback>(deserializationSetter)
		);
	}

	template<typename... ArgumentTypes>
	struct Properties : public Tuple<ArgumentTypes...>
	{
		using BaseType = Tuple<ArgumentTypes...>;
		using BaseType::BaseType;
		using BaseType::operator=;
	};
	template<typename... Ts>
	Properties(const Ts&...) -> Properties<Ts...>;
	template<typename... Ts>
	Properties(Ts&&...) -> Properties<Ts...>;
};
