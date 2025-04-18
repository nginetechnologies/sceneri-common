#pragma once

#include <Common/Storage/Identifier.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Scripting
{
	struct FunctionIdentifier : public TIdentifier<uint32, 12>
	{
		using BaseType = TIdentifier<uint32, 12>;
		FunctionIdentifier(const BaseType identifier)
			: BaseType(identifier)
		{
		}
		using BaseType::BaseType;
		using BaseType::operator=;

		bool Serialize(const Serialization::Reader reader);
		bool Serialize(const Serialization::Writer writer) const;
	};
}
