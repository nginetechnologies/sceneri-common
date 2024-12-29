#pragma once

#include "ZeroTerminatedStringView.h"

#include <Common/Memory/Containers/VectorBase.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/Math/Max.h>
#include <Common/Guid.h>
#include <Common/Platform/StaticUnreachable.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include "ForwardDeclarations/StringBase.h"
#include "ForwardDeclarations/String.h"

#include <cctype>

namespace ngine
{
	enum class ConvertType
	{
		Convert
	};

	namespace FormatInternal
	{
		template<typename StringType, typename... Args>
		void FormatString(StringType& string, const TStringView<const char> format, Args&&... args);
	}

	namespace Internal
	{
		bool UTF8ToUTF16(const TStringView<char16_t> destination, const TStringView<const UTF8CharType> source) noexcept;
		bool UTF8ToUTF32(const TStringView<char32_t> destination, const TStringView<const UTF8CharType> source) noexcept;
		bool UTF16ToUTF8(const TStringView<UTF8CharType> destination, const TStringView<const char16_t> source) noexcept;
		bool UTF16ToUTF32(const TStringView<char32_t> destination, const TStringView<const char16_t> source) noexcept;
		bool UTF32ToUTF8(const TStringView<UTF8CharType> destination, const TStringView<const char32_t> source) noexcept;
		bool UTF32ToUTF16(const TStringView<char16_t> destination, const TStringView<const char32_t> source) noexcept;
		UnicodeString FromModifiedUTF8(const TStringView<const char> source) noexcept;
	}

	template<typename CharType_, typename _AllocatorType, unsigned char Flags>
	struct TString : protected TVector<CharType_, _AllocatorType, Flags>
	{
		inline static constexpr Guid TypeGuid = "{2B5B8FD6-2BE0-450A-A629-518ED389B8C5}"_guid;

		using CharType = CharType_;
		using ValueType = CharType;
		using AllocatorType = _AllocatorType;
		using BaseType = TVector<CharType, AllocatorType, Flags>;
		using iterator = typename BaseType::iterator;
		using const_iterator = typename BaseType::const_iterator;
		using SizeType = typename BaseType::SizeType;
		using View = TStringView<CharType, SizeType>;
		using ConstView = TStringView<const CharType, SizeType>;
		using ZeroTerminatedView = TZeroTerminatedStringView<CharType, SizeType>;
		using ConstZeroTerminatedView = TZeroTerminatedStringView<const CharType, SizeType>;

		inline static constexpr uint64 InvalidPosition = Math::NumericLimits<size>::Max;
		inline static bool IsWritable = !TypeTraits::IsConst<CharType>;
		inline static constexpr bool SupportResize = BaseType::SupportResize;
		inline static constexpr bool SupportReallocate = BaseType::SupportReallocate;

		TString() = default;
		TString(const Memory::ReserveType type, const SizeType capacity) noexcept
			: BaseType(type, capacity + 1u)
		{
			BaseType::m_size = 1u;
			SetNullTerminator();
		}
		TString(const Memory::ConstructWithSizeType type, const Memory::UninitializedType, const SizeType size) noexcept
			: BaseType(type, Memory::Uninitialized, size + 1u)
		{
			SetNullTerminator();
		}
		template<typename ViewSizeType>
		explicit TString(const TStringView<const CharType, ViewSizeType> stringView) noexcept
			: BaseType(Memory::ConstructWithSize, Memory::Uninitialized, SizeType(stringView.GetSize() + 1u))
		{
			BaseType::m_allocator.GetView().CopyFrom(stringView);

			SetNullTerminator();
		}
		TString(const CharType* pSourceData, const SizeType size) noexcept
			: TString(ConstView(pSourceData, size))
		{
		}
		template<SizeType Size>
		TString(const CharType (&data)[Size]) noexcept
			: TString(data, Size - (data[Size - 1] == '\0'))
		{
		}

