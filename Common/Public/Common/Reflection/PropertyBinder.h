#pragma once

#include <Common/Memory/Containers/String.h>
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Memory/Any.h>
#include <Common/Reflection/Type.h>

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Reflection
{
	// TODO(Ben): This could eventually be just replaced by generic map support
	class PropertyBinder
	{
	public:
		using PropertyMap = UnorderedMap<String, Any, String::Hash>;

		static constexpr Guid TypeGuid = "def8d15b-ca64-49ea-b388-afdf6c6a5b21"_guid;
	public:
		PropertyBinder();
		PropertyBinder(const PropertyBinder& other) noexcept;
		PropertyBinder(PropertyBinder&& other) noexcept;
		~PropertyBinder();

		void SetProperty(String&& key, Any&& value);
		[[nodiscard]] ConstAnyView GetProperty(ConstStringView key) const;

		[[nodiscard]] const PropertyMap& GetProperties() const;

		void Reset();

		void DeserializeCustomData(const Optional<Serialization::Reader> pReader);
		bool SerializeCustomData(Serialization::Writer writer) const;

		bool operator==(const PropertyBinder& other) const;
		PropertyBinder& operator=(const PropertyBinder& other) noexcept;
		PropertyBinder& operator=(PropertyBinder&& other) noexcept;
	protected:
		friend struct Reflection::ReflectedType<PropertyBinder>;
	private:
		PropertyMap m_properties;
	};

	template<>
	struct ReflectedType<PropertyBinder>
	{
		static constexpr auto Type = Reflection::Reflect<PropertyBinder>(PropertyBinder::TypeGuid, MAKE_UNICODE_LITERAL("Property Binder"));
	};
}
