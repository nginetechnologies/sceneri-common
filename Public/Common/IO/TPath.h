#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Memory/Move.h>
#include <Common/IO/ForwardDeclarations/Path.h>
#include <Common/IO/PathView.h>
#include <Common/Memory/Containers/StringBase.h>
#include <Common/Memory/Allocators/DynamicInlineStorageAllocator.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/IO/ForwardDeclarations/ZeroTerminatedPathView.h>
#include <Common/IO/PathFlags.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	struct Project;

	namespace Time
	{
		struct Timestamp;
	}
}

namespace ngine::IO
{
	template<typename CharType_, uint8 Flags, CharType_ PathSeprator_, uint16 MaximumPathLength_>
	struct TRIVIAL_ABI TPath
	{
#if PLATFORM_WINDOWS
		inline static constexpr uint16 MaximumShortPathLength = 260;
#endif

		using ViewType = TPathView<CharType_, Flags, PathSeprator_, MaximumPathLength_>;
		using ConstViewType = TPathView<const CharType_, Flags, PathSeprator_, MaximumPathLength_>;

		using CharType = TypeTraits::WithoutConst<typename ViewType::CharType>;
		using StringType = TString<
			CharType,
			Memory::DynamicInlineStorageAllocator<CharType, 250, uint16>,
			Memory::VectorFlags::AllowReallocate | Memory::VectorFlags::AllowResize>;
		using StringViewType = TStringView<CharType, uint16>;
		using ConstStringViewType = typename ViewType::ConstStringViewType;
		using ConstZeroTerminatedStringView = TZeroTerminatedPathView<const CharType>;
		using ZeroTerminatedStringView = TZeroTerminatedPathView<CharType>;
		using SizeType = typename ViewType::SizeType;

		struct Hash
		{
			using is_transparent = void;

			size operator()(const ViewType path) const
			{
				typename ConstStringViewType::Hash hash;
				return hash(path.GetStringView());
			}
		};

		inline static constexpr CharType PathSeparator = ViewType::PathSeparator;
		inline static constexpr SizeType MaximumPathLength = ViewType::MaximumPathLength;

#if PLATFORM_WINDOWS
		inline static constexpr ConstStringViewType ExtendedPathPrefix = ViewType::ExtendedPathPrefix;
#endif

		TPath() = default;
		TPath(const TPath& other)
			: m_path(other.m_path)
		{
		}
		TPath& operator=(const TPath& other)
		{
			m_path = other.m_path;
			return *this;
		}
		TPath(TPath&& other)
			: m_path(Move(other.m_path))
		{
		}
		TPath& operator=(TPath&& other)
		{
			m_path = Move(other.m_path);
			return *this;
		}

		template<size Size>
		explicit TPath(const CharType (&data)[Size])
			: m_path(data, Size - 1)
		{
			AdjustLongPathsIfNecessary();
		}
		TPath(const CharType* pData, const typename StringType::SizeType size)
			: m_path(pData, size)
		{
			AdjustLongPathsIfNecessary();
		}
		explicit TPath(const ViewType view)
			: m_path(view.GetStringView())
		{
		}
		template<typename ViewSizeType>
		explicit TPath(const TStringView<const CharType, ViewSizeType> view)
			: m_path(view)
		{
		}
		explicit TPath(StringType&& other)
			: m_path(Move(other))
		{
		}
		TPath(const Memory::ReserveType type, const SizeType capacity)
			: m_path(type, capacity + 1)
		{
		}
		~TPath() = default;

		[[nodiscard]] operator ViewType() const LIFETIME_BOUND
		{
			return {m_path.GetView().begin(), m_path.GetSize()};
		}
		[[nodiscard]] StringViewType GetMutableView() LIFETIME_BOUND
		{
			return {m_path.GetView().begin(), m_path.GetSize()};
		}
		[[nodiscard]] ViewType GetView() const LIFETIME_BOUND
		{
			return {m_path.GetView().begin(), m_path.GetSize()};
		}

		[[nodiscard]] operator ConstZeroTerminatedStringView() const LIFETIME_BOUND
		{
			return {m_path.GetData(), static_cast<SizeType>(m_path.GetSize() + static_cast<SizeType>(1))};
		}
		[[nodiscard]] ConstZeroTerminatedStringView GetZeroTerminated() const LIFETIME_BOUND
		{
			return *this;
		}
		[[nodiscard]] operator ZeroTerminatedStringView() LIFETIME_BOUND
		{
			return {m_path.GetData(), static_cast<SizeType>(m_path.GetSize() + static_cast<SizeType>(1))};
		}
		[[nodiscard]] ZeroTerminatedStringView GetZeroTerminated() LIFETIME_BOUND
		{
			return *this;
		}