		TString(const ConstZeroTerminatedView zeroTerminatedStringView) noexcept
			: TString((ConstView)zeroTerminatedStringView)
		{
		}
		TString(const TString& other) noexcept
			: TString(other.GetView())
		{
		}
		template<typename OtherAllocatorType, uint8 OtherFlags, typename OtherCharType = CharType_>
		TString(const TString<OtherCharType, OtherAllocatorType, OtherFlags>& other) noexcept
			: TString(other.GetView())
		{
		}

		// UTF8 to UTF16 or UTF32
		template<
			typename CharType__ = CharType,
			typename = EnableIf<
				(TypeTraits::IsSame<CharType__, wchar_t> && (sizeof(wchar_t) == sizeof(char16_t) || sizeof(wchar_t) == sizeof(char32_t))) ||
				TypeTraits::IsSame<CharType__, char16_t> || TypeTraits::IsSame<CharType__, char32_t>>>
		explicit TString(const TStringView<const UTF8CharType, SizeType> charString) noexcept
			: TString(Memory::ConstructWithSize, Memory::Uninitialized, charString.GetSize())
		{
			if constexpr (TypeTraits::IsSame<CharType__, char16_t> || (TypeTraits::IsSame<CharType__, wchar_t> && sizeof(wchar_t) == sizeof(char16_t)))
			{
				Internal::UTF8ToUTF16(TStringView<char16_t>{reinterpret_cast<char16_t*>(GetData()), GetSize()}, charString);
			}
			else if constexpr (TypeTraits::IsSame<CharType__, char32_t> || (TypeTraits::IsSame<CharType__, wchar_t> && sizeof(wchar_t) == sizeof(char32_t)))
			{
				Internal::UTF8ToUTF32(TStringView<char32_t>{reinterpret_cast<char32_t*>(GetData()), GetSize()}, charString);
			}
		}
		// wchar_t to UTF8
		template<
			typename CharType__ = CharType,
			typename = EnableIf<(TypeTraits::IsSame<CharType__, UTF8CharType> || TypeTraits::IsSame<CharType__, char16_t>)&&(
				sizeof(wchar_t) == sizeof(char16_t) || sizeof(wchar_t) == sizeof(char32_t)
			)>>
		explicit TString(const TStringView<const wchar_t, SizeType> charString) noexcept
			: TString(Memory::ConstructWithSize, Memory::Uninitialized, charString.GetSize())
		{
			if constexpr (TypeTraits::IsSame<CharType__, UTF8CharType>)
			{
				if constexpr (sizeof(wchar_t) == sizeof(char16_t))
				{
					Internal::UTF16ToUTF8(
						GetView(),
						TStringView<const char16_t>{reinterpret_cast<const char16_t*>(charString.GetData()), charString.GetSize()}
					);
				}
				else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
				{
					Internal::UTF32ToUTF8(
						GetView(),
						TStringView<const char32_t>{reinterpret_cast<const char32_t*>(charString.GetData()), charString.GetSize()}
					);
				}
			}
			else if constexpr (TypeTraits::IsSame<CharType__, char16_t> && PLATFORM_WINDOWS)
			{
				// Allow explicit conversion between wchar_t and char16_t on windows
				Memory::CopyNonOverlappingElements(GetData(), reinterpret_cast<const char16_t*>(charString.GetData()), charString.GetSize());
				SetNullTerminator();
			}
		}
		// UTF16 to UTF8 or UTF32
		template<
			typename CharType__ = CharType,
			typename = EnableIf<
				TypeTraits::IsSame<CharType__, UTF8CharType> || (TypeTraits::IsSame<CharType__, wchar_t> && sizeof(wchar_t) == sizeof(char16_t)) ||
				TypeTraits::IsSame<CharType__, char32_t>>>
		explicit TString(const TStringView<const char16_t, SizeType> charString) noexcept
			: TString(Memory::ConstructWithSize, Memory::Uninitialized, charString.GetSize())
		{
			if constexpr (TypeTraits::IsSame<CharType__, UTF8CharType>)
			{
				Internal::UTF16ToUTF8(GetView(), charString);
			}
			else if constexpr (TypeTraits::IsSame<CharType__, wchar_t> && PLATFORM_WINDOWS)
			{
				// Allow explicit conversion between wchar_t and char16_t on windows
				Memory::CopyNonOverlappingElements(GetData(), reinterpret_cast<const wchar_t*>(charString.GetData()), charString.GetSize());
				SetNullTerminator();
			}
			else if constexpr (TypeTraits::IsSame<CharType__, char32_t>)
			{
				Internal::UTF16ToUTF32(GetView(), charString);
			}
		}
		// UTF32 to UTF8
		template<
			typename CharType__ = CharType,
			typename = EnableIf<TypeTraits::IsSame<CharType__, UTF8CharType> || TypeTraits::IsSame<CharType__, char16_t>>>
		explicit TString(const TStringView<const char32_t, SizeType> charString) noexcept
			: TString(Memory::ConstructWithSize, Memory::Uninitialized, charString.GetSize())
		{
			if constexpr (TypeTraits::IsSame<CharType__, UTF8CharType>)
			{
				Internal::UTF32ToUTF8(GetView(), charString);
			}
			else if constexpr (TypeTraits::IsSame<CharType__, char16_t>)
			{
				Internal::UTF32ToUTF16(GetView(), charString);
			}
		}

