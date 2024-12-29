#pragma once

#include <Common/Math/NumericLimits.h>
#include <Common/Math/Select.h>
#include <Common/Math/Power.h>
#include <Common/Memory/NativeCharType.h>
#include <Common/Memory/Containers/ArrayView.h>
#include <Common/Memory/Containers/ForwardDeclarations/StringView.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Platform/StaticUnreachable.h>

namespace ngine
{
	namespace Internal
	{
		template<typename T>
		struct CharTraits
		{
			using CharType = char;
			inline static constexpr const auto* Choose(
				const char* const narrow,
				const wchar_t* const wide,
#if IS_UNICODE_CHAR8_UNIQUE_TYPE
				const char8_t* const char8,
#endif
				const char16_t* const char16,
				const char32_t* const char32
			)
			{
				if constexpr (TypeTraits::IsSame<T, char>)
				{
					return narrow;
				}
				else if constexpr (TypeTraits::IsSame<T, wchar_t>)
				{
					return wide;
				}
#if IS_UNICODE_CHAR8_UNIQUE_TYPE
				else if constexpr (TypeTraits::IsSame<T, char8_t>)
				{
					return char8;
				}
#endif
				else if constexpr (TypeTraits::IsSame<T, char16_t>)
				{
					return char16;
				}
				else if constexpr (TypeTraits::IsSame<T, char32_t>)
				{
					return char32;
				}
				else
				{
					static_unreachable("Unimplemented type");
				}
			}
			inline static constexpr auto Choose(
				const char narrow,
				const wchar_t wide,
#if IS_UNICODE_CHAR8_UNIQUE_TYPE
				const char8_t char8,
#endif
				const char16_t char16,
				const char32_t char32
			)
			{
				if constexpr (TypeTraits::IsSame<T, char>)
				{
					return narrow;
				}
				else if constexpr (TypeTraits::IsSame<T, wchar_t>)
				{
					return wide;
				}
#if IS_UNICODE_CHAR8_UNIQUE_TYPE
				else if constexpr (TypeTraits::IsSame<T, char8_t>)
				{
					return char8;
				}
#endif
				else if constexpr (TypeTraits::IsSame<T, char16_t>)
				{
					return char16;
				}
				else if constexpr (TypeTraits::IsSame<T, char32_t>)
				{
					return char32;
				}
				else
				{
					static_unreachable("Unimplemented type");
				}
			}
		};
	}

#if IS_UNICODE_CHAR8_UNIQUE_TYPE
#define MAKE_LITERAL(T, x) Internal::CharTraits<TypeTraits::WithoutConst<T>>::Choose(x, L##x, u8##x, u##x, U##x)
#else
#define MAKE_LITERAL(T, x) Internal::CharTraits<TypeTraits::WithoutConst<T>>::Choose(x, L##x, u##x, U##x)
#endif

	template<typename InternalCharType, typename InternalSizeType>
	struct TRIVIAL_ABI TStringView : public ArrayView<InternalCharType, InternalSizeType>
	{
		struct Hash
		{
			using is_transparent = void;

			constexpr size operator()(TStringView view) const
			{
#if PLATFORM_64BIT
				using CharacterType = TypeTraits::WithoutConst<InternalCharType>;
				size hash = 14695981039346656037ull;
				for (CharacterType character : view)
				{
					constexpr auto iterationCount = sizeof(CharacterType) / sizeof(char);
					for (uint8 i = 0; i < iterationCount; ++i)
					{
						CharacterType byte = (character & 0xFF);
						character <<= sizeof(CharacterType);

						hash ^= static_cast<size>(byte);
						hash *= 1099511628211ull;
					}
				}
#else
				const size m = 0x5bd1e995;
				size hash = m ^ view.GetSize();
				const int r = 24;

				const InternalCharType* data = view.GetData();
				uint32 len = view.GetSize();

				while (len >= 4)
				{
					uint32 k = static_cast<uint32>(data[0]);
					k |= data[1] << 8;
					k |= data[2] << 16;
					k |= data[3] << 24;

					k *= m;
					k ^= k >> r;
					k *= m;

					hash *= m;
					hash ^= k;

					data += 4;
					len -= 4;
				}

				// Handle the last few bytes of the input array

				switch (len)
				{
					case 3:
						hash ^= data[2] << 16;
						[[fallthrough]];
					case 2:
						hash ^= data[1] << 8;
						[[fallthrough]];
					case 1:
						hash ^= data[0];
						hash *= m;
				};

				// Do a few final mixes of the hash to ensure the last few
				// bytes are well-incorporated.

				hash ^= hash >> 13;
				hash *= m;
				hash ^= hash >> 15;
#endif
				return hash;
			}
		};
		using ConstView = TStringView<const InternalCharType, InternalSizeType>;

