#include <Common/Memory/Containers/String.h>
#include "Common/Memory/Containers/StringView.h"
#include <Common/Memory/Endian.h>
#include "Common/Memory/Containers/Format/StringView.h"
#include "Common/Memory/Containers/Format/String.h"

#if PLATFORM_WINDOWS
#include <Platform/Windows.h>
#endif

#include <cstdio>
#include <string_view>
#include <string>
#include <iostream>
#include <locale>
#include <codecvt>

#if PLATFORM_WINDOWS
#include <uchar.h>
#endif

#include <Common/Math/Hash.h>

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <Common/TypeTraits/IsSame.h>

namespace ngine
{
	namespace Internal
	{
		PUSH_MSVC_WARNINGS
		DISABLE_MSVC_WARNINGS(4996)

		PUSH_CLANG_WARNINGS
		DISABLE_CLANG_WARNING("-Wdeprecated-declarations")

		bool UTF8ToUTF16(TStringView<char16_t> destination, const TStringView<const UTF8CharType> source) noexcept
		{
			Assert(destination.GetSize() >= source.GetSize());

			using Converter = std::codecvt_utf8_utf16<char16_t>;
			Converter converter;
			mbstate_t state = {};

			const UTF8CharType* firstSource = source.begin();
			const UTF8CharType* lastSource = source.end();

			char16_t* firstDestination = destination.begin();
			char16_t* lastDestination = destination.end();
			char16_t* nextDestination;

			for (; firstSource != lastSource;)
			{
				switch (converter.in(state, firstSource, lastSource, firstSource, firstDestination, lastDestination, nextDestination))
				{
					case Converter::partial:
					case Converter::ok:
					{
						if (firstDestination < nextDestination)
						{
							firstDestination = nextDestination;
						}
						else
						{
							return false;
						}
					}
					break;
					case Converter::noconv:
					{
						for (; firstSource != lastSource; ++firstSource, ++firstDestination)
						{
							*firstDestination = static_cast<char16_t>(static_cast<unsigned char>(*firstSource));
						}
						return true;
					}
					case Converter::error:
						return false;
					default:
						ExpectUnreachable();
				}
			}
			return true;
		}

		bool UTF8ToUTF32(TStringView<char32_t> destination, const TStringView<const UTF8CharType> source) noexcept
		{
			Assert(destination.GetSize() >= source.GetSize());

			using Converter = std::codecvt_utf8<char32_t>;
			Converter converter;
			mbstate_t state = {};

			const UTF8CharType* firstSource = source.begin();
			const UTF8CharType* lastSource = source.end();

			char32_t* firstDestination = destination.begin();
			char32_t* lastDestination = destination.end();
			char32_t* nextDestination;

			for (; firstSource != lastSource;)
			{
				switch (converter.in(state, firstSource, lastSource, firstSource, firstDestination, lastDestination, nextDestination))
				{
					case Converter::partial:
					case Converter::ok:
					{
						if (firstDestination < nextDestination)
						{
							firstDestination = nextDestination;
						}
						else
						{
							return false;
						}
					}
					break;
					case Converter::noconv:
					{
						for (; firstSource != lastSource; ++firstSource, ++firstDestination)
						{
							*firstDestination = static_cast<char32_t>(static_cast<unsigned char>(*firstSource));
						}
						return true;
					}
					case Converter::error:
						return false;
					default:
						ExpectUnreachable();
				}
			}
			return true;
		}

