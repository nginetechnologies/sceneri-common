#pragma once

#include <Common/Memory/Containers/Array.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/Memory/Containers/ByteView.h>
#include <Common/Memory/Optional.h>
#include <Common/Platform/StaticUnreachable.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/TypeTraits/IsEnum.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/TypeTraits/IsPrimitive.h>
#include <Common/Memory/Endian.h>

namespace ngine::IO
{
	enum class SeekOrigin : uint8
	{
		StartOfFile = 0,
		CurrentPosition = 1,
		EndOfFile = 2
	};

	struct File;

	struct TRIVIAL_ABI FileView
	{
		FileView() = default;
		FileView(void* pFile)
			: m_pFile(pFile)
		{
		}

		using SizeType = long long;

		template<typename CharType, size Size>
		size Write(const CharType(data)[Size]) const
		{
			Expect(m_pFile != nullptr);
			return Write(data, sizeof(CharType), Size);
		}

		template<typename Type, typename SizeType>
		size Write(const ArrayView<Type, SizeType> view) const
		{
			Expect(m_pFile != nullptr);
			return Write(view.GetData(), sizeof(char), view.GetDataSize());
		}

		size Write(const ConstStringView view) const
		{
			Expect(m_pFile != nullptr);
			return Write(view.GetData(), sizeof(char), view.GetSize());
		}

		size Write(const ConstByteView view) const
		{
			Expect(m_pFile != nullptr);
			return Write(view.GetData(), sizeof(ByteType), view.GetDataSize());
		}

		size Write(const char character) const
		{
			Expect(m_pFile != nullptr);
			return WriteCharacter(character);
		}

		size Write(const unsigned char character) const
		{
			Expect(m_pFile != nullptr);
			return WriteCharacter(character);
		}

		size Write(const uint16 character) const
		{
			Expect(m_pFile != nullptr);
			return Write(&character, sizeof(uint16), 1);
		}

		size Write(const int16 character) const
		{
			Expect(m_pFile != nullptr);
			return Write(&character, sizeof(int16), 1);
		}

		size Write(const uint32 character) const
		{
			Expect(m_pFile != nullptr);
			return Write(&character, sizeof(uint32), 1);
		}

		size Write(const int32 character) const
		{
			Expect(m_pFile != nullptr);
			return Write(&character, sizeof(int32), 1);
		}

		size Write(const uint64 character) const
		{
			Expect(m_pFile != nullptr);
			return Write(&character, sizeof(uint64), 1);
		}

		size Write(const int64 character) const
		{
			Expect(m_pFile != nullptr);
			return Write(&character, sizeof(int64), 1);
		}

		size Write(const FileView other) const
		{
			Expect(m_pFile != nullptr);
			Expect(other.m_pFile != nullptr);
			char buffer[1024];
			size bytes;
			size writtenBytes = 0;

			while (0 < (bytes = other.Read(buffer, 1, sizeof(buffer))))
			{
				writtenBytes += Write(buffer, 1, bytes);
			}
			return writtenBytes;
		}

		void Write(const File&) const = delete;

		template<typename OtherType>
		size Write(const OtherType& value) const
		{
			if constexpr (TypeTraits::IsEnum<OtherType>)
			{
				return Write((UNDERLYING_TYPE(OtherType))value);
			}
			else
			{
				return Write(&value, sizeof(OtherType), 1);
			}
		}

		template<typename CharType, size Size>
		[[nodiscard]] bool Read(CharType (&data)[Size]) const
		{
			Expect(m_pFile != nullptr);
			const size readDataSize = Read(data, sizeof(char), Size * sizeof(CharType));
			return readDataSize == sizeof(CharType) * Size;
		}

		template<typename Type>
		[[nodiscard]] bool Read(Type* pElements, const size numElements = 1) const
		{
			Expect(m_pFile != nullptr);
			const size readDataSize = Read(pElements, sizeof(char), (sizeof(Type) * numElements) / sizeof(char));
			return (readDataSize == sizeof(Type) * numElements);
		}

		[[nodiscard]] bool Read(char* pElements, const size numElements) const
		{
			Expect(m_pFile != nullptr);
			const size readDataSize = Read(pElements, sizeof(char), numElements);
			return (readDataSize == sizeof(char) * numElements);
		}

		template<typename T>
		Optional<T> Read() const
		{
			if constexpr (TypeTraits::IsEnum<T>)
			{
				using EnumUnderlyingType = UNDERLYING_TYPE(T);
				if (Optional<EnumUnderlyingType> value = Read<EnumUnderlyingType>())
				{
					return static_cast<T>(*value);
				}
				return Invalid;
			}
			else if constexpr (TypeTraits::IsPrimitive<T>)
			{
				T data;
				const bool wasRead = Read(&data);
				return {data, wasRead};
			}
			else
			{
				T value;
				return {value, Read(&value)};
			}
		}

		template<typename Type, typename SizeType, typename IndexType>
		[[nodiscard]] bool ReadIntoView(const ArrayView<Type, SizeType, IndexType> data) const
		{
			return Read(data.GetData(), static_cast<size>(data.GetSize()));
		}
		template<typename Type, size Size, typename IndexType>
		[[nodiscard]] bool ReadIntoView(const FixedArrayView<Type, Size, IndexType> data) const
		{
			return Read(data.GetData(), Size);
		}

		[[nodiscard]] bool ReadLineIntoView(const ArrayView<char, uint32> data) const;

		template<typename Type, size Size>
		Optional<Array<Type, Size>> ReadArray() const
		{
			Array<Type, Size> data;
			const bool wasRead = ReadIntoView(data.GetView());
			return {data, wasRead};
		}

		template<typename EnumType>
		EnableIf<TypeTraits::IsEnum<EnumType>, Optional<EnumType>> ReadEnum() const
		{
			if (Optional<UNDERLYING_TYPE(EnumType)> value = Read<UNDERLYING_TYPE(EnumType)>())
			{
				return static_cast<EnumType>(*value);
			}
			return Invalid;
		}

		[[nodiscard]] SizeType Tell() const;
		[[nodiscard]] SizeType GetSize() const;
		[[nodiscard]] bool ReachedEnd() const
		{
			return Tell() == GetSize();
		}

		[[nodiscard]] bool Seek(const long offset, const SeekOrigin origin) const;

		void Flush() const;

		[[nodiscard]] void* GetFile() const LIFETIME_BOUND
		{
			return m_pFile;
		}
		[[nodiscard]] bool IsValid() const
		{
			return m_pFile != nullptr;
		}
	protected:
		size Write(const void* pBuffer, const size elementSize, const size count) const;
		int WriteCharacter(const int character) const;
		size Read(void* pBuffer, const size elementSize, const size count) const;
	protected:
		void* m_pFile = nullptr;
	};
}
