#pragma once

#include "SavingFlags.h"

#include <Common/Serialization/ForwardDeclarations/SerializedData.h>
#include <Common/Serialization/Common.h>
#include <Common/Serialization/Context.h>
#include <Common/EnumFlags.h>
#include <Common/Memory/Containers/StringBase.h>
#include <Common/Memory/Containers/Vector.h>
#include <Common/EnumFlags.h>
#include <Common/IO/File.h>
#include <Common/IO/ForwardDeclarations/ZeroTerminatedPathView.h>

namespace ngine::Serialization
{
	struct Data
	{
		Data(const EnumFlags<ContextFlags> contextFlags = {})
			: m_document(Document(nullptr, 0))
			, m_contextFlags(contextFlags)
		{
		}

		Data(const IO::ConstZeroTerminatedPathView filePath)
			: Data(IO::File(filePath, IO::AccessModeFlags::ReadBinary, IO::SharingFlags::DisallowWrite))
		{
		}

		Data(const IO::FileView jsonFile)
			: m_contextFlags(ContextFlags::FromDisk)
		{
			if (jsonFile.IsValid())
			{
				const uint32 size = static_cast<uint32>(jsonFile.GetSize());
				FixedSizeVector<char, uint32> jsonContents(Memory::ConstructWithSize, Memory::Zeroed, size);
				if (LIKELY(jsonFile.ReadIntoView(jsonContents.GetView())))
				{
					m_document.Parse(jsonContents.GetData(), jsonContents.GetSize());
				}
				else
				{
					m_document = Document(rapidjson::Type::kNullType);
				}
			}
			else
			{
				m_document = Document(rapidjson::Type::kNullType);
			}
		}

		Data(const ConstStringView jsonData)
			: m_contextFlags(ContextFlags::FromBuffer)
		{
			m_document.Parse(jsonData.GetData(), jsonData.GetSize());
		}

		Data(const rapidjson::Type type, const EnumFlags<ContextFlags> contextFlags = {})
			: m_document(type)
			, m_contextFlags(contextFlags)
		{
		}

		Data(const Value& value, const EnumFlags<ContextFlags> contextFlags = {})
			: m_document(value.GetType())
			, m_contextFlags(contextFlags)
		{
			m_document.CopyFrom(value, m_document.GetAllocator());
		}

		Data(Document&& document, const EnumFlags<ContextFlags> contextFlags = {})
			: m_document(Forward<Document>(document))
			, m_contextFlags(contextFlags)
		{
		}

		explicit Data(const Data& other) = default;
		Data& operator=(const Data& other) = default;
		Data(Data&& other) = default;
		Data& operator=(Data&& other) = default;
		[[nodiscard]] Document& GetDocument() LIFETIME_BOUND
		{
			return m_document;
		}
		[[nodiscard]] bool IsValid() const
		{
			return !m_document.HasParseError() && !m_document.IsNull();
		}
		[[nodiscard]] const Document& GetDocument() const LIFETIME_BOUND
		{
			return m_document;
		}

		[[nodiscard]] EnumFlags<ContextFlags> GetContextFlags() const
		{
			return m_contextFlags;
		}

		[[nodiscard]] void SetContextFlags(EnumFlags<ContextFlags> flags)
		{
			m_contextFlags = flags;
		}

		template<typename StringType, typename WriterType>
		StringType SaveToBufferGeneric() const
		{
			using StringBufferType = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
			StringBufferType buffer;
			WriterType jsonWriter(buffer);
			if (UNLIKELY(!m_document.Accept(jsonWriter)))
			{
				return {};
			}

			return StringType{buffer.GetString(), (uint32)buffer.GetSize()};
		}

		template<typename StringType>
		StringType SaveToBuffer(const EnumFlags<SavingFlags> flags) const
		{
			if (flags.IsSet(SavingFlags::HumanReadable))
			{
				using StringBufferType = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
				using PrettyWriterType =
					rapidjson::PrettyWriter<StringBufferType, rapidjson::UTF8<>, rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
				return SaveToBufferGeneric<StringType, PrettyWriterType>();
			}
			else
			{
				using StringBufferType = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
				using WriterType = rapidjson::Writer<StringBufferType, rapidjson::UTF8<>, rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
				return SaveToBufferGeneric<StringType, WriterType>();
			}
		}

		template<typename WriterType>
		[[nodiscard]] bool SaveToFileGeneric(const IO::ConstZeroTerminatedPathView filePath) const
		{
			using StringBufferType = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
			StringBufferType buffer;
			WriterType jsonWriter(buffer);
			jsonWriter.SetMaxDecimalPlaces(4);
			if (UNLIKELY(!m_document.Accept(jsonWriter)))
			{
				return false;
			}

			const IO::File file(filePath, IO::AccessModeFlags::Write);
			if (UNLIKELY(!file.IsValid()))
			{
				return false;
			}

			// TODO
			// return file.Write(buffer.GetString(), buffer.GetSize()) == buffer.GetSize();
			file.Write(ArrayView<const char, size>{buffer.GetString(), buffer.GetSize()});
			return true;
		}

		[[nodiscard]] bool SaveToFile(const IO::ConstZeroTerminatedPathView filePath, const EnumFlags<SavingFlags> flags) const
		{
			if (flags.IsSet(SavingFlags::HumanReadable))
			{
				using StringBufferType = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
				using PrettyWriterType =
					rapidjson::PrettyWriter<StringBufferType, rapidjson::UTF8<>, rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
				return SaveToFileGeneric<PrettyWriterType>(filePath);
			}
			else
			{
				using StringBufferType = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
				using WriterType = rapidjson::Writer<StringBufferType, rapidjson::UTF8<>, rapidjson::UTF8<>, Internal::RapidJsonAllocator>;
				return SaveToFileGeneric<WriterType>(filePath);
			}
		}
	protected:
		Document m_document;
		EnumFlags<ContextFlags> m_contextFlags;
	};
}
