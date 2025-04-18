#include <Common/Memory/Containers/StringView.h>

#include <Common/TypeTraits/IsSame.h>
#include <Common/Math/Hash.h>

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <string>

namespace ngine
{
	template struct TStringView<const char, uint32>;
	template struct TStringView<const wchar_t, uint32>;
	template struct TStringView<char, uint32>;
	template struct TStringView<wchar_t, uint32>;
	template struct TStringView<const char, uint16>;
	template struct TStringView<const wchar_t, uint16>;
	template struct TStringView<char, uint16>;
	template struct TStringView<wchar_t, uint16>;

	template<typename InternalCharType, typename InternalSizeType>
	bool TStringView<InternalCharType, InternalSizeType>::Serialize(const Serialization::Reader serializer)
	{
		if constexpr (TypeTraits::IsSame<InternalCharType, const char>)
		{
			const Serialization::Value& __restrict currentElement = serializer.GetValue();
			if (currentElement.IsString())
			{
				*this = TStringView<const char, SizeType>{currentElement.GetString(), (SizeType)currentElement.GetStringLength()};
				return true;
			}
			return false;
		}
		else
		{
			return false;
		}
	}

	template<typename InternalCharType, typename InternalSizeType>
	bool TStringView<InternalCharType, InternalSizeType>::Serialize(Serialization::Writer serializer) const
	{
		if (IsEmpty())
		{
			return false;
		}

		Serialization::Value& __restrict currentElement = serializer.GetValue();
		using MutableCharType = TypeTraits::WithoutConst<InternalCharType>;
		if constexpr (TypeTraits::IsSame<MutableCharType, char>)
		{
			currentElement = Serialization::Value(GetData(), GetSize(), serializer.GetDocument().GetAllocator());
			return true;
		}
		else if constexpr (TypeTraits::IsSame<MutableCharType, wchar_t>)
		{
			TString<char, Memory::DynamicAllocator<char, SizeType>, Memory::VectorFlags::None> tempString(*this);
			currentElement = Serialization::Value(tempString.GetData(), tempString.GetSize(), serializer.GetDocument().GetAllocator());
			return true;
		}
#if IS_UNICODE_CHAR8_UNIQUE_TYPE
		else if constexpr (TypeTraits::IsSame<MutableCharType, UTF8CharType>)
		{
			TString<char, Memory::DynamicAllocator<char, SizeType>, Memory::VectorFlags::None> tempString(*this);
			currentElement = Serialization::Value(tempString.GetData(), tempString.GetSize(), serializer.GetDocument().GetAllocator());
			return true;
		}
#endif
		else if constexpr (TypeTraits::IsSame<MutableCharType, char16_t>)
		{
			TString<char, Memory::DynamicAllocator<char, SizeType>, Memory::VectorFlags::None> tempString(*this);
			currentElement = Serialization::Value(tempString.GetData(), tempString.GetSize(), serializer.GetDocument().GetAllocator());
			return true;
		}
		else if constexpr (TypeTraits::IsSame<MutableCharType, char32_t>)
		{
			TString<char, Memory::DynamicAllocator<char, SizeType>, Memory::VectorFlags::None> tempString(*this);
			currentElement = Serialization::Value(tempString.GetData(), tempString.GetSize(), serializer.GetDocument().GetAllocator());
			return true;
		}
		else
		{
			Assert(false);
			return false;
		}
	}

	template bool TStringView<const char, uint32>::Serialize(Serialization::Reader serializer);
	template bool TStringView<const char, uint16>::Serialize(Serialization::Reader serializer);
	template bool TStringView<const char, uint8>::Serialize(Serialization::Reader serializer);

	template bool TStringView<const char, uint32>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<const char, uint16>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<const char, uint8>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<const wchar_t, uint32>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<const wchar_t, uint16>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<const wchar_t, uint8>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<char, uint32>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<char, uint16>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<char, uint8>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<wchar_t, uint32>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<wchar_t, uint16>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<wchar_t, uint8>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<const char16_t, uint32>::Serialize(Serialization::Writer serializer) const;
	template bool TStringView<const char32_t, uint32>::Serialize(Serialization::Writer serializer) const;
}