		[[nodiscard]] bool IsEmpty() const
		{
			return m_path.IsEmpty();
		}
		[[nodiscard]] bool HasElements() const
		{
			return m_path.HasElements();
		}

		[[nodiscard]] bool IsRelativeTo(const ViewType other) const
		{
			return GetView().IsRelativeTo(other);
		}

		[[nodiscard]] bool HasExtension() const LIFETIME_BOUND
		{
			return GetView().HasExtension();
		}
		[[nodiscard]] bool StartsWithExtensions(const ViewType extension) const LIFETIME_BOUND
		{
			return GetView().StartsWithExtensions(extension);
		}
		[[nodiscard]] bool EndsWithExtensions(const ViewType extension) const LIFETIME_BOUND
		{
			return GetView().EndsWithExtensions(extension);
		}
		[[nodiscard]] bool HasExactExtensions(const ViewType extension) const LIFETIME_BOUND
		{
			return GetView().HasExactExtensions(extension);
		}
		[[nodiscard]] ViewType GetRightMostExtension() const LIFETIME_BOUND
		{
			return GetView().GetRightMostExtension();
		}
		[[nodiscard]] ViewType GetLeftMostExtension() const LIFETIME_BOUND
		{
			return GetView().GetLeftMostExtension();
		}
		[[nodiscard]] ViewType GetAllExtensions() const LIFETIME_BOUND
		{
			return GetView().GetAllExtensions();
		}
		[[nodiscard]] ViewType GetFileName() const LIFETIME_BOUND
		{
			return GetView().GetFileName();
		}
		[[nodiscard]] ViewType GetFileNameWithoutExtensions() const LIFETIME_BOUND
		{
			return GetView().GetFileNameWithoutExtensions();
		}
		[[nodiscard]] ViewType GetWithoutExtensions() const LIFETIME_BOUND
		{
			return GetView().GetWithoutExtensions();
		}
		[[nodiscard]] ViewType GetParentPath() const LIFETIME_BOUND
		{
			return GetView().GetParentPath();
		}
		[[nodiscard]] ViewType GetRelativeToParent(const ViewType parent) const LIFETIME_BOUND
		{
			return GetView().GetRelativeToParent(parent);
		}
		void ReplaceAllExtensions(const ViewType newExtension);
		void ReplaceFileNameWithoutExtensions(const ViewType newFileName);
		void ResolveAbsolutePath();

		bool OpenWithAssociatedApplication() const;

		friend struct Filesystem;
		friend Project;

		void MakeRelativeToParent(const ViewType parent)
		{
			const ViewType relativeToParent = GetView().GetRelativeToParent(parent);
			const StringViewType removedView = m_path.GetView().GetSubstring(0u, GetSize() - relativeToParent.GetSize());
			m_path.Remove(removedView);
		}

		void MakeRelativeTo(const ViewType other);

		void TrimNumberOfTrailingCharacters(const typename StringType::SizeType count)
		{
			m_path.TrimNumberOfTrailingCharacters(count);
		}

		void TrimTrailingCharacters(const CharType character)
		{
			m_path.TrimTrailingCharacters(character);
		}

		void TrimTrailingSeparators()
		{
			m_path.TrimTrailingCharacters(IO::PathSeparator);
		}

		TPath& MakeForwardSlashes()
		{
#if PLATFORM_WINDOWS
			m_path.ReplaceCharacterOccurrences(PathSeparator, MAKE_PATH_LITERAL('/'));
#endif
			return *this;
		}

		TPath& MakeNativeSlashes()
		{
#if PLATFORM_WINDOWS
			m_path.ReplaceCharacterOccurrences(MAKE_PATH_LITERAL('/'), PathSeparator);
#else
			m_path.ReplaceCharacterOccurrences(MAKE_PATH_LITERAL('\\'), PathSeparator);
#endif
			return *this;
		}

		TPath& MakeLower()
		{
			m_path.MakeLower();
			return *this;
		}

		TPath& MakeUpper()
		{
			m_path.MakeUpper();
			return *this;
		}

		void AssignFrom(const ViewType view)
		{
			m_path = view.GetStringView();
		}

		[[nodiscard]] SizeType GetSize() const
		{
			return m_path.GetSize();
		}

		[[nodiscard]] SizeType GetDataSize() const
		{
			return m_path.GetDataSize();
		}

		[[nodiscard]] bool operator==(const ViewType otherPath) const;
		[[nodiscard]] bool operator!=(const ViewType otherPath) const;
		[[nodiscard]] bool operator<(const ViewType otherPath) const;
		[[nodiscard]] bool operator>(const ViewType otherPath) const;