		// UTF16 to UTF8
#if IS_UTF8_CHAR_UNIQUE_TYPE
		template<
			typename TargetCharType = CharType,
			typename = EnableIf<TypeTraits::IsSame<TargetCharType, char> && !TypeTraits::IsSame<UTF8CharType, char>>>
		explicit TString(const TStringView<const UTF8CharType, SizeType> charString) noexcept
			: TString(Memory::ConstructWithSize, Memory::Uninitialized, charString.GetSize())
		{
			Internal::UTF16ToUTF8(GetView(), charString);
		}
#endif
		TString(TString&& other) noexcept
			: BaseType(static_cast<BaseType&&>(other))
		{
		}
		TString& operator=(TString&& other) noexcept
		{
			return static_cast<TString&>(BaseType::operator=(static_cast<BaseType&&>(other)));
		}
		TString& operator=(const TString& other) noexcept
		{
			return static_cast<TString&>(BaseType::operator=(static_cast<const BaseType&>(other)));
		}
		template<typename OtherAllocatorType, uint8 OtherFlags, typename OtherCharType = CharType_>
		TString& operator=(const TString<OtherCharType, OtherAllocatorType, OtherFlags>& other) noexcept
		{
			Reserve(other.GetSize() + 1);
			BaseType::operator=(other.GetView());

			BaseType::m_size++;
			SetNullTerminator();
			return *this;
		}
		template<
			typename ElementType = CharType,
			bool AllowResize = SupportResize,
			typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType> && AllowResize)>>
		TString& operator=(const ConstView view) noexcept
		{
			Reserve(view.GetSize() + 1);
			BaseType::operator=(view);

			BaseType::m_size++;
			SetNullTerminator();
			return *this;
		}
		~TString() = default;

		struct Hash
		{
			using is_transparent = void;

			size operator()(const ConstView string) const noexcept;
		};

