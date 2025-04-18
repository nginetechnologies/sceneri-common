#pragma once

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine::Serialization
{
	template<typename Type>
	struct RawPointer
	{
		bool Serialize(const Serialization::Reader reader)
		{
			Assert(reader.GetContext() == Serialization::Context::Duplication || reader.GetContext() == Serialization::Context::UndoHistory);
			m_pPointer = reinterpret_cast<Type*>(reader.ReadWithDefaultValue<uintptr>(nullptr));
			return true;
		}

		bool Serialize(Serialization::Writer writer) const
		{
			Assert(writer.GetContext() == Serialization::Context::Duplication || writer.GetContext() == Serialization::Context::UndoHistory);
			uintptr address = reinterpret_cast<uintptr>(m_pPointer);
			return writer.SerializeInPlace(address);
		}

		Type* m_pPointer = nullptr;
	};
}