		bool UTF16ToUTF8(TStringView<UTF8CharType> destination, const TStringView<const char16_t> source) noexcept
		{
			Assert(destination.GetSize() >= source.GetSize());

			using Converter = std::codecvt_utf8_utf16<char16_t>;
			Converter converter;
			mbstate_t state = {};

			const char16_t* firstSource = source.begin();
			const char16_t* lastSource = source.end();

			UTF8CharType* firstDestination = destination.begin();
			UTF8CharType* lastDestination = destination.end();
			UTF8CharType* nextDestination;

			for (; firstSource != lastSource;)
			{
				switch (converter.out(state, firstSource, lastSource, firstSource, firstDestination, lastDestination, nextDestination))
				{
					case Converter::partial:
					case Converter::ok:
					{
						if (firstDestination < nextDestination)
						{
							firstDestination = nextDestination;
						}
						else
						{
							return false;
						}
					}
					break;
					case Converter::noconv:
					{
						for (; firstSource != lastSource; ++firstSource, ++firstDestination)
						{
							*firstDestination = static_cast<UTF8CharType>(static_cast<unsigned char>(*firstSource));
						}
						return true;
					}
					case Converter::error:
						return false;
					default:
						ExpectUnreachable();
				}
			}
			return true;
		}

		bool UTF16ToUTF32(TStringView<char32_t> destination, const TStringView<const char16_t> source) noexcept
		{
			Assert(destination.GetSize() >= source.GetSize());

			using Converter = std::codecvt_utf16<char32_t>;
			Converter converter;
			mbstate_t state = {};

			const char* firstSource = reinterpret_cast<const char*>(source.begin().Get());
			const char* lastSource = reinterpret_cast<const char*>(source.end().Get());

			char32_t* firstDestination = destination.begin();
			char32_t* lastDestination = destination.end();
			char32_t* nextDestination;

			for (; firstSource != lastSource;)
			{
				switch (converter.in(state, firstSource, lastSource, firstSource, firstDestination, lastDestination, nextDestination))
				{
					case Converter::partial:
					case Converter::ok:
					{
						if (firstDestination < nextDestination)
						{
							firstDestination = nextDestination;
						}
						else
						{
							return false;
						}
					}
					break;
					case Converter::noconv:
					{
						for (; firstSource != lastSource; ++firstSource, ++firstDestination)
						{
							*firstDestination = static_cast<char16_t>(static_cast<unsigned char>(*firstSource));
						}
						return true;
					}
					case Converter::error:
						return false;
					default:
						ExpectUnreachable();
				}
			}

			for (char16_t& character : ArrayView<char16_t>{(char16_t*)destination.GetData(), uint32(destination.GetSize() * 2)})
			{
				character = (char16_t)Memory::ByteSwap((uint16)character);
			}

			return true;
		}

		bool UTF32ToUTF8(TStringView<UTF8CharType> destination, const TStringView<const char32_t> source) noexcept
		{
			Assert(destination.GetSize() >= source.GetSize());

			using Converter = std::codecvt_utf8<char32_t>;
			Converter converter;
			mbstate_t state = {};

			const char32_t* firstSource = source.begin();
			const char32_t* lastSource = source.end();

			UTF8CharType* firstDestination = reinterpret_cast<char*>(destination.begin().Get());
			UTF8CharType* lastDestination = reinterpret_cast<char*>(destination.end().Get());
			UTF8CharType* nextDestination;

			for (; firstSource != lastSource;)
			{
				switch (converter.out(state, firstSource, lastSource, firstSource, firstDestination, lastDestination, nextDestination))
				{
					case Converter::partial:
					case Converter::ok:
					{
						if (firstDestination < nextDestination)
						{
							firstDestination = nextDestination;
						}
						else
						{
							return false;
						}
					}
					break;
					case Converter::noconv:
					{
						for (; firstSource != lastSource; ++firstSource, ++firstDestination)
						{
							*firstDestination = static_cast<UTF8CharType>(static_cast<unsigned char>(*firstSource));
						}
						return true;
					}
					case Converter::error:
						return false;
					default:
						ExpectUnreachable();
				}
			}
			return true;
		}