		[[nodiscard]] FORCE_INLINE PURE_STATICS operator View() noexcept LIFETIME_BOUND
		{
			return {GetData(), GetSize()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS operator ConstView() const noexcept LIFETIME_BOUND
		{
			return {GetData(), GetSize()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS View GetView() noexcept LIFETIME_BOUND
		{
			return {GetData(), GetSize()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS ConstView GetView() const noexcept LIFETIME_BOUND
		{
			return {GetData(), GetSize()};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS operator ZeroTerminatedView() noexcept LIFETIME_BOUND
		{
			return {GetData(), static_cast<SizeType>(GetSize() + static_cast<SizeType>(1))};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS ZeroTerminatedView GetZeroTerminated() noexcept LIFETIME_BOUND
		{
			return *this;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS operator ConstZeroTerminatedView() const noexcept LIFETIME_BOUND
		{
			return {GetData(), static_cast<SizeType>(GetSize() + static_cast<SizeType>(1))};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS ConstZeroTerminatedView GetZeroTerminated() const noexcept LIFETIME_BOUND
		{
			return *this;
		}

		using BaseType::begin;
		[[nodiscard]] FORCE_INLINE PURE_STATICS typename BaseType::IteratorType end() noexcept LIFETIME_BOUND
		{
			return BaseType::end() - 1u;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS typename BaseType::ConstIteratorType end() const noexcept LIFETIME_BOUND
		{
			return BaseType::end() - 1u;
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void Clear()
		{
			BaseType::Clear();

			BaseType::m_size = BaseType::GetCapacity() > 0;
			if (BaseType::m_size == 1)
			{
				SetNullTerminator();
			}
		}

		using BaseType::Contains;

		template<
			bool AllowResize = SupportResize,
			typename ElementType = ValueType,
			bool CanDefaultConstruct = TypeTraits::IsDefaultConstructible<ElementType>>
		EnableIf<AllowResize && CanDefaultConstruct>
		Resize(const SizeType size, const Memory::DefaultConstructType = Memory::DefaultConstruct) noexcept
		{
			BaseType::Resize(size + 1, Memory::DefaultConstruct);
		}

		template<bool AllowResize = SupportResize>
		EnableIf<AllowResize> Resize(const SizeType size, const Memory::UninitializedType) noexcept
		{
			BaseType::Resize(size + 1, Memory::Uninitialized);
		}

		template<bool AllowResize = SupportResize>
		EnableIf<AllowResize> Resize(const SizeType size, const Memory::ZeroedType) noexcept
		{
			BaseType::Resize(size + 1, Memory::Zeroed);
		}

		void Reserve(const SizeType size) noexcept
		{
			BaseType::Reserve(size + 1);
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void Remove(const CharType* const elementIt) noexcept
		{
			BaseType::Remove(elementIt);
			SetNullTerminator();
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void Remove(const ConstView view) noexcept
		{
			BaseType::Remove(view);
			if (HasElements())
			{
				SetNullTerminator();
			}
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void operator--() noexcept
		{
			Assert(HasElements());
			Remove(end() - 1);
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void operator--(int) noexcept
		{
			Assert(HasElements());
			Remove(end() - 1);
		}

		using BaseType::GetData;
		[[nodiscard]] PURE_STATICS SizeType GetSize() const noexcept
		{
			return BaseType::GetSize() - (BaseType::m_size >= 1u);
		}
		[[nodiscard]] PURE_STATICS SizeType GetDataSize() const noexcept
		{
			return GetSize() * sizeof(CharType);
		}
		[[nodiscard]] PURE_STATICS SizeType GetCapacity() const noexcept
		{
			return BaseType::GetCapacity() - (BaseType::GetCapacity() >= 1u);
		}
		[[nodiscard]] PURE_STATICS bool IsEmpty() const noexcept
		{
			return BaseType::m_size <= 1u;
		}
		[[nodiscard]] PURE_STATICS bool HasElements() const noexcept
		{
			return BaseType::m_size > 1u;
		}

		template<typename SizeType>
		[[nodiscard]] PURE_STATICS SizeType ToIntegral() const noexcept
		{
			return GetView().template ToIntegral<SizeType>();
		}

		[[nodiscard]] PURE_STATICS double ToDouble() const noexcept
		{
			return GetView().ToDouble();
		}
		[[nodiscard]] PURE_STATICS float ToFloat() const noexcept
		{
			return GetView().ToFloat();
		}

		template<typename... Args>
		TString& Format(const TStringView<const char> format, Args&&... args) noexcept LIFETIME_BOUND
		{
			FormatInternal::FormatString<TString>(*this, format, Forward<Args>(args)...);
			return *this;
		}

		void MakeLower() noexcept
		{
			for (CharType *pCharacter = GetData(), *endIt = end(); pCharacter != endIt; ++pCharacter)
			{
				*pCharacter = View::MakeLower(*pCharacter);
			}
		}

		void MakeUpper() noexcept
		{
			for (CharType *pCharacter = GetData(), *endIt = end(); pCharacter != endIt; ++pCharacter)
			{
				*pCharacter = View::MakeUpper(*pCharacter);
			}
		}

		template<bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		void Emplace(const typename BaseType::ConstPointerType whereIt, const CharType element)
		{
			const uint32 index = static_cast<SizeType>(whereIt - begin().Get());
			BaseType::Resize(Math::Max(index, GetSize()) + 1, Memory::DefaultConstruct);
			BaseType::Emplace(begin() + index, Memory::DefaultConstruct, CharType(element));
			SetNullTerminator();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator==(const ConstView other) const noexcept
		{
			return ConstView{GetData(), GetSize()}.EqualsCaseSensitive(other);
		}
		template<typename OtherAllocatorType, uint8 OtherFlags, typename OtherCharType = CharType_>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator==(const TString<OtherCharType, OtherAllocatorType, OtherFlags>& other
		) const noexcept
		{
			return ConstView{GetData(), GetSize()}.EqualsCaseSensitive(static_cast<const ConstView>(other.GetView()));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator!=(const ConstView other) const noexcept
		{
			return !TString::operator==(other);
		}
		template<typename OtherAllocatorType, uint8 OtherFlags, typename OtherCharType = CharType_>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator!=(const TString<OtherCharType, OtherAllocatorType, OtherFlags>& other
		) const noexcept
		{
			return !TString::operator==(other);
		}

		template<bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		[[nodiscard]] TString operator+(const ConstView other) const noexcept
		{
			TString newString(Memory::Reserve, GetSize() + static_cast<SizeType>(other.GetSize()));

			typename BaseType::View memoryView = newString.m_allocator.GetView();
			memoryView.CopyFrom(GetView());
			memoryView += GetSize();
			memoryView.CopyFrom(other);

			newString.BaseType::m_size = GetSize() + static_cast<SizeType>(other.GetSize()) + 1;

			newString.SetNullTerminator();

			return newString;
		}
		template<
			typename OtherAllocatorType,
			uint8 OtherFlags,
			typename OtherCharType = CharType_,
			bool CanResize = SupportResize,
			typename = EnableIf<CanResize>>
		[[nodiscard]] TString operator+(const TString<OtherCharType, OtherAllocatorType, OtherFlags>& other) const noexcept
		{
			return operator+(other.GetView());
		}

		template<typename SourceCharType, typename OtherSizeType, bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		TString& operator+=(const TStringView<SourceCharType, OtherSizeType> other) noexcept LIFETIME_BOUND
		{
			Reserve(GetSize() + (SizeType)other.GetSize());

			if constexpr (TypeTraits::IsSame<TypeTraits::WithoutConst<SourceCharType>, CharType>)
			{
				(BaseType::m_allocator.GetView() + GetSize()).CopyFrom(other);
			}
			else if constexpr (TypeTraits::IsSame<CharType, char16_t> && TypeTraits::IsSame<TypeTraits::WithoutConst<SourceCharType>, wchar_t>)
			{
				(BaseType::m_allocator.GetView() + GetSize())
					.CopyFrom(TStringView<const char16_t, OtherSizeType>{reinterpret_cast<const char16_t*>(other.GetData()), other.GetSize()});
			}
			else if constexpr (TypeTraits::IsSame<CharType, wchar_t> && TypeTraits::IsSame<TypeTraits::WithoutConst<SourceCharType>, char16_t>)
			{
				(BaseType::m_allocator.GetView() + GetSize())
					.CopyFrom(TStringView<const wchar_t, OtherSizeType>{reinterpret_cast<const wchar_t*>(other.GetData()), other.GetSize()});
			}
			else if constexpr (TypeTraits::IsSame<CharType, UTF8CharType> && TypeTraits::IsSame<TypeTraits::WithoutConst<SourceCharType>, wchar_t>)
			{
				Internal::UTF16ToUTF8(
					TStringView<UTF8CharType>{BaseType::GetData() + GetSize(), other.GetSize()},
					TStringView<const char16_t, uint32>{reinterpret_cast<const char16_t*>(other.GetData()), other.GetSize()}
				);
			}
			else if constexpr (TypeTraits::IsSame<CharType, UTF8CharType> && TypeTraits::IsSame<TypeTraits::WithoutConst<SourceCharType>, char16_t>)
			{
				Internal::UTF16ToUTF8(
					TStringView<UTF8CharType>{BaseType::GetData() + GetSize(), other.GetSize()},
					TStringView<const char16_t, uint32>{reinterpret_cast<const char16_t*>(other.GetData()), other.GetSize()}
				);
			}
			else if constexpr (TypeTraits::IsSame<CharType, UTF8CharType> && TypeTraits::IsSame<TypeTraits::WithoutConst<SourceCharType>, char32_t>)
			{
				Internal::UTF32ToUTF8(
					TStringView<UTF8CharType>{BaseType::GetData() + GetSize(), other.GetSize()},
					TStringView<const char32_t, uint32>{reinterpret_cast<const char32_t*>(other.GetData()), other.GetSize()}
				);
			}
			else if constexpr ((TypeTraits::IsSame<CharType, wchar_t> || TypeTraits::IsSame<CharType, UTF8CharType>)&&TypeTraits::
			                     IsSame<TypeTraits::WithoutConst<SourceCharType>, UTF8CharType>)
			{
				Internal::UTF8ToUTF16(TStringView<char16_t>{reinterpret_cast<char16_t*>(BaseType::GetData()) + GetSize(), other.GetSize()}, other);
			}
			else
			{
				static_unreachable("Conversion not supported");
			}

			BaseType::m_size = GetSizeWithNullTerminator() + (SizeType)other.GetSize();

			SetNullTerminator();

			return *this;
		}

		template<typename OtherType, size Size, bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		TString& operator+=(OtherType (&constLiteral)[Size]) noexcept LIFETIME_BOUND
		{
			return operator+=(TStringView<OtherType, Memory::NumericSize<Size>>(constLiteral));
		}

		template<bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		TString& operator+=(const TString& other) noexcept LIFETIME_BOUND
		{
			return operator+=(other.GetView());
		}

		template<bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		[[nodiscard]] TString operator+(const CharType character) const noexcept
		{
			TString newString(Memory::ConstructWithSize, Memory::Uninitialized, GetSize() + 1);

			newString.m_allocator.GetView().CopyFrom(GetView());
			newString.m_allocator.GetData()[GetSize()] = character;

			return newString;
		}

		template<bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		TString& operator+=(const CharType character) noexcept LIFETIME_BOUND
		{
			if (BaseType::m_allocator.GetCapacity() < BaseType::m_size + 1)
			{
				BaseType::m_allocator.Allocate(BaseType::m_size + 1);
			}

			BaseType::m_allocator.GetData()[GetSize()] = character;
			BaseType::m_size = GetSizeWithNullTerminator() + 1;

			SetNullTerminator();

			return *this;
		}

		template<
			typename OtherAllocatorType,
			uint8 OtherFlags,
			typename OtherCharType = CharType_,
			bool CanResize = SupportResize,
			typename = EnableIf<CanResize>>
		TString& operator+=(const TString<OtherCharType, OtherAllocatorType, OtherFlags>& other) noexcept LIFETIME_BOUND
		{
			operator+=(other.GetView());

			return *this;
		}

		template<bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		void ReplaceFirstOccurrence(const ConstView searchString, const ConstView replacedString)
		{
			ConstView stringMatch = searchString;
			for (CharType *it = GetData(), *itEnd = GetData() + GetSize(); it != itEnd; ++it)
			{
				if (*it == stringMatch[0])
				{
					stringMatch++;
					if (stringMatch.IsEmpty())
					{
						CharType* startIt = it - searchString.GetSize() + 1;
						BaseType::Remove(ConstView(startIt, searchString.GetSize()));
						BaseType::CopyEmplaceRange(startIt, Memory::Uninitialized, replacedString);
						break;
					}
				}
				else
				{
					stringMatch = searchString;
				}
			}
		}

		template<bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		void ReplaceAllOccurrences(const CharType character, const ConstView replacedString)
		{
			for (CharType *it = GetData(), *itEnd = GetData() + GetSize(); it != itEnd; ++it)
			{
				if (*it == character)
				{
					BaseType::Remove(it);
					const typename BaseType::View emplacedRange = BaseType::CopyEmplaceRange(it, Memory::Uninitialized, replacedString);
					it = emplacedRange.begin();
					itEnd = end();
				}
			}
		}

		template<bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		void ReplaceAllOccurrences(const ConstView searchString, const ConstView replacedString)
		{
			ConstView stringMatch = searchString;
			for (CharType *it = GetData(), *itEnd = GetData() + GetSize(); it != itEnd; ++it)
			{
				if (*it == stringMatch[0])
				{
					stringMatch++;
					if (stringMatch.IsEmpty())
					{
						CharType* startIt = it - searchString.GetSize() + 1;
						BaseType::Remove(ConstView(startIt, searchString.GetSize()));
						const typename BaseType::View emplacedRange = BaseType::CopyEmplaceRange(startIt, Memory::Uninitialized, replacedString);
						it = emplacedRange.begin();
						itEnd = end();
						stringMatch = searchString;
					}
				}
				else
				{
					stringMatch = searchString;
				}
			}
		}

		using BaseType::MoveEmplaceRange;
		using BaseType::MoveEmplaceRangeBack;
		using BaseType::CopyEmplaceRange;
		using BaseType::CopyEmplaceRangeBack;

		void ReplaceCharacterOccurrences(const CharType character, const CharType replacement) noexcept
		{
			GetView().ReplaceCharacterOccurrences(character, replacement);
		}

		void TrimNumberOfTrailingCharacters(const SizeType count) noexcept
		{
			BaseType::m_size -= Math::Min(count, GetSize());
			SetNullTerminator();
		}

		void TrimTrailingCharacters(const CharType character) noexcept
		{
			SizeType count = 0;
			for (const CharType *it = end() - 1, *lastIt = begin() - 1; it != lastIt; --it)
			{
				if (*it == character)
				{
					count++;
				}
				else
				{
					break;
				}
			}
			TrimNumberOfTrailingCharacters(count);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr CharType& operator[](const SizeType index) noexcept LIFETIME_BOUND
		{
			Expect(index < BaseType::m_size);
			return *(GetData() + index);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const CharType& operator[](const SizeType index) const noexcept LIFETIME_BOUND
		{
			Expect(index < BaseType::m_size);
			return *(GetData() + index);
		}

		template<typename... Args>
		[[nodiscard]] static TString Merge(const Args&... args) noexcept
		{
			SizeType totalSize = 0u;
			(GetMergedSize(totalSize, args), ...);

			// Reserve memory
			TString result(Memory::Reserve, totalSize);
			// Add the paths
			(MergeInternal(result, args), ...);
			return result;
		}

		// Encodes / escapes a strings content for use as URLs and more
		[[nodiscard]] static TString Escape(const ConstView view) noexcept;

		// Decodes / unescapes a strings content from use as a URL or similar
		[[nodiscard]] static Optional<TString> Unescape(const ConstView view) noexcept;

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	protected:
		FORCE_INLINE PURE_STATICS static void GetMergedSize(SizeType& sizeOut, const ConstView path) noexcept
		{
			sizeOut += path.GetSize();
		}

		FORCE_INLINE PURE_STATICS static void GetMergedSize(SizeType& sizeOut, const CharType) noexcept
		{
			sizeOut++;
		}

		template<bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		FORCE_INLINE static void MergeInternal(TString& target, const ConstView path) noexcept
		{
			target += path;
		}

		template<bool CanResize = SupportResize, typename = EnableIf<CanResize>>
		FORCE_INLINE static void MergeInternal(TString& target, const CharType character) noexcept
		{
			target += character;
		}

		[[nodiscard]] PURE_STATICS FORCE_INLINE SizeType GetSizeWithNullTerminator() const noexcept
		{
			return Math::Max(BaseType::m_size, (SizeType)1);
		}
	protected:
		FORCE_INLINE void SetNullTerminator()
		{
			this->operator[](BaseType::m_size - 1u) = '\0';
		}
	};
}