		template<typename... Args>
		[[nodiscard]] static TPath Combine(const Args&... args)
		{
			// Start with calculating number of path separator characters
			typename ViewType::SizeType totalSize = sizeof...(args);
			(GetRequiredPathSize(totalSize, args), ...);

			// Reserve memory
			TPath result(Memory::Reserve, totalSize);
			// Add the paths
			uint8 remainingPaths = sizeof...(args);
			(Add(result, remainingPaths, args), ...);
			result.AdjustLongPathsIfNecessary();
			return result;
		}

		template<typename... Args>
		[[nodiscard]] static TPath Merge(const Args&... args)
		{
			typename ViewType::SizeType totalSize = 0u;
			(GetRequiredPathSize(totalSize, args), ...);

			// Reserve memory
			TPath result(Memory::Reserve, totalSize);
			// Add the paths
			(MergeInternal(result, args), ...);
			result.AdjustLongPathsIfNecessary();
			return result;
		}

		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	protected:
		void AdjustLongPathsIfNecessary()
		{
#if PLATFORM_WINDOWS
			if constexpr (TypeTraits::IsSame<CharType, PathCharType>)
			{
				if (m_path.GetSize() > MaximumShortPathLength && !m_path.GetView().StartsWith(ExtendedPathPrefix))
				{
					m_path.CopyEmplaceRange(m_path.begin(), Memory::Uninitialized, ExtendedPathPrefix);
				}
			}
#endif
		}
	protected:
		static void GetRequiredPathSize(typename ViewType::SizeType& sizeOut, const ViewType path)
		{
			sizeOut += path.GetSize();
		}

		template<typename... ViewArgs>
		static void GetRequiredPathSize(typename ViewType::SizeType& sizeOut, const TStringView<ViewArgs...> path)
		{
			sizeOut += static_cast<SizeType>(path.GetSize());
		}

		template<typename... ViewArgs>
		static void GetRequiredPathSize(typename ViewType::SizeType& sizeOut, const TZeroTerminatedStringView<ViewArgs...> path)
		{
			sizeOut += static_cast<SizeType>(path.GetSize());
		}

		template<size Size>
		static void GetRequiredPathSize(typename ViewType::SizeType& sizeOut, const CharType (&)[Size])
		{
			sizeOut += Size;
		}

		static void GetRequiredPathSize(typename ViewType::SizeType& sizeOut, const typename ViewType::CharType)
		{
			sizeOut++;
		}

		template<typename... ViewArgs>
		static void Add(TPath& target, uint8& remainingPathCount, const TStringView<ViewArgs...> path)
		{
			target.m_path += path;

			remainingPathCount--;
			if (remainingPathCount > 0 && path.HasElements())
			{
				target.m_path += PathSeparator;
			}
		}

		template<typename... ViewArgs>
		static void Add(TPath& target, uint8& remainingPathCount, const TZeroTerminatedStringView<ViewArgs...> path)
		{
			target.m_path += path;

			remainingPathCount--;
			if (remainingPathCount > 0 && path.HasElements())
			{
				target.m_path += PathSeparator;
			}
		}

		static void Add(TPath& target, uint8& remainingPathCount, const ViewType path)
		{
			Add(target, remainingPathCount, path.GetStringView());
		}

		template<size Size>
		static void Add(TPath& target, uint8& remainingPathCount, const CharType (&data)[Size])
		{
			Add(target, remainingPathCount, ViewType(data));
		}

		static void Add(TPath& target, uint8& remainingPathCount, const typename ViewType::CharType character)
		{
			target.m_path += character;

			remainingPathCount--;
			if (remainingPathCount > 0)
			{
				target.m_path += PathSeparator;
			}
		}

		template<typename... ViewArgs>
		static void MergeInternal(TPath& target, const TStringView<ViewArgs...> path)
		{
			target.m_path += path;
		}

		static void MergeInternal(TPath& target, const ViewType path)
		{
			target.m_path += path.GetStringView();
		}

		template<size Size>
		static void MergeInternal(TPath& target, const CharType (&data)[Size])
		{
			target.m_path += typename StringType::ConstView(data);
		}

		static void MergeInternal(TPath& target, const typename ViewType::CharType character)
		{
			target.m_path += character;
		}
	protected:
		StringType m_path;
	};

	template<typename SizeType, typename CharType_, uint8 Flags, CharType_ PathSeparator_, uint16 MaximumPathLength_>
	[[nodiscard]] inline bool
	operator==(const TPath<CharType_, Flags, PathSeparator_, MaximumPathLength_>& left, const TStringView<CharType_, SizeType> right)
	{
		return left.GetView() == right;
	}

	template<typename SizeType, typename CharType_, uint8 Flags, CharType_ PathSeparator_, uint16 MaximumPathLength_>
	[[nodiscard]] inline bool
	operator==(const TStringView<CharType_, SizeType> left, const TPath<CharType_, Flags, PathSeparator_, MaximumPathLength_>& right)
	{
		return right.GetView() == left;
	}
}