		bool UTF32ToUTF16(TStringView<char16_t> destination, const TStringView<const char32_t> source) noexcept
		{
			Assert(destination.GetSize() >= source.GetSize());

			using Converter = std::codecvt_utf16<char32_t>;
			Converter converter;
			mbstate_t state = {};

			const char32_t* firstSource = source.begin();
			const char32_t* lastSource = source.end();

			char* firstDestination = reinterpret_cast<char*>(destination.begin().Get());
			char* lastDestination = reinterpret_cast<char*>(destination.end().Get());
			char* nextDestination;

			for (; firstSource != lastSource;)
			{
				switch (converter.out(state, firstSource, lastSource, firstSource, firstDestination, lastDestination, nextDestination))
				{
					case Converter::partial:
					case Converter::ok:
					{
						if (firstDestination < nextDestination)
						{
							firstDestination = nextDestination;
						}
						else
						{
							return false;
						}
					}
					break;
					case Converter::noconv:
					{
						for (; firstSource != lastSource; ++firstSource, ++firstDestination)
						{
							*firstDestination = static_cast<char>(static_cast<unsigned char>(*firstSource));
						}

						for (char16_t& character : destination)
						{
							character = (char16_t)Memory::ByteSwap((uint16)character);
						}

						return true;
					}
					case Converter::error:
						return false;
					default:
						ExpectUnreachable();
				}
			}

			for (char16_t& character : destination)
			{
				character = (char16_t)Memory::ByteSwap((uint16)character);
			}
			return true;
		}

		UnicodeString FromModifiedUTF8(TStringView<const char> source) noexcept
		{
			UnicodeString destination;
			destination.Reserve(source.GetSize() * 2);

			for (auto sourceIt = source.begin(), endIt = source.end(); sourceIt < endIt;)
			{
				const unsigned char byte = *sourceIt;

				if (byte == 0xC0 && sourceIt + 1 < endIt && (unsigned char)*(sourceIt + 1) == 0x80)
				{
					// Handle modified UTF-8 null character (U+0000)
					destination += MAKE_UNICODE_LITERAL('\0');
					sourceIt += 2;
				}
				else if ((byte & 0xF0) == 0xE0 && sourceIt + 2 < endIt)
				{
					// Handle 3-byte UTF-8 sequence (potential surrogate pair)
					const unsigned char byte2 = *(sourceIt + 1);
					const unsigned char byte3 = *(sourceIt + 2);
					if (UNLIKELY_ERROR((byte2 & 0xC0) != 0x80 || (byte3 & 0xC0) != 0x80))
					{
						Assert(false, "Invalid modified UTF-8 encoding");
						return {};
					}

					uint16 highSurrogate = (uint16)(((byte & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F));
					if (0xD800 <= highSurrogate && highSurrogate <= 0xDBFF)
					{
						// High surrogate, expect a low surrogate
						const bool isValidSurrogate = (sourceIt + 6 <= endIt) && ((*(sourceIt + 3) & 0xF0) == 0xE0);
						Assert(isValidSurrogate, "Unpaired high surrogate");
						if (UNLIKELY_ERROR(!isValidSurrogate))
						{
							return {};
						}

						const unsigned char byte4 = *(sourceIt + 3);
						const unsigned char byte5 = *(sourceIt + 4);
						const unsigned char byte6 = *(sourceIt + 5);

						const bool isInvalid = (byte4 & 0xF0) != 0xE0 || (byte5 & 0xC0) != 0x80 || (byte6 & 0xC0) != 0x80;
						Assert(!isInvalid, "Invalid modified UTF-8 encoding for low surrogate");
						if (UNLIKELY_ERROR(isInvalid))
						{
							return {};
						}

						const uint16 lowSurrogate = (uint16)(((byte4 & 0x0F) << 12) | ((byte5 & 0x3F) << 6) | (byte6 & 0x3F));
						const bool isInvalidLowSurrogate = lowSurrogate < 0xDC00 || lowSurrogate > 0xDFFF;
						Assert(!isInvalidLowSurrogate, "Invalid low surrogate");
						if (UNLIKELY_ERROR(isInvalidLowSurrogate))
						{
							return {};
						}

						// Decode surrogate pair to code point
						const uint32_t codePoint = 0x10000 + (((highSurrogate - 0xD800) << 10) | (lowSurrogate - 0xDC00));

						// Encode as regular UTF-8
						destination += 0xF0 | ((codePoint >> 18) & 0x07);
						destination += 0x80 | ((codePoint >> 12) & 0x3F);
						destination += 0x80 | ((codePoint >> 6) & 0x3F);
						destination += 0x80 | (codePoint & 0x3F);

						sourceIt += 6;
					}
					else
					{
						// Not a surrogate, copy as-is
						destination += UnicodeString(source.GetSubstring(source.GetIteratorIndex(sourceIt), 3));
						sourceIt += 3;
					}
				}
				else
				{
					// Copy 1-byte and 2-byte sequences as-is
					destination += byte;
					sourceIt += 1 + ((byte & 0xE0) == 0xC0);
				}
			}

			return destination;
		}
		POP_CLANG_WARNINGS
		POP_MSVC_WARNINGS
	}

