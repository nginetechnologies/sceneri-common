#pragma once

#include "../Type.h"
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/Threading/Jobs/AsyncJob.h>
#include <Common/Threading/Jobs/JobBatch.h>
#include <Common/Memory/UniquePtr.h>
#include <Common/Memory/AddressOf.h>
#include <Common/TypeTraits/HasMemberFunction.h>

#include <Common/Reflection/Serialization/Property.h>

namespace ngine::Reflection
{
	namespace Internal
	{
		HasMemberFunction(GetParent, void);
	}

	template<typename OwnerType, typename InitializerType>
	inline static constexpr bool CanCreateInstance = TypeTraits::HasConstructor<OwnerType, InitializerType&&> ||
	                                                 TypeTraits::HasConstructor<OwnerType, const InitializerType&>;

	template<typename OwnerType, typename InitializerType = typename Internal::GetTypeInitializer<OwnerType>::Type>
	EnableIf<CanCreateInstance<OwnerType, InitializerType>> CreateInstanceInPlace(OwnerType& object, InitializerType&& initializer)
	{
		new (Memory::GetAddressOf(object)) OwnerType(Forward<InitializerType>(initializer));
	}

	template<typename OwnerType, typename InitializerType = typename Internal::GetTypeInitializer<OwnerType>::Type>
	[[nodiscard]] inline EnableIf<TypeTraits::IsMoveConstructible<OwnerType> && CanCreateInstance<OwnerType, InitializerType>, OwnerType>
	CreateInstanceType(InitializerType&& initializer)
	{
		alignas(OwnerType) ByteType objectStorage[sizeof(OwnerType)];
		OwnerType& object = reinterpret_cast<OwnerType&>(objectStorage);
		new (Memory::GetAddressOf(object)) OwnerType(Forward<InitializerType>(initializer));
		return object;
	}

	template<typename OwnerType, typename ClonerType>
	inline static constexpr bool CanCloneInstance = TypeTraits::HasConstructor<OwnerType, const OwnerType&, ClonerType&&> ||
	                                                TypeTraits::HasConstructor<OwnerType, const OwnerType&, const ClonerType&>;

	template<typename OwnerType, typename ClonerType = typename Internal::GetTypeCloner<OwnerType>::Type>
	EnableIf<CanCloneInstance<OwnerType, ClonerType>> CloneInPlace(OwnerType& object, const OwnerType& objectTemplate, ClonerType&& cloner)
	{
		new (Memory::GetAddressOf(object)) OwnerType(objectTemplate, Forward<ClonerType>(cloner));
	}

	template<typename OwnerType, typename ClonerType = typename Internal::GetTypeCloner<OwnerType>::Type>
	[[nodiscard]] inline EnableIf<TypeTraits::IsMoveConstructible<OwnerType> && CanCloneInstance<OwnerType, ClonerType>, OwnerType>
	CloneType(const OwnerType& objectTemplate, ClonerType&& cloner)
	{
		alignas(OwnerType) ByteType objectStorage[sizeof(OwnerType)];
		OwnerType& object = reinterpret_cast<OwnerType&>(objectStorage);
		new (Memory::GetAddressOf(object)) OwnerType(objectTemplate, Forward<ClonerType>(cloner));
		return Move(object);
	}

	template<
		typename OwnerType,
		typename TagsTuple,
		typename PropertiesTuple,
		typename FunctionsTuple,
		typename EventsTuple,
		typename ExtensionsTuple>
	template<typename... Args>
	inline bool Type<OwnerType, TagsTuple, PropertiesTuple, FunctionsTuple, EventsTuple, ExtensionsTuple>::SerializeTypePropertiesInline(
		[[maybe_unused]] Serialization::Writer writer,
		[[maybe_unused]] const OwnerType& object,
		[[maybe_unused]] const Optional<const ParentType*> pParent,
		[[maybe_unused]] Args&&... args
	) const
	{
		bool serializedAny = false;
		if constexpr (HasProperties)
		{
			m_properties.ForEach(
				[object = Memory::GetAddressOf(object), pParent, writer, &serializedAny](auto& value) mutable
				{
					if (value.m_flags.IsNotSet(Reflection::PropertyFlags::NotSavedToDisk))
					{
						serializedAny |= value.Serialize(writer, *object, pParent);
					}
				}
			);
		}

		if constexpr (HasCustomDataSerialization<OwnerType, const ParentType&, Args...>)
		{
			serializedAny |= object.SerializeCustomData(writer, *pParent, Forward<Args>(args)...);
		}

		if constexpr (HasCustomDataSerialization<OwnerType, Args...>)
		{
			serializedAny |= object.SerializeCustomData(writer, Forward<Args>(args)...);
		}

		return serializedAny;
	}

