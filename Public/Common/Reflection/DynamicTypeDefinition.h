#pragma once

#include <Common/Memory/IsAligned.h>
#include <Common/Memory/Containers/Vector.h>
#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>
#include <Common/Asset/AssetFormat.h>
#include <Common/Reflection/TypeDefinition.h>
#include <Common/Reflection/TypeInterface.h>
#include <Common/Reflection/DynamicPropertyInfo.h>

namespace ngine::Reflection
{
	struct Registry;

	enum class TypeDefinitionType : uint8
	{
		Invalid,
		//! Native type known at compile time
		Native,
		//! Nested types are laid out sequentially based on alignment
		Structure,
		//! Nested types make up a dynamic variant
		Variant
	};

	struct DynamicTypeDefinition final : public TypeDefinition, public TypeInterface
	{
		inline static constexpr ngine::Asset::Format AssetFormat = {"{DCE92B86-A9A7-4676-9E5D-A09CEA4824F4}"_guid, MAKE_PATH(".type.nasset")};

		using Type = TypeDefinitionType;
		using VariantIndexType = uint32;

		static void IterateProperties(const TypeInterface& typeInterface, IteratePropertiesCallback&& callback);
		static void IterateFunctions(const TypeInterface& typeInterface, IterateFunctionsCallback&&);
		static void IterateEvents(const TypeInterface& typeInterface, IterateEventsCallback&&);

		DynamicTypeDefinition();

		DynamicTypeDefinition(
			const Guid typeGuid,
			const ConstUnicodeStringView name,
			const ConstUnicodeStringView description = {},
			const EnumFlags<TypeFlags> flags = {},
			const TypeInterface* pParent = nullptr
		);

		void SetName(UnicodeString&& name);

		[[nodiscard]] static DynamicTypeDefinition MakeStructure(Vector<DynamicPropertyInfo>&& properties);
		[[nodiscard]] static DynamicTypeDefinition MakeStructure(const ArrayView<const Reflection::TypeDefinition> types);
		[[nodiscard]] static DynamicTypeDefinition MakeVariant(const ArrayView<const Reflection::TypeDefinition> types);

		bool Serialize(const Serialization::Reader reader);
		bool Serialize(Serialization::Writer writer) const;

		void RecalculateMemoryData();

		[[nodiscard]] PURE_STATICS Type GetType() const
		{
			return m_type;
		}

		[[nodiscard]] constexpr bool operator==(const DynamicTypeDefinition& other) const
		{
			return TypeDefinition::operator==(other) && m_type == other.m_type && m_storedName == other.m_storedName &&
			       m_properties.GetView() == other.m_properties.GetView();
		}
		[[nodiscard]] constexpr bool operator!=(const DynamicTypeDefinition& other) const
		{
			return !operator==(other);
		}

		[[nodiscard]] PURE_STATICS ArrayView<const DynamicPropertyInfo> GetProperties() const LIFETIME_BOUND
		{
			switch (m_type)
			{
				case Type::Invalid:
				case Type::Native:
					ExpectUnreachable();
				case Type::Structure:
					return m_properties.GetView();
				case Type::Variant:
					return {};
			}
			ExpectUnreachable();
		}
		[[nodiscard]] PURE_STATICS ArrayView<const DynamicPropertyInfo> GetVariantFields() const LIFETIME_BOUND
		{
			switch (m_type)
			{
				case Type::Invalid:
				case Type::Native:
					ExpectUnreachable();
				case Type::Structure:
					return {};
				case Type::Variant:
					return m_properties.GetView();
			}
			ExpectUnreachable();
		}

		//! Whether any of the variant fields support the specified types as a value
		template<typename OtherType>
		[[nodiscard]] PURE_STATICS bool SupportsType() const
		{
			switch (m_type)
			{
				case Type::Invalid:
				case Type::Native:
					ExpectUnreachable();
				case Type::Structure:
					return false;
				case Type::Variant:
				{
					const Reflection::TypeDefinition typeDefinition = Reflection::TypeDefinition::Get<OtherType>();
					for (const DynamicPropertyInfo& property : m_properties)
					{
						if (property.m_typeDefinition == typeDefinition)
						{
							return true;
						}
					}

					return false;
				}
			}
			ExpectUnreachable();
		}
	protected:
		void ConstructAt(void* pAddress) const;
		void CopyConstructAt(void* pAddress, const void* pSourceAddress) const;
		void MoveConstructAt(void* pAddress, void* pSourceAddress) const;
		void DestroyAt(void* pAddress) const;

		[[nodiscard]] bool AreEqual(const void* pLeftAddress, const void* pRightAddress) const;

		[[nodiscard]] bool Serialize(void* pAddress, const Serialization::Reader reader) const;
		[[nodiscard]] bool Serialize(const void* pAddress, Serialization::Writer writer) const;

		[[nodiscard]] uint32 CalculateFixedCompressedDataSize(EnumFlags<Reflection::PropertyFlags> requiredFlags = {}) const;
		[[nodiscard]] uint32
		CalculateDynamicCompressedDataSize(const void* pAddress, EnumFlags<Reflection::PropertyFlags> requiredFlags = {}) const;
		[[nodiscard]] bool Compress(const void* pAddress, BitView& target, const EnumFlags<Reflection::PropertyFlags> requiredFlags = {}) const;
		[[nodiscard]] bool
		Decompress(void* pAddress, ConstBitView& source, const EnumFlags<Reflection::PropertyFlags> requiredFlags = {}) const;

		static void Manage(const Operation operation, OperationArgument& argument);

		[[nodiscard]] virtual PropertyQueryResult FindProperty(const Guid propertyGuid) const override final;
		[[nodiscard]] virtual Optional<const FunctionInfo*> FindFunction([[maybe_unused]] const Guid functionGuid) const override final;
		[[nodiscard]] virtual Optional<const EventInfo*> FindEvent([[maybe_unused]] const Guid eventGuid) const override final;

		[[nodiscard]] virtual ArrayView<const Tag::Guid> GetTags() const override;

		[[nodiscard]] virtual Optional<const ExtensionInterface*> FindExtension([[maybe_unused]] const Guid typeGuid) const override final;
	public:
		Type m_type{Type::Structure};
		UnicodeString m_storedName;
		uint32 m_size = 0;
		uint16 m_alignment = 1;
		Vector<DynamicPropertyInfo> m_properties;
	};
}
