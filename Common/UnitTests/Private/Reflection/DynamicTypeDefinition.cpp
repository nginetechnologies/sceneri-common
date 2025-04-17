#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Reflection/DynamicTypeDefinition.h>
#include <Common/Reflection/Registry.inl>
#include <Common/Reflection/Type.h>
#include <Common/Math/Transform.h>
#include <Common/Serialization/Deserialize.h>
#include <Common/Serialization/Serialize.h>
#include <Common/Reflection/GenericType.h>
#include <Common/Reflection/Serialization/Type.h>
#include <Common/System/Query.h>

namespace ngine::Tests
{
	struct NativeType
	{
		NativeType() = default;
		Math::WorldTransform m_transform = Math::WorldTransform(Math::Identity, Math::WorldCoordinate(0, 1, 2));
	};

	struct NativeType2
	{
		NativeType2() = default;
		Math::WorldTransform m_transform = Math::WorldTransform(Math::Identity, Math::WorldCoordinate(1, 2, 3));
	};
}

namespace ngine::Reflection
{
	template<>
	struct ReflectedType<Tests::NativeType>
	{
		inline static constexpr auto Type = Reflection::Reflect<Tests::NativeType>(
			"{11D4F24F-5F67-4302-8F50-C09879469164}"_guid,
			MAKE_UNICODE_LITERAL("Test Type"),
			Reflection::TypeFlags::DisableDynamicCloning | Reflection::TypeFlags::DisableDynamicDeserialization |
				Reflection::TypeFlags::DisableDynamicInstantiation,
			Reflection::Tags{},
			Reflection::Properties{Reflection::Property{
				MAKE_UNICODE_LITERAL("Transform"),
				"Transform",
				"{744A8035-5B7E-492F-9877-5991579888BA}"_guid,
				MAKE_UNICODE_LITERAL("Transform"),
				Reflection::PropertyFlags{},
				&Tests::NativeType::m_transform
			}}
		);
	};

	template<>
	struct ReflectedType<Tests::NativeType2>
	{
		inline static constexpr auto Type = Reflection::Reflect<Tests::NativeType2>(
			"{DA9ED82D-A854-4B43-8C2A-D0AF86844B9F}"_guid,
			MAKE_UNICODE_LITERAL("Test Type 2"),
			Reflection::TypeFlags::DisableDynamicCloning | Reflection::TypeFlags::DisableDynamicDeserialization |
				Reflection::TypeFlags::DisableDynamicInstantiation,
			Reflection::Tags{},
			Reflection::Properties{Reflection::Property{
				MAKE_UNICODE_LITERAL("Transform"),
				"Transform",
				"{6C77FE37-CC7F-4F1F-BDB4-3801D53A798E}"_guid,
				MAKE_UNICODE_LITERAL("Transform"),
				Reflection::PropertyFlags{},
				&Tests::NativeType2::m_transform
			}}
		);
	};
}