	template<
		typename OwnerType,
		typename TagsTuple,
		typename PropertiesTuple,
		typename FunctionsTuple,
		typename EventsTuple,
		typename ExtensionsTuple>
	inline bool Type<OwnerType, TagsTuple, PropertiesTuple, FunctionsTuple, EventsTuple, ExtensionsTuple>::SerializeTypeProperties(
		Serialization::Writer writer, const OwnerType& object, const Optional<const ParentType*> pParent
	) const
	{
		bool serializedAny = false;
		if constexpr (HasProperties || HasCustomDataSerialization<OwnerType>)
		{
			serializedAny |= writer.SerializeObjectWithCallback(
				m_guid.ToString().GetView(),
				[object = Memory::GetAddressOf(object), pParent, this](Serialization::Writer serializer) -> bool
				{
					return SerializeTypePropertiesInline(serializer, *object, pParent);
				}
			);
		}
		return serializedAny;
	}

	template<
		typename OwnerType,
		typename TagsTuple,
		typename PropertiesTuple,
		typename FunctionsTuple,
		typename EventsTuple,
		typename ExtensionsTuple>
	inline bool Type<OwnerType, TagsTuple, PropertiesTuple, FunctionsTuple, EventsTuple, ExtensionsTuple>::SerializeType(
		Serialization::Writer writer, const OwnerType& object, const Optional<const ParentType*> pParent
	) const
	{
		bool serializedAny = false;
		if constexpr (Reflection::HasBaseType<OwnerType>)
		{
			using BaseType = typename OwnerType::BaseType;
			if constexpr (Reflection::IsReflected<BaseType>)
			{
				using BaseParentType = Reflection::ParentType<BaseType>;
				serializedAny |= GetType<BaseType>().SerializeType(writer, object, static_cast<const BaseParentType*>(pParent.Get()));
			}
		}
		serializedAny |= SerializeTypeProperties(writer, object, pParent);
		return serializedAny;
	}

	template<
		typename OwnerType,
		typename TagsTuple,
		typename PropertiesTuple,
		typename FunctionsTuple,
		typename EventsTuple,
		typename ExtensionsTuple>
	inline bool Type<OwnerType, TagsTuple, PropertiesTuple, FunctionsTuple, EventsTuple, ExtensionsTuple>::Serialize(
		Serialization::Writer writer, const OwnerType& object, const Optional<const ParentType*> pParent
	) const
	{
		if constexpr (Reflection::HasShouldSerialize<OwnerType>)
		{
			if (!static_cast<const OwnerType&>(object).ShouldSerialize(writer))
			{
				return false;
			}
		}

		writer.Serialize("typeGuid", m_guid);

		SerializeType(writer, object, pParent);

		return true;
	}

	template<
		typename OwnerType,
		typename TagsTuple,
		typename PropertiesTuple,
		typename FunctionsTuple,
		typename EventsTuple,
		typename ExtensionsTuple>
	template<typename... Args>
	inline bool Type<OwnerType, TagsTuple, PropertiesTuple, FunctionsTuple, EventsTuple, ExtensionsTuple>::SerializeTypePropertiesInline(
		[[maybe_unused]] const Serialization::Reader objectReader,
		[[maybe_unused]] const Optional<Serialization::Reader> typeReader,
		[[maybe_unused]] OwnerType& object,
		[[maybe_unused]] const Optional<ParentType*> pParent,
		[[maybe_unused]] Threading::JobBatch& jobBatchOut,
		[[maybe_unused]] Args&&... args
	) const
	{
		if constexpr (HasCustomDataDeserialization<OwnerType, ParentType&, Args...>)
		{
			object.DeserializeCustomData(typeReader, *pParent, Forward<Args>(args)...);
		}
		else if constexpr (HasCustomDataDeserialization<OwnerType, Args...>)
		{
			object.DeserializeCustomData(typeReader, Forward<Args>(args)...);
		}

		if (typeReader.IsInvalid())
			return false;

		if constexpr (HasProperties)
		{
			PropertyMask changedPropertiesMask;
			m_properties.ForEach(
				[object = Memory::GetAddressOf(object), pParent, objectReader, typeReader, &changedPropertiesMask, &jobBatchOut](auto& value
			  ) mutable
				{
					if (value.m_flags.IsNotSet(Reflection::PropertyFlags::NotReadFromDisk))
					{
						Threading::JobBatch valueJobBatch;
						if (value.Serialize(objectReader, *typeReader, *object, pParent, valueJobBatch))
						{
							changedPropertiesMask.Set(PropertyMask::BitIndexType(1u << value.GetPropertyIndex()));
							jobBatchOut.QueueAfterStartStage(valueJobBatch);
						}
					}
				}
			);

			if constexpr (HasOnPropertiesChangedWithParent)
			{
				if (changedPropertiesMask.AreAnySet())
				{
					object.OnPropertiesChanged(changedPropertiesMask, *pParent);
				}
			}
			else if constexpr (HasOnPropertiesChanged)
			{
				if (changedPropertiesMask.AreAnySet())
				{
					object.OnPropertiesChanged(changedPropertiesMask);
				}
			}
		}

		return true;
	}

