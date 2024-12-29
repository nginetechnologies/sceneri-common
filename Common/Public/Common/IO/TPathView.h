#pragma once

#include <Common/Memory/Containers/StringView.h>

#include <Common/IO/ForwardDeclarations/Path.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::IO
{
	template<typename CharType_, uint8, CharType_ PathSeprator_, uint16 MaximumPathLength_>
	struct TPath;

	template<typename CharType_, uint8 Flags, CharType_ PathSeparator_, uint16 MaximumPathLength_>
	struct TRIVIAL_ABI TPathView : protected TStringView<const CharType_, uint16>
	{
		using PathType = TPath<CharType_, Flags, PathSeparator_, MaximumPathLength_>;

		using BaseType = TStringView<const CharType_, uint16>;

		using CharType = typename BaseType::CharType;
		using SizeType = typename BaseType::SizeType;
		using IndexType = typename BaseType::IndexType;

		inline static constexpr bool CaseSensitive = (PathFlags{Flags} & PathFlags::CaseSensitive) == PathFlags::CaseSensitive;
		inline static constexpr bool SupportsQueryStrings = (PathFlags{Flags} & PathFlags::SupportQueries) == PathFlags::SupportQueries;

		inline static constexpr CharType PathSeparator = PathSeparator_;
		inline static constexpr SizeType MaximumPathLength = MaximumPathLength_;

		using StringViewType = TStringView<CharType, uint16>;
		using ConstStringViewType = TStringView<const CharType, uint16>;

#if PLATFORM_WINDOWS
		inline static constexpr ConstStringViewType ExtendedPathPrefix = ConstStringViewType{MAKE_LITERAL(CharType, "\\\\?\\"), 4};
#endif

		constexpr TPathView() = default;
		template<size Size>
		constexpr explicit TPathView(const CharType (&data)[Size])
			: BaseType(data)
		{
		}
		constexpr TPathView(CharType* pBegin, const SizeType count)
			: BaseType(pBegin, count)
		{
		}
		template<typename OtherViewSizeType>
		constexpr TPathView(const TStringView<CharType, OtherViewSizeType> view)
			: BaseType(view)
		{
		}
		TPathView(PathType&&) = delete;
		TPathView& operator=(PathType&&) = delete;

		constexpr inline ConstStringViewType GetStringView() const
		{
			return *this;
		}

		using BaseType::begin;
		using BaseType::end;
		using BaseType::GetSize;
		using BaseType::IsEmpty;
		using BaseType::HasElements;
		using BaseType::FindFirstOf;
		using BaseType::FindLastOf;
		using BaseType::InvalidPosition;
		using BaseType::GetData;

		[[nodiscard]] constexpr CharType operator[](const IndexType index) const
		{
			return BaseType::operator[](index);
		}

		[[nodiscard]] constexpr bool operator==(TPathView otherPath) const
		{
			TPathView thisPath = *this;
#if PLATFORM_WINDOWS
			// Ensure we can compare extended and non-extended strings
			if (thisPath.StartsWith(ExtendedPathPrefix))
			{
				thisPath += ExtendedPathPrefix.GetSize();
			}
			if (otherPath.StartsWith(ExtendedPathPrefix))
			{
				otherPath += ExtendedPathPrefix.GetSize();
			}
#endif

			if constexpr (CaseSensitive)
			{
				return thisPath.GetStringView().EqualsCaseSensitive(otherPath.GetStringView());
			}
			else
			{
				return thisPath.GetStringView().EqualsCaseInsensitive(otherPath.GetStringView());
			}
		}

		[[nodiscard]] constexpr bool operator!=(const TPathView otherPath) const
		{
			return !operator==(otherPath);
		}

		[[nodiscard]] constexpr TPathView GetSubView(const SizeType offset, const SizeType count) const
		{
			const ConstStringViewType view = BaseType::GetSubstring(offset, count);
			return TPathView(view.begin(), view.GetSize());
		}

		[[nodiscard]] bool HasExtension() const;
		[[nodiscard]] TPathView GetRightMostExtension() const;
		[[nodiscard]] TPathView GetLeftMostExtension() const;
		[[nodiscard]] TPathView GetAllExtensions() const;
		[[nodiscard]] TPathView GetParentExtension() const;
		//! Whether the path's extensions starts with the specified extensions
		[[nodiscard]] bool StartsWithExtensions(const TPathView extension) const;
		//! Whether the path's extensions ends with the specified extensions
		[[nodiscard]] bool EndsWithExtensions(const TPathView extension) const;
		//! Whether the path's extensions equal the specified extensions
		[[nodiscard]] bool HasExactExtensions(const TPathView extension) const;
		[[nodiscard]] TPathView GetFileName() const;
		[[nodiscard]] TPathView GetFileNameWithoutExtensions() const;
		[[nodiscard]] TPathView GetWithoutExtensions() const;
		[[nodiscard]] TPathView GetWithoutQueryString() const;
		[[nodiscard]] TPathView GetParentPath() const;
		[[nodiscard]] TPathView GetFirstPath() const;
		[[nodiscard]] TPathView GetSharedParentPath(const TPathView other) const;
		[[nodiscard]] bool IsRelativeTo(const TPathView other) const;
		[[nodiscard]] TPathView GetRelativeToParent(const TPathView parent) const;
		[[nodiscard]] TPathView GetProtocol() const;
		[[nodiscard]] TPathView GetFullDomain() const;
		[[nodiscard]] TPathView GetFullDomainWithProtocol() const;
		[[nodiscard]] TPathView GetPort() const;
		[[nodiscard]] TPathView GetPath() const;
		[[nodiscard]] TPathView GetQueryString() const;
		[[nodiscard]] bool HasQueryString() const;
		[[nodiscard]] TPathView GetQueryParameterValue(const TPathView parameter) const;
		[[nodiscard]] bool HasQueryParameter(const TPathView parameter) const;
		[[nodiscard]] TPathView GetFragment() const;
		[[nodiscard]] uint16 GetDepth() const;

		[[nodiscard]] PURE_STATICS bool StartsWith(const TPathView other) const noexcept
		{
			return BaseType::StartsWith(other);
		}
		[[nodiscard]] PURE_STATICS bool EndsWith(const TPathView other) const noexcept
		{
			return BaseType::EndsWith(other);
		}

		using BaseType::operator==;
		using BaseType::operator!=;

		[[nodiscard]] bool operator>(TPathView otherPath) const
		{
			TPathView thisPath = *this;
#if PLATFORM_WINDOWS
			// Ensure we can compare extended and non-extended strings
			if (thisPath.StartsWith(ExtendedPathPrefix))
			{
				thisPath += ExtendedPathPrefix.GetSize();
			}
			if (otherPath.StartsWith(ExtendedPathPrefix))
			{
				otherPath += ExtendedPathPrefix.GetSize();
			}
#endif

			if constexpr (CaseSensitive)
			{
				return thisPath.GetStringView().GreaterThanCaseSensitive(otherPath.GetStringView());
			}
			else
			{
				return thisPath.GetStringView().GreaterThanCaseInsensitive(otherPath.GetStringView());
			}
		}

		[[nodiscard]] bool operator<(TPathView otherPath) const
		{
			TPathView thisPath = *this;
#if PLATFORM_WINDOWS
			// Ensure we can compare extended and non-extended strings
			if (thisPath.StartsWith(ExtendedPathPrefix))
			{
				thisPath += ExtendedPathPrefix.GetSize();
			}
			if (otherPath.StartsWith(ExtendedPathPrefix))
			{
				otherPath += ExtendedPathPrefix.GetSize();
			}
#endif

			if constexpr (CaseSensitive)
			{
				return thisPath.GetStringView().LessThanCaseSensitive(otherPath.GetStringView());
			}
			else
			{
				return thisPath.GetStringView().LessThanCaseInsensitive(otherPath.GetStringView());
			}
		}

		constexpr TPathView& operator+=(const SizeType offset)
		{
			BaseType::operator+=(offset);
			return *this;
		}
		constexpr TPathView& operator-=(const SizeType offset)
		{
			BaseType::operator-=(offset);
			return *this;
		}
	};
}