		[[nodiscard]] PURE_STATICS static constexpr FORCE_INLINE InternalCharType MakeLower(const InternalCharType character) noexcept
		{
			return ((character >= MAKE_LITERAL(InternalCharType, 'A')) & (character <= MAKE_LITERAL(InternalCharType, 'Z')))
			         ? character - MAKE_LITERAL(InternalCharType, 'A') + MAKE_LITERAL(InternalCharType, 'a')
			         : character;
		}

		[[nodiscard]] PURE_STATICS static constexpr FORCE_INLINE InternalCharType MakeUpper(const InternalCharType character) noexcept
		{
			return ((character >= MAKE_LITERAL(InternalCharType, 'a')) & (character <= MAKE_LITERAL(InternalCharType, 'z')))
			         ? character - MAKE_LITERAL(InternalCharType, 'a') + MAKE_LITERAL(InternalCharType, 'A')
			         : character;
		}

		using BaseType = ArrayView<InternalCharType, InternalSizeType>;
		using ConstArrayView = ArrayView<const InternalCharType, InternalSizeType>;

		using SizeType = typename BaseType::SizeType;
		using CharType = InternalCharType;
		inline static constexpr SizeType InvalidPosition = Math::NumericLimits<SizeType>::Max;

		constexpr TStringView() = default;
		FORCE_INLINE constexpr explicit TStringView(const BaseType& other)
			: BaseType(other)
		{
		}
		FORCE_INLINE constexpr explicit TStringView(BaseType&& other)
			: BaseType(Forward<BaseType>(other))
		{
		}
		FORCE_INLINE constexpr TStringView(CharType* const pBegin, CharType* const pEnd) noexcept
			: BaseType(pBegin, pEnd)
		{
		}
		FORCE_INLINE constexpr TStringView(CharType* pData, const SizeType size) noexcept
			: TStringView(pData, pData + size)
		{
		}
		template<size Size>
		FORCE_INLINE constexpr TStringView& operator=(const CharType (&data)[Size] LIFETIME_BOUND) noexcept
		{
			const SizeType size = Size - (data[Size - 1] == '\0');
			BaseType::operator=(BaseType{data, data + size});
			return *this;
		}
		template<size Size>
		FORCE_INLINE constexpr TStringView(CharType (&data)[Size] LIFETIME_BOUND) noexcept
			: TStringView(data, Size - (data[Size - 1] == '\0'))
		{
		}
		template<typename OtherSizeType>
		FORCE_INLINE constexpr TStringView(const TStringView<CharType, OtherSizeType> other) noexcept
			: BaseType(other)
		{
		}
		FORCE_INLINE constexpr TStringView(const TStringView& other)
			: BaseType(other)
		{
		}
		FORCE_INLINE constexpr TStringView& operator=(const TStringView& other) noexcept
		{
			BaseType::operator=(static_cast<const BaseType&>(other));
			return *this;
		}
		template<
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename ElementType = CharType,
			typename = EnableIf<TypeTraits::IsConst<ElementType>>>
		FORCE_INLINE constexpr TStringView(const TStringView<typename TypeTraits::WithoutConst<ElementType>, OtherSizeType>& otherView) noexcept
			: BaseType(otherView.GetData(), (SizeType)otherView.GetSize())
		{
		}
		template<
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename ElementType = CharType,
			typename = EnableIf<TypeTraits::IsConst<ElementType>>>
		FORCE_INLINE constexpr TStringView& operator=(const TStringView<typename TypeTraits::WithoutConst<ElementType>, OtherSizeType> otherView
		) noexcept
		{
			BaseType::operator=(BaseType{otherView.GetData(), otherView.GetSize()});
			return *this;
		}
		constexpr TStringView(TStringView&&) = default;
		constexpr TStringView& operator=(TStringView&&) = default;
		~TStringView() = default;

