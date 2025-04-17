#pragma once

#include <Common/Reflection/Type.h>
#include <Common/Memory/Any.h>

namespace ngine::Reflection
{
	template<>
	struct ReflectedType<bool>
	{
		static constexpr auto Type = Reflection::Reflect<bool>("72ddbadf-77cb-468d-ba13-45fc48c98b49"_guid, MAKE_UNICODE_LITERAL("Boolean"));
	};
	template<>
	struct ReflectedType<float>
	{
		static constexpr auto Type = Reflection::Reflect<float>("0d0dc49b-4336-4781-a1e7-8c7b84e4cb8d"_guid, MAKE_UNICODE_LITERAL("Float"));
	};
	template<>
	struct ReflectedType<double>
	{
		static constexpr auto Type = Reflection::Reflect<double>("d34e3db7-e026-4a38-a944-552a76cbcc19"_guid, MAKE_UNICODE_LITERAL("Double"));
	};
	template<>
	struct ReflectedType<int32>
	{
		static constexpr auto Type = Reflection::Reflect<int32>("3e65bf78-5d2f-4ecf-88c0-ecfc48dee330"_guid, MAKE_UNICODE_LITERAL("Integer"));
	};
	template<>
	struct ReflectedType<int64>
	{
		static constexpr auto Type = Reflection::Reflect<int64>("1d3557fd-78c3-47b3-bf4e-895d986f325e"_guid, MAKE_UNICODE_LITERAL("Integer64"));
	};
	template<>
	struct ReflectedType<nullptr_type>
	{
		static constexpr auto Type =
			Reflection::Reflect<nullptr_type>("b8b4aa3a-1886-4396-97f5-e210e05aeb90"_guid, MAKE_UNICODE_LITERAL("Nullptr"));
	};
	template<>
	struct ReflectedType<Any>
	{
		static constexpr auto Type = Reflection::Reflect<Any>("04b5e404-b7e0-4970-a1a1-a0d2973a409f"_guid, MAKE_UNICODE_LITERAL("Any"));
	};
}