	template<typename CharType, typename AllocatorType, unsigned char Flags>
	size TString<CharType, AllocatorType, Flags>::Hash::operator()(const ConstView string) const noexcept
	{
		return Math::Hash(std::basic_string_view<CharType>(string.GetData(), string.GetSize()));
	}

	template<typename CharType, typename AllocatorType, uint8 Flags>
	bool TString<CharType, AllocatorType, Flags>::Serialize(const Serialization::Reader serializer)
	{
		if constexpr (TypeTraits::IsSame<CharType, char>)
		{
			const Serialization::Value& __restrict currentElement = serializer.GetValue();
			if (currentElement.IsString())
			{
				*this = TString<char, AllocatorType, Flags>(
					currentElement.GetString(),
					static_cast<typename AllocatorType::SizeType>((typename AllocatorType::SizeType)currentElement.GetStringLength())
				);
				return true;
			}
			return false;
		}
		else if constexpr (TypeTraits::IsSame<CharType, wchar_t>)
		{
			using ReboundAllocator = typename AllocatorType::template Rebind<char>;
			TString<char, ReboundAllocator, Flags> convertedValue;
			const bool success = convertedValue.Serialize(serializer);
			*this = TString<wchar_t, AllocatorType, Flags>(convertedValue.GetView());
			return success;
		}
#if IS_UNICODE_CHAR8_UNIQUE_TYPE
		else if constexpr (TypeTraits::IsSame<CharType, UTF8CharType>)
		{
			using ReboundAllocator = typename AllocatorType::template Rebind<char>;
			TString<char, ReboundAllocator, Flags> convertedValue;
			const bool success = convertedValue.Serialize(serializer);
			*this = TString<UTF8CharType, AllocatorType, Flags>(convertedValue.GetView());
			return success;
		}
#endif
		else if constexpr (TypeTraits::IsSame<CharType, char16_t>)
		{
			using ReboundAllocator = typename AllocatorType::template Rebind<char>;
			TString<char, ReboundAllocator, Flags> convertedValue;
			const bool success = convertedValue.Serialize(serializer);
			*this = TString<char16_t, AllocatorType, Flags>(convertedValue.GetView());
			return success;
		}
		else if constexpr (TypeTraits::IsSame<CharType, char32_t>)
		{
			using ReboundAllocator = typename AllocatorType::template Rebind<char>;
			TString<char, ReboundAllocator, Flags> convertedValue;
			const bool success = convertedValue.Serialize(serializer);
			*this = TString<char32_t, AllocatorType, Flags>(convertedValue.GetView());
			return success;
		}
		else
		{
			return false;
		}
	}

	template<typename CharType, typename AllocatorType, uint8 Flags>
	bool TString<CharType, AllocatorType, Flags>::Serialize(Serialization::Writer serializer) const
	{
		return operator ConstView().Serialize(serializer);
	}