		using BaseType::GetData;
		using BaseType::GetSize;
		using BaseType::IsEmpty;

		using BaseType::begin;
		using BaseType::end;

		using BaseType::operator[];
		using BaseType::GetIteratorIndex;

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType FindFirstOf(const CharType value, const SizeType offset = 0) const noexcept
		{
			const auto it = GetSubstringFrom(offset).ConstView::BaseType::Find(value);
			if (it.IsValid())
			{
				return static_cast<SizeType>(it.Get() - begin());
			}
			return InvalidPosition;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType FindLastOf(const CharType value, const SizeType offset = 0) const noexcept
		{
			const auto it = GetSubstringUpTo(GetSize() - offset).ConstView::BaseType::FindLastOf(value);
			if (it.IsValid())
			{
				return static_cast<SizeType>(it.Get() - begin());
			}
			return InvalidPosition;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView
		FindFirstRange(const ConstView value, const SizeType offset = 0) const noexcept
		{
			const ConstView searchedString = GetSubstringFrom(offset);
			return ConstView{searchedString.ConstView::BaseType::FindFirstRange((ConstArrayView)value)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView
		FindLastRange(const ConstView value, const SizeType offset = 0) const noexcept
		{
			const ConstView searchedString = GetSubstringUpTo(GetSize() - offset);
			return ConstView{searchedString.ConstView::BaseType::FindLastRange((ConstArrayView)value)};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr TStringView GetSubstring(const SizeType offset, const SizeType count) noexcept
		{
			return TStringView(BaseType::GetSubView(offset, count));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView GetSubstring(const SizeType offset, const SizeType count) const noexcept
		{
			return TStringView(BaseType::GetSubView(offset, count));
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr TStringView GetSubstringUpTo(const SizeType offset) noexcept
		{
			return TStringView(BaseType::GetSubViewUpTo(BaseType::begin() + offset));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView GetSubstringUpTo(const SizeType offset) const noexcept
		{
			return TStringView(BaseType::GetSubViewUpTo(BaseType::begin() + offset));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr TStringView GetSubstringFrom(const SizeType offset) noexcept
		{
			return TStringView(BaseType::GetSubViewFrom(BaseType::begin() + offset));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView GetSubstringFrom(const SizeType offset) const noexcept
		{
			return TStringView(BaseType::GetSubViewFrom(BaseType::begin() + offset));
		}

		[[nodiscard]] PURE_STATICS double ToDouble() const noexcept
		{
			if (IsEmpty())
			{
				return 0.0;
			}

			int8 sign = 1;

			ConstView it = *this;

			// Take care of +/- sign
			if (it[0] == '-')
			{
				++it;
				sign = -1;
			}
			else if (it[0] == '+')
			{
				++it;
			}

			auto parseExponent = [&it]() -> double
			{
				Assert(it.HasElements());
				int8 expSign = 1;
				if (it[0] == '-')
				{
					expSign = -1;
					++it;
				}
				else if (it[0] == '+')
				{
					++it;
				}

				uint8 e = 0;
				while (it.HasElements() && ((it[0] >= '0') & (it[0] <= '9')))
				{
					e = (uint8)(e * (uint8)10) + (uint8)(it[0] - '0');
					++it;
				}

				return (double)Math::Power10(e) * expSign;
			};

			auto parseFraction = [&it]() -> double
			{
				Assert(it.HasElements());
				double fractionExpo = 0.1;
				double fractionPart = 0.0;

				do
				{
					if ((it[0] >= '0') & (it[0] <= '9'))
					{
						fractionPart += fractionExpo * (it[0] - '0');
						fractionExpo *= 0.1;
					}
					else
					{
						return fractionPart;
					}
					++it;
				} while (it.HasElements());

				return fractionPart;
			};

			double integerPart = 0.0;

			while (it.HasElements())
			{
				if ((it[0] >= '0') & (it[0] <= '9'))
				{
					integerPart = integerPart * 10 + (it[0] - '0');
					++it;
					if (!it.HasElements())
					{
						return double(sign * integerPart);
					}
				}
				else if (it[0] == '.')
				{
					++it;
					if (!it.HasElements())
					{
						return double(sign * integerPart);
					}

					const double fractionPart = parseFraction();
					if (it.HasElements() && it[0] == 'e')
					{
						++it;
						const double exponentPart = parseExponent();
						return double(sign * (integerPart + fractionPart) * exponentPart);
					}

					return float(sign * (integerPart + fractionPart));
				}
				else if (it[0] == 'e')
				{
					++it;
					if (!it.HasElements())
					{
						return double(sign * integerPart);
					}

					const double exponentPart = parseExponent();
					return double(sign * integerPart * exponentPart);
				}
				else
				{
					return double(sign * integerPart);
				}
			}

			return double(sign * integerPart);
		}

		struct ToDoubleResult
		{
			bool success{false};
			double value;
		};

		[[nodiscard]] PURE_STATICS ToDoubleResult TryToDouble() const noexcept
		{
			if (IsEmpty())
			{
				return ToDoubleResult{false};
			}

			const InternalCharType* it = begin();
			const InternalCharType* const endIt = end();

			// Allow - or + at the start
			it += (*it == MAKE_LITERAL(InternalCharType, '-')) || *it == MAKE_LITERAL(InternalCharType, '+');

			for (; it != endIt; it++)
			{
				if ((*it >= '0') & (*it <= '9'))
					;                  // Numeric, valid
				else if (*it == '.') // Detect first period
				{
					++it;
					if (it == endIt || ((*it < MAKE_LITERAL(InternalCharType, '0')) | (*it > MAKE_LITERAL(InternalCharType, '9'))))
					{
						// No decimal specified after separator
						return ToDoubleResult{false};
					}

					for (; it != endIt;)
					{
						if (*it == 'e')
						{
							break;
						}
						else if ((*it < MAKE_LITERAL(InternalCharType, '0')) | (*it > MAKE_LITERAL(InternalCharType, '9')))
						{
							// Contained non-numeric character
							return ToDoubleResult{false};
						}
						++it;
					}

					if (it != endIt && *it == 'e')
					{
						++it;
						if (it == endIt)
						{
							return ToDoubleResult{false};
						}

						for (; it != endIt; ++it)
						{
							if ((*it < MAKE_LITERAL(InternalCharType, '0')) | (*it > MAKE_LITERAL(InternalCharType, '9')))
							{
								// Contained non-numeric character
								return ToDoubleResult{false};
							}
						}
					}

					Assert(it == endIt);

					return ToDoubleResult{true, ToDouble()};
				}
				else if (*it == 'e') // Detect exponent
				{
					++it;
					if (it == endIt || ((*it < MAKE_LITERAL(InternalCharType, '0')) | (*it > MAKE_LITERAL(InternalCharType, '9'))))
					{
						// No decimal specified after separator
						return ToDoubleResult{false};
					}

					for (; it != endIt; ++it)
					{
						if ((*it < MAKE_LITERAL(InternalCharType, '0')) | (*it > MAKE_LITERAL(InternalCharType, '9')))
						{
							// Contained non-numeric character
							return ToDoubleResult{false};
						}
					}

					return ToDoubleResult{true, ToDouble()};
				}
				else
				{
					return ToDoubleResult{false};
				}
			}

			return ToDoubleResult{true, ToDouble()};
		}

		[[nodiscard]] PURE_STATICS float ToFloat() const noexcept
		{
			return (float)ToDouble();
		}

		struct ToFloatResult
		{
			bool success{false};
			float value;
		};

		[[nodiscard]] PURE_STATICS ToFloatResult TryToFloat() const noexcept
		{
			const ToDoubleResult result = TryToDouble();
			return ToFloatResult{result.success, (float)result.value};
		}

		template<typename IntegerType>
		[[nodiscard]] PURE_STATICS IntegerType ToIntegral() const noexcept
		{
			const InternalCharType* it = begin();
			const InternalCharType* const endIt = end();

			IntegerType result = 0;

			const bool shouldNegate = *it == MAKE_LITERAL(InternalCharType, '-');
			it += shouldNegate;

			for (; it != endIt && (*it >= MAKE_LITERAL(InternalCharType, '0')) & (*it <= MAKE_LITERAL(InternalCharType, '9')); it++)
			{
				result *= (IntegerType)10;
				result += *it - MAKE_LITERAL(InternalCharType, '0');
			}
			if constexpr (!Math::NumericLimits<TypeTraits::WithoutConst<IntegerType>>::IsUnsigned)
			{
				result *= shouldNegate ? (IntegerType)-1 : (IntegerType)1;
			}
			else
			{
				Assert(!shouldNegate);
			}

			return result;
		}

		template<typename IntegerType>
		struct ToIntegralResult
		{
			bool success{false};
			IntegerType value;
		};

		template<typename IntegerType>
		[[nodiscard]] PURE_STATICS ToIntegralResult<IntegerType> TryToIntegral() const noexcept
		{
			if (IsEmpty())
			{
				return {false};
			}

			const InternalCharType* it = begin();
			const InternalCharType* const endIt = end();

			const bool hasNegation = *it == MAKE_LITERAL(InternalCharType, '-');
			it += hasNegation;
			if constexpr (Math::NumericLimits<TypeTraits::WithoutConst<InternalCharType>>::IsUnsigned)
			{
				if (hasNegation)
				{
					return {false};
				}
			}

			for (; it != endIt; it++)
			{
				if ((*it < MAKE_LITERAL(InternalCharType, '0')) | (*it > MAKE_LITERAL(InternalCharType, '9')))
				{
					// Contained non-numeric character
					return {false};
				}
			}

			return {true, ToIntegral<IntegerType>()};
		}

		[[nodiscard]] PURE_STATICS bool ToBool() const noexcept
		{
			if (GetSize() == 1)
			{
				if (this->operator[](0) == MAKE_LITERAL(InternalCharType, '0'))
				{
					return false;
				}
				else if (this->operator[](0) == MAKE_LITERAL(InternalCharType, '1'))
				{
					return true;
				}
			}
			else if (GetSize() == 4)
			{
				if (MakeUpper(this->operator[](0)) == MAKE_LITERAL(InternalCharType, 'T') &&
					MakeUpper(this->operator[](1)) == MAKE_LITERAL(InternalCharType, 'R') &&
					MakeUpper(this->operator[](2)) == MAKE_LITERAL(InternalCharType, 'U') &&
					MakeUpper(this->operator[](3)) == MAKE_LITERAL(InternalCharType, 'E'))
				{
					return true;
				}
			}
			else if (GetSize() == 5)
			{
				if (MakeUpper(this->operator[](0)) == MAKE_LITERAL(InternalCharType, 'F') &&
					MakeUpper(this->operator[](1)) == MAKE_LITERAL(InternalCharType, 'A') &&
					MakeUpper(this->operator[](2)) == MAKE_LITERAL(InternalCharType, 'L') &&
					MakeUpper(this->operator[](3)) == MAKE_LITERAL(InternalCharType, 'S') &&
					MakeUpper(this->operator[](4)) == MAKE_LITERAL(InternalCharType, 'E'))
				{
					return false;
				}
			}
			return false;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const ConstView other) const noexcept
		{
			return EqualsCaseSensitive(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator!=(const ConstView other) const noexcept
		{
			return !(*this == other);
		}

		template<size Size>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool operator==(const CharType (&data)[Size]) const noexcept
		{
			return operator==(ConstView(data));
		}

		template<typename OtherSizeType>
		[[nodiscard]] PURE_STATICS constexpr bool EqualsCaseInsensitive(const TStringView<const CharType, OtherSizeType> other) const noexcept
		{
			if (GetSize() != other.GetSize())
			{
				return false;
			}

			for (SizeType i = 0; i < GetSize(); ++i)
			{
				if (MakeUpper(this->operator[](i)) != MakeUpper(other[i]))
				{
					return false;
				}
			}

			return true;
		}

		template<typename OtherSizeType>
		[[nodiscard]] PURE_STATICS constexpr bool EqualsCaseSensitive(const TStringView<const CharType, OtherSizeType> other) const noexcept
		{
			return BaseType::operator==((typename TStringView<const CharType, OtherSizeType>::BaseType)other);
		}

		template<typename OtherSizeType>
		[[nodiscard]] PURE_STATICS constexpr bool GreaterThanCaseSensitive(const TStringView<const CharType, OtherSizeType> other
		) const noexcept
		{
			for (SizeType i = 0, size = Math::Min(GetSize(), other.GetSize()); i < size; ++i)
			{
				if (this->operator[](i) > other[i])
				{
					return true;
				}
				else if (this->operator[](i) < other[i])
				{
					return false;
				}
			}

			return false;
		}

		template<typename OtherSizeType>
		[[nodiscard]] PURE_STATICS constexpr bool GreaterThanCaseInsensitive(const TStringView<const CharType, OtherSizeType> other
		) const noexcept
		{
			for (SizeType i = 0, size = Math::Min(GetSize(), other.GetSize()); i < size; ++i)
			{
				if (MakeUpper(this->operator[](i)) > MakeUpper(other[i]))
				{
					return true;
				}
				else if (MakeUpper(this->operator[](i)) < MakeUpper(other[i]))
				{
					return false;
				}
			}

			return false;
		}

		template<typename OtherSizeType>
		[[nodiscard]] PURE_STATICS constexpr bool GreaterThanOrEqualCaseSensitive(const TStringView<const CharType, OtherSizeType> other
		) const noexcept
		{
			return GreaterThanCaseSensitive(other) || EqualsCaseSensitive(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator>(const ConstView other) const noexcept
		{
			return GreaterThanCaseSensitive(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator>=(const ConstView other) const noexcept
		{
			return GreaterThanOrEqualCaseSensitive(other);
		}

		template<typename OtherSizeType>
		[[nodiscard]] PURE_STATICS constexpr bool LessThanCaseSensitive(const TStringView<const CharType, OtherSizeType> other) const noexcept
		{
			for (SizeType i = 0, size = Math::Min(GetSize(), other.GetSize()); i < size; ++i)
			{
				if (this->operator[](i) < other[i])
				{
					return true;
				}
				else if (this->operator[](i) > other[i])
				{
					return false;
				}
			}

			return false;
		}

		template<typename OtherSizeType>
		[[nodiscard]] PURE_STATICS constexpr bool LessThanCaseInsensitive(const TStringView<const CharType, OtherSizeType> other) const noexcept
		{
			for (SizeType i = 0, size = Math::Min(GetSize(), other.GetSize()); i < size; ++i)
			{
				if (MakeUpper(this->operator[](i)) < MakeUpper(other[i]))
				{
					return true;
				}
				else if (MakeUpper(this->operator[](i)) > MakeUpper(other[i]))
				{
					return false;
				}
			}

			return false;
		}

		template<typename OtherSizeType>
		[[nodiscard]] PURE_STATICS constexpr bool LessThanOrEqualCaseSensitive(const TStringView<const CharType, OtherSizeType> other
		) const noexcept
		{
			return LessThanCaseSensitive(other) || EqualsCaseSensitive(other);
		}

		[[nodiscard]] PURE_STATICS bool operator<(const ConstView other) const noexcept
		{
			return LessThanCaseSensitive(other);
		}

		[[nodiscard]] PURE_STATICS bool operator<=(const ConstView other) const noexcept
		{
			return LessThanOrEqualCaseSensitive(other);
		}

		[[nodiscard]] PURE_STATICS bool StartsWith(const ConstView other) const noexcept
		{
			return GetSubstring(0, other.GetSize()) == other;
		}
		[[nodiscard]] PURE_STATICS bool EndsWith(const ConstView other) const noexcept
		{
			if (GetSize() < other.GetSize())
			{
				return false;
			}

			return GetSubstring(GetSize() - other.GetSize(), other.GetSize()) == other;
		}

		[[nodiscard]] PURE_STATICS bool Contains(const ConstView other) const noexcept
		{
			return BaseType::ContainsRange((ConstArrayView)other);
		}

		[[nodiscard]] PURE_STATICS bool ContainsCaseInsensitive(const ConstView other) const noexcept
		{
			if (GetSize() < other.GetSize())
			{
				return false;
			}

			SizeType index = 0;
			for (const CharType element : *this)
			{
				if (MakeUpper(element) == MakeUpper(other[index]))
				{
					if (++index == other.GetSize())
					{
						return true;
					}
				}
				else
				{
					index = 0;
				}
			}

			return false;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr TStringView operator+(SizeType offset) const noexcept
		{
			return TStringView(BaseType::operator+(offset));
		}
		FORCE_INLINE constexpr TStringView& operator+=(SizeType offset) noexcept
		{
			return static_cast<TStringView&>(BaseType::operator+=(offset));
		}
		FORCE_INLINE constexpr TStringView& operator++() noexcept
		{
			return static_cast<TStringView&>(BaseType::operator++());
		}
		FORCE_INLINE constexpr TStringView& operator++(int) noexcept
		{
			return static_cast<TStringView&>(BaseType::operator++(0));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr TStringView operator-(const SizeType offset) const noexcept
		{
			return TStringView(BaseType::operator-(offset));
		}
		FORCE_INLINE constexpr TStringView& operator-=(const SizeType offset) noexcept
		{
			return static_cast<TStringView&>(BaseType::operator-=(offset));
		}
		FORCE_INLINE constexpr TStringView& operator--() noexcept
		{
			return static_cast<TStringView&>(BaseType::operator--());
		}
		FORCE_INLINE constexpr TStringView& operator--(int) noexcept
		{
			return static_cast<TStringView&>(BaseType::operator--(0));
		}

		[[nodiscard]] PURE_STATICS bool Contains(const CharType other) const noexcept
		{
			return BaseType::Contains(other);
		}

		[[nodiscard]] PURE_STATICS bool ContainsCaseInsensitive(const CharType other) const noexcept
		{
			for (const CharType element : *this)
			{
				if (MakeUpper(element) == MakeUpper(other))
				{
					return true;
				}
			}

			return false;
		}

		template<typename ElementType = CharType, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		void ReplaceCharacterOccurrences(const CharType character, const CharType replacement) noexcept
		{
			for (CharType *it = begin(), *itEnd = end(); it != itEnd; ++it)
			{
				if (*it == character)
				{
					*it = replacement;
				}
			}
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	};

	extern template struct TStringView<const char, uint32>;
	extern template struct TStringView<const wchar_t, uint32>;
	extern template struct TStringView<char, uint32>;
	extern template struct TStringView<wchar_t, uint32>;
	extern template struct TStringView<const char, uint16>;
	extern template struct TStringView<const wchar_t, uint16>;
	extern template struct TStringView<char, uint16>;
	extern template struct TStringView<wchar_t, uint16>;
}