	template<
		typename OwnerType,
		typename TagsTuple,
		typename PropertiesTuple,
		typename FunctionsTuple,
		typename EventsTuple,
		typename ExtensionsTuple>
	inline bool Type<OwnerType, TagsTuple, PropertiesTuple, FunctionsTuple, EventsTuple, ExtensionsTuple>::SerializeTypeProperties(
		const Serialization::Reader reader, OwnerType& object, const Optional<ParentType*> pParent, Threading::JobBatch& jobBatchOut
	) const
	{
		const Optional<Serialization::Reader> typeDataReader = reader.FindSerializer(m_guid.ToString().GetView());
		return SerializeTypePropertiesInline(reader, typeDataReader, object, pParent, jobBatchOut);
	}

	template<
		typename OwnerType,
		typename TagsTuple,
		typename PropertiesTuple,
		typename FunctionsTuple,
		typename EventsTuple,
		typename ExtensionsTuple>
	inline bool Type<OwnerType, TagsTuple, PropertiesTuple, FunctionsTuple, EventsTuple, ExtensionsTuple>::SerializeType(
		const Serialization::Reader reader, OwnerType& object, const Optional<ParentType*> pParent, Threading::JobBatch& jobBatchOut
	) const
	{
		if constexpr (Reflection::HasBaseType<OwnerType>)
		{
			using BaseType = typename OwnerType::BaseType;
			if constexpr (Reflection::IsReflected<BaseType>)
			{
				using BaseParentType = Reflection::ParentType<BaseType>;
				GetType<BaseType>().SerializeType(reader, object, static_cast<BaseParentType*>(pParent.Get()), jobBatchOut);
			}
		}

		SerializeTypeProperties(reader, object, pParent, jobBatchOut);
		return true;
	}

	template<
		typename OwnerType,
		typename TagsTuple,
		typename PropertiesTuple,
		typename FunctionsTuple,
		typename EventsTuple,
		typename ExtensionsTuple>
	inline bool Type<OwnerType, TagsTuple, PropertiesTuple, FunctionsTuple, EventsTuple, ExtensionsTuple>::Serialize(
		const Serialization::Reader reader, OwnerType& object, const Optional<ParentType*> pParent, Threading::JobBatch& jobBatchOut
	) const
	{
		SerializeType(reader, object, pParent, jobBatchOut);
		return true;
	}

	template<
		typename OwnerType_,
		typename TagsTuple,
		typename PropertiesTuple,
		typename FunctionsTuple,
		typename EventsTuple,
		typename ExtensionsTuple>
	template<bool CanDeserialize>
	EnableIf<CanDeserialize, OwnerType_>
	Type<OwnerType_, TagsTuple, PropertiesTuple, FunctionsTuple, EventsTuple, ExtensionsTuple>::DeserializeType(Deserializer&& deserializer
	) const
	{
		Optional<ParentType*> pParent;
		if constexpr (Internal::HasGetParent<Deserializer>)
		{
			pParent = deserializer.GetParent();
		}

		OwnerType object;
		SerializeType(deserializer.m_reader, object, pParent, *deserializer.m_pJobBatch);
		return Move(object);
	}
};