	template<typename CharType, typename AllocatorType, uint8 Flags>
	[[nodiscard]] /* static */ TString<CharType, AllocatorType, Flags> TString<CharType, AllocatorType, Flags>::Escape(const ConstView view
	) noexcept
	{
		if constexpr (SupportResize)
		{
			auto shouldEncode = [](const CharType character)
			{
				const bool isAllowedCharacter = (character >= '0' && character <= '9') || (character >= 'a' && character <= 'z') ||
				                                (character >= 'A' && character <= 'Z') || (character == '-') || (character == '.') ||
				                                (character == '_') || (character == '-');
				return !isAllowedCharacter;
			};

			TString result;
			result.Reserve(view.GetSize() * 3);
			for (const CharType character : view)
			{
				if (shouldEncode(character))
				{
					result += TString().Format("%{:#04X}", (char)character);
				}
				else
				{
					result += character;
				}
			}
			return result;
		}
		else
		{
			Assert(false, "Not supported");
			ExpectUnreachable();
		}
	}

	template<typename CharType, typename AllocatorType, uint8 Flags>
	/* static */ Optional<TString<CharType, AllocatorType, Flags>> TString<CharType, AllocatorType, Flags>::Unescape(const ConstView view
	) noexcept
	{
		if constexpr (SupportResize)
		{
			TString result;
			result.Reserve(view.GetSize());

			for (auto it = view.begin(), end = view.end(); it != end;)
			{
				const CharType character = *it;
				if (character == '%' && (it + 2) < end && isxdigit(*(it + 1)) && isxdigit(*(it + 2)))
				{
					// This is a valid encoded character
					const CharType hexString[3] = {*(it + 1), *(it + 2), 0};
					if constexpr (TypeTraits::IsSame<CharType, char>)
					{
						char* endPointer;
						const CharType hexCharacter = (CharType)(uint8)strtoul(hexString, &endPointer, 16);
						if (hexCharacter < 0x20 || hexCharacter == 0)
						{
							return Invalid;
						}

						result += hexCharacter;
						it += 3;
						continue;
					}
					else if constexpr (TypeTraits::IsSame<CharType, wchar_t>)
					{
						wchar_t* endPointer;
						const CharType hexCharacter = (CharType)(uint8)wcstoul(hexString, &endPointer, 16);
						if (hexCharacter < 0x20 || hexCharacter == 0)
						{
							return Invalid;
						}

						result += hexCharacter;
						it += 3;
						continue;
					}
					else
					{
						Assert(false, "Not supported");
						return {};
					}
				}

				result += character;
				++it;
			}

			return result;
		}
		else
		{
			Assert(false, "Not supported");
			ExpectUnreachable();
		}
	}

	// char
	template struct TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	static_assert(TypeTraits::IsSame<
								String,
								TString<
									char,
									Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>>);

	template struct TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	static_assert(TypeTraits::IsSame<
								FixedCapacityString,
								TString<
									char,
									Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::AllowResize>>);

	template struct TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;
	static_assert(TypeTraits::IsSame<
								FixedSizeString,
								TString<
									char,
									Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::None>>);

	// wchar_t
	template struct TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	static_assert(TypeTraits::IsSame<
								WideString,
								TString<
									wchar_t,
									Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>>);

	template struct TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	static_assert(TypeTraits::IsSame<
								FixedCapacityWideString,
								TString<
									wchar_t,
									Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::AllowResize>>);

	template struct TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;
	static_assert(TypeTraits::IsSame<
								FixedSizeWideString,
								TString<
									wchar_t,
									Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::None>>);

	// UTF8
	template struct TString<
		UTF8CharType,
		Memory::DynamicAllocator<UTF8CharType, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	static_assert(TypeTraits::IsSame<
								UTF8String,
								TString<
									UTF8CharType,
									Memory::DynamicInlineStorageAllocator<UTF8CharType, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>>);

