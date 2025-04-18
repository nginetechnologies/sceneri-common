#pragma once

#include "../Property.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/TypeTraits/IsPrimitive.h>
#include <Common/TypeTraits/IsEnum.h>

namespace ngine::Reflection
{
	namespace Internal
	{
		template<typename OwnerType, typename Type_, typename OnChangeCallback>
		template<typename... Args>
		inline bool MemberVariablePointerProperty<OwnerType, Type_, OnChangeCallback>::Serialize(
			const Serialization::Reader,
			[[maybe_unused]] const Serialization::Reader typeReader,
			[[maybe_unused]] OwnerType& owner,
			[[maybe_unused]] const Optional<ParentType*> pParent,
			[[maybe_unused]] Threading::JobBatch& jobBatch,
			Args&&... args
		) const
		{
			[[maybe_unused]] Type& value = const_cast<Type&>(GetValue(owner));
			if constexpr (Reflection::IsReflected<Type_> && !TypeTraits::IsPrimitive<Type_> && !TypeTraits::IsEnum<Type_>)
			{
				Optional<Serialization::Reader> newReader = typeReader.FindSerializer(m_name);
				if (newReader.IsValid() && Reflection::GetType<Type_>().Serialize(*newReader, value, Invalid, jobBatch))
				{
					OnChangedValueBatched(owner, pParent);
					return true;
				}
				else
				{
					return false;
				}
			}
			else if constexpr (Serialization::Internal::CanRead<Type, Args..., ParentType&>)
			{
				if (typeReader.Serialize(m_name, value, Forward<Args>(args)..., *pParent))
				{
					OnChangedValueBatched(owner, pParent);
					return true;
				}
				else
				{
					return false;
				}
			}
			else if constexpr (Serialization::Internal::CanRead<Type, Args...>)
			{
				if (typeReader.Serialize(m_name, value, Forward<Args>(args)...))
				{
					OnChangedValueBatched(owner, pParent);
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				static_unreachable("Reflected type member could not be serialized!");
				return false;
			}
		}

		template<typename OwnerType, typename Type_, typename OnChangeCallback>
		template<typename... Args>
		inline bool MemberVariablePointerProperty<OwnerType, Type_, OnChangeCallback>::Serialize(
			Serialization::Writer writer,
			[[maybe_unused]] const OwnerType& owner,
			[[maybe_unused]] const Optional<const ParentType*> pParent,
			Args&&... args
		) const
		{
			if constexpr (Reflection::IsReflected<Type_> && !TypeTraits::IsPrimitive<Type_> && !TypeTraits::IsEnum<Type_>)
			{
				return writer.SerializeObjectWithCallback(
					m_name,
					[this, &owner](Serialization::Writer writer)
					{
						return Reflection::GetType<Type_>().Serialize(writer, GetValue(owner), Invalid);
					}
				);
			}
			else if constexpr (Serialization::Internal::CanWrite<Type, Args..., const ParentType&>)
			{
				return writer.Serialize(m_name, GetValue(owner), Forward<Args>(args)..., *pParent);
			}
			else if constexpr (Serialization::Internal::CanWrite<Type, Args...>)
			{
				return writer.Serialize(m_name, GetValue(owner), Forward<Args>(args)...);
			}
			else
			{
				static_unreachable("Reflected type member could not be serialized!");
				return false;
			}
		}

		template<typename OwnerType, typename Type_, typename Setter, typename Getter, typename SetFromDeserializationCallback>
		inline bool SetterGetterProperty<OwnerType, Type_, Setter, Getter, SetFromDeserializationCallback>::Serialize(
			const Serialization::Reader objectReader,
			const Serialization::Reader typeReader,
			OwnerType& owner,
			const Optional<ParentType*> pParent,
			Threading::JobBatch& jobBatchOut
		) const
		{
			static_assert(Serialization::Internal::CanRead<MutableReturnType>, "Reflected type member could not be serialized!");
			MutableReturnType value = GetDeserializationTarget(owner, pParent);
			bool wasRead;
			if constexpr (Serialization::Internal::CanRead<MutableReturnType, ParentType&>)
			{
				wasRead = typeReader.Serialize(m_name, value, *pParent);
			}
			else
			{
				wasRead = typeReader.Serialize(m_name, value);
			}

			if (wasRead)
			{
				if constexpr (TypeTraits::IsSame<TypeTraits::ReturnType<SetFromDeserializationCallback>, void>)
				{
					SetDeserializedValue(owner, pParent, Move(value), objectReader, typeReader);
				}
				else
				{
					jobBatchOut = SetDeserializedValue(owner, pParent, Move(value), objectReader, typeReader);
				}
				return true;
			}
			return false;
		}

		template<typename OwnerType, typename Type_, typename Setter, typename Getter, typename SetFromDeserializationCallback>
		inline bool SetterGetterProperty<OwnerType, Type_, Setter, Getter, SetFromDeserializationCallback>::Serialize(
			Serialization::Writer writer, const OwnerType& owner, [[maybe_unused]] const Optional<const ParentType*> pParent
		) const
		{
			static_assert(Serialization::Internal::CanWrite<ReturnType>, "Reflected type member could not be serialized!");
			const ReturnType memberValue = GetValue(owner, pParent);
			if constexpr (Serialization::Internal::CanWrite<ReturnType, const ParentType&>)
			{
				return writer.Serialize(m_name, memberValue, *pParent);
			}
			else
			{
				return writer.Serialize(m_name, memberValue);
			}
		}

		template<typename OwnerType, typename Type_, typename Setter, typename Getter, typename SetFromDeserializationCallback>
		inline auto SetterGetterProperty<OwnerType, Type_, Setter, Getter, SetFromDeserializationCallback>::SetDeserializedValue(
			OwnerType& owner,
			[[maybe_unused]] const Optional<ParentType*> pParent,
			MutableType&& newValue,
			const Serialization::Reader objectReader,
			const Serialization::Reader typeReader
		) const
		{
			if constexpr (TypeTraits::IsSame<SetFromDeserializationCallback, Internal::DummyFunction>)
			{
				return SetValueBatched(owner, pParent, Forward<MutableType>(newValue));
			}
			// Allow this to return a job that needs to be queued
			else if constexpr (TypeTraits::IsMemberFunction<SetFromDeserializationCallback>)
			{
				if constexpr (TypeTraits::IsInvocable<
												SetFromDeserializationCallback,
												void,
												ParentType&,
												MutableType,
												Serialization::Reader,
												Serialization::Reader>)
				{
					return (owner.*m_deserializationSetter)(*pParent, Forward<MutableType>(newValue), objectReader, typeReader);
				}
				else
				{
					return (owner.*m_deserializationSetter)(Forward<MutableType>(newValue), objectReader, typeReader);
				}
			}
			else
			{
				if constexpr (TypeTraits::IsInvocable<
												SetFromDeserializationCallback,
												void,
												OwnerType&,
												ParentType&,
												MutableType,
												Serialization::Reader,
												Serialization::Reader>)
				{
					return m_deserializationSetter(owner, *pParent, Forward<MutableType>(newValue), objectReader, typeReader);
				}
				else
				{
					return m_deserializationSetter(owner, Forward<MutableType>(newValue), objectReader, typeReader);
				}
			}
		}
	}
}