namespace ngine::Tests
{
	UNIT_TEST(Reflection, DynamicTypeDefinition_Runtime)
	{
		using ExpectedType = Tuple<NativeType, NativeType2>;

		Reflection::Registry registry;
		registry.RegisterDynamicType<NativeType>();
		registry.RegisterDynamicType<NativeType2>();

		Reflection::DynamicTypeDefinition dynamicType{Guid::Generate(), MAKE_UNICODE_LITERAL("Custom Type")};

		dynamicType.m_properties.EmplaceBack(Reflection::PropertyInfo{
			MAKE_UNICODE_LITERAL("Property 1"),
			"Property 1",
			"{6F133CF7-610F-44B2-AF04-DE106B84BECB}"_guid,
			MAKE_UNICODE_LITERAL("Property 1"),
			{},
			{},
			Reflection::GetType<NativeType>().GetTypeDefinition()
		});
		dynamicType.m_properties.EmplaceBack(Reflection::PropertyInfo{
			MAKE_UNICODE_LITERAL("Property 2"),
			"Property 2",
			"{3862FECD-184D-46F7-903C-2E93565E6265}"_guid,
			MAKE_UNICODE_LITERAL("Property 2"),
			{},
			{},
			Reflection::GetType<NativeType2>().GetTypeDefinition()
		});
		dynamicType.RecalculateMemoryData();

		EXPECT_EQ(dynamicType.GetSize(), sizeof(ExpectedType));
		EXPECT_EQ(dynamicType.GetAlignment(), alignof(ExpectedType));

		Any dynamicObject(Memory::DefaultConstruct, dynamicType);
		EXPECT_TRUE(Memory::IsAligned(dynamicObject.GetData(), alignof(ExpectedType)));
		ExpectedType& value = *static_cast<ExpectedType*>(dynamicObject.GetData());
		const ExpectedType referenceValue{};

		EXPECT_TRUE(value.Get<0>().m_transform.IsEquivalentTo(referenceValue.Get<0>().m_transform));
		EXPECT_TRUE(value.Get<1>().m_transform.IsEquivalentTo(referenceValue.Get<1>().m_transform));
	}

	UNIT_TEST(Reflection, DynamicTypeDefinition_Deserialized)
	{
		using ExpectedType = Tuple<NativeType, NativeType2>;

		Reflection::Registry registry;
		System::Query::GetInstance().RegisterSystem(registry);
		registry.RegisterDynamicType<NativeType>();
		registry.RegisterDynamicType<NativeType2>();

		const ConstStringView jsonContents = R"(
{
    "guid": "69D3FC2E-8049-4BFD-B268-8A672F81E346",
    "name": "My Custom Type",
    "properties": [
        {
            "name": "My Property 1",
            "type": "11D4F24F-5F67-4302-8F50-C09879469164"
        },
        {
            "name": "My Property 2",
            "type": "DA9ED82D-A854-4B43-8C2A-D0AF86844B9F"
        }
    ]
}
)";

		Reflection::DynamicTypeDefinition dynamicType;
		const bool wasRead = Serialization::DeserializeFromBuffer(jsonContents, dynamicType);
		EXPECT_TRUE(wasRead);

		EXPECT_EQ(dynamicType.GetSize(), sizeof(ExpectedType));
		EXPECT_EQ(dynamicType.GetAlignment(), alignof(ExpectedType));

		Any dynamicObject(Memory::DefaultConstruct, dynamicType);
		EXPECT_TRUE(Memory::IsAligned(dynamicObject.GetData(), alignof(ExpectedType)));
		ExpectedType& value = *static_cast<ExpectedType*>(dynamicObject.GetData());
		const ExpectedType referenceValue{};

		EXPECT_TRUE(value.Get<0>().m_transform.IsEquivalentTo(referenceValue.Get<0>().m_transform));
		EXPECT_TRUE(value.Get<1>().m_transform.IsEquivalentTo(referenceValue.Get<1>().m_transform));

		Optional<String> result = Serialization::SerializeToBuffer(dynamicType, Serialization::SavingFlags::HumanReadable);
		EXPECT_TRUE(result.IsValid());
		const ConstStringView expectedJsonContents = R"({
    "type": 2,
    "guid": "69d3fc2e-8049-4bfd-b268-8a672f81e346",
    "name": "My Custom Type",
    "properties": [
        {
            "name": "My Property 1",
            "type": "11d4f24f-5f67-4302-8f50-c09879469164",
            "owner_offset": 0,
            "offset_to_next": 48
        },
        {
            "name": "My Property 2",
            "type": "da9ed82d-a854-4b43-8c2a-d0af86844b9f",
            "owner_offset": 48,
            "offset_to_next": 0
        }
    ]
})";

		EXPECT_EQ(*result, expectedJsonContents);

		System::Query::GetInstance().DeregisterSystem<Reflection::Registry>();
	}
}