	template struct TString<UTF8CharType, Memory::DynamicAllocator<UTF8CharType, uint32>, Memory::VectorFlags::AllowResize>;
	static_assert(TypeTraits::IsSame<
								FixedCapacityUTF8String,
								TString<
									UTF8CharType,
									Memory::DynamicInlineStorageAllocator<UTF8CharType, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::AllowResize>>);

	template struct TString<UTF8CharType, Memory::DynamicAllocator<UTF8CharType, uint32>, Memory::VectorFlags::None>;
	static_assert(TypeTraits::IsSame<
								FixedSizeUTF8String,
								TString<
									UTF8CharType,
									Memory::DynamicInlineStorageAllocator<UTF8CharType, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::None>>);

	// UTF16
	template struct TString<
		char16_t,
		Memory::DynamicInlineStorageAllocator<char16_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	static_assert(TypeTraits::IsSame<
								UTF16String,
								TString<
									char16_t,
									Memory::DynamicInlineStorageAllocator<char16_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>>);

	template struct TString<
		char16_t,
		Memory::DynamicInlineStorageAllocator<char16_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	static_assert(TypeTraits::IsSame<
								FixedCapacityUTF16String,
								TString<
									char16_t,
									Memory::DynamicInlineStorageAllocator<char16_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::AllowResize>>);

	template struct TString<
		char16_t,
		Memory::DynamicInlineStorageAllocator<char16_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;
	static_assert(TypeTraits::IsSame<
								FixedSizeUTF16String,
								TString<
									char16_t,
									Memory::DynamicInlineStorageAllocator<char16_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::None>>);

	// UTF32
	template struct TString<
		char32_t,
		Memory::DynamicInlineStorageAllocator<char32_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	static_assert(TypeTraits::IsSame<
								UTF32String,
								TString<
									char32_t,
									Memory::DynamicInlineStorageAllocator<char32_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>>);

	template struct TString<
		char32_t,
		Memory::DynamicInlineStorageAllocator<char32_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::AllowResize>;
	static_assert(TypeTraits::IsSame<
								FixedCapacityUTF32String,
								TString<
									char32_t,
									Memory::DynamicInlineStorageAllocator<char32_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::AllowResize>>);

	template struct TString<
		char32_t,
		Memory::DynamicInlineStorageAllocator<char32_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
		Memory::VectorFlags::None>;
	static_assert(TypeTraits::IsSame<
								FixedSizeUTF32String,
								TString<
									char32_t,
									Memory::DynamicInlineStorageAllocator<char32_t, Memory::Internal::SmallStringOptimizationBufferSize, uint32>,
									Memory::VectorFlags::None>>);

	// Path variants
	template struct TString<
		char,
		Memory::DynamicInlineStorageAllocator<char, Memory::Internal::SmallPathOptimizationBufferSize, uint16>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
	template struct TString<
		wchar_t,
		Memory::DynamicInlineStorageAllocator<wchar_t, Memory::Internal::SmallPathOptimizationBufferSize, uint16>,
		Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;

	template bool FlatString<10>::Serialize(const Serialization::Reader serializer);
	template bool FlatString<15>::Serialize(const Serialization::Reader serializer);
	template bool FlatString<40>::Serialize(const Serialization::Reader serializer);
	template bool FlatString<37>::Serialize(const Serialization::Reader serializer);
	template bool FlatString<64>::Serialize(const Serialization::Reader serializer);

	template bool FlatString<10>::Serialize(Serialization::Writer serializer) const;
	template bool FlatString<15>::Serialize(Serialization::Writer serializer) const;
	template bool FlatString<40>::Serialize(Serialization::Writer serializer) const;
	template bool FlatString<37>::Serialize(Serialization::Writer serializer) const;
	template bool FlatString<64>::Serialize(Serialization::Writer serializer) const;
	template bool FlatString<256>::Serialize(Serialization::Writer serializer) const;
	template bool FlatString<300>::Serialize(Serialization::Writer serializer) const;
}
