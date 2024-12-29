#pragma once

#include <Common/Memory/Containers/ArrayView.h>
#include "ForwardDeclarations/FixedArrayView.h"
#include <Common/TypeTraits/IsSame.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	template<typename ContainedType, size Size_, typename InternalIndexType, typename InternalSizeType, uint8 Flags_>
	struct TRIVIAL_ABI FixedArrayView
	{
		inline static constexpr uint8 Flags = Flags_;
		inline static constexpr bool IsRestricted = (Flags & (uint8)ArrayViewFlags::Restrict) != 0;

		inline static constexpr size Size = Size_;
		using SizeType = InternalSizeType;
		using IndexType = InternalIndexType;
		using DataSizeType = Memory::NumericSize<Size * sizeof(ContainedType)>;

		using StoredType = ContainedType;

		using View = FixedArrayView<ContainedType, Size, IndexType, SizeType, Flags>;
		using ConstView = FixedArrayView<const ContainedType, Size, IndexType, SizeType, Flags>;

		using DynamicView = ArrayView<ContainedType, SizeType, IndexType, StoredType, Flags>;
		using ConstDynamicView = ArrayView<const ContainedType, SizeType, IndexType, const StoredType, Flags>;
		using RestrictedView = FixedArrayView<ContainedType, Size, IndexType, SizeType, (uint8)ArrayViewFlags::Restrict>;
		using ConstRestrictedView = FixedArrayView<const ContainedType, Size, IndexType, SizeType, (uint8)ArrayViewFlags::Restrict>;

		using PointerType = typename DynamicView::PointerType;
		using ConstPointerType = typename DynamicView::ConstPointerType;
		using StoredPointerType = typename DynamicView::StoredPointerType;
		using ConstStoredPointerType = typename DynamicView::ConstStoredPointerType;

		using ReferenceType = typename DynamicView::ReferenceType;

		using IteratorType = typename DynamicView::IteratorType;
		using ReverseIteratorType = typename DynamicView::ReverseIteratorType;
		using ConstIteratorType = typename DynamicView::ConstIteratorType;
		using ConstReverseIteratorType = typename DynamicView::ConstReverseIteratorType;
		using iterator = typename DynamicView::iterator;
		using reverse_iterator = typename DynamicView::reverse_iterator;
		using const_iterator = typename DynamicView::const_iterator;
		using const_reverse_iterator = typename DynamicView::const_reverse_iterator;

		using OptionalIteratorType = typename DynamicView::OptionalIteratorType;
		using OptionalReverseIteratorType = typename DynamicView::OptionalReverseIteratorType;
		using OptionalConstIteratorType = typename DynamicView::OptionalConstIteratorType;
		using OptionalConstReverseIteratorType = typename DynamicView::OptionalConstReverseIteratorType;

		constexpr FixedArrayView() = default;
		constexpr FixedArrayView(ContainedType (&element)[Size])
			: m_data(element)
		{
		}

		template<
			typename OtherElementType,
			typename OtherIndexType,
			typename OtherSizeType = OtherIndexType,
			typename ElementType = ContainedType,
			typename = EnableIf<
				TypeTraits::IsBaseOf<ElementType, OtherElementType> && sizeof(ElementType) == sizeof(OtherElementType) &&
				alignof(ElementType) == alignof(OtherElementType)>>
		FORCE_INLINE constexpr FixedArrayView(const FixedArrayView<OtherElementType, Size, OtherIndexType, OtherSizeType, Flags> otherView
		) noexcept
			: m_data(reinterpret_cast<ContainedType (&)[Size]>(otherView.m_data))
		{
		}
		template<
			typename OtherElementType,
			typename OtherIndexType,
			typename OtherSizeType = OtherIndexType,
			typename ElementType = ContainedType,
			typename = EnableIf<
				TypeTraits::IsBaseOf<ElementType, OtherElementType> && sizeof(ElementType) == sizeof(OtherElementType) &&
				alignof(ElementType) == alignof(OtherElementType)>>
		FORCE_INLINE constexpr FixedArrayView&
		operator=(const FixedArrayView<OtherElementType, Size, OtherIndexType, OtherSizeType, Flags> otherView) noexcept
		{
			m_data = reinterpret_cast<ContainedType(&)[Size]>(otherView.m_data);
			return *this;
		}

		/*template<typename OtherElementType, typename OtherIndexType, typename OtherSizeType = OtherIndexType,
	typename ElementType = ContainedType,
	typename = EnableIf<
		TypeTraits::IsBaseOf<ElementType, OtherElementType> && (sizeof(ElementType) != sizeof(OtherElementType) || alignof(ElementType) !=
alignof(OtherElementType))>> FORCE_INLINE constexpr FixedArrayView(const FixedArrayView<OtherElementType, Size, OtherIndexType,
OtherSizeType, Flags> otherView) noexcept : m_data(otherView.GetData())
{
}
template<
	typename OtherElementType,
	typename OtherSizeType,
	typename OtherIndexType = OtherSizeType,
	typename ElementType = ContainedType,
	typename = EnableIf<
		TypeTraits::IsBaseOf<ElementType, OtherElementType> &&
		(sizeof(ElementType) != sizeof(OtherElementType) || alignof(ElementType) != alignof(OtherElementType))>>
FORCE_INLINE constexpr FixedArrayView&
operator=(const FixedArrayView<OtherElementType, Size, OtherIndexType, OtherSizeType, Flags> otherView) noexcept
{
	m_data = otherView.m_data;
	return *this;
}*/

		[[nodiscard]] FORCE_INLINE PURE_STATICS operator RestrictedView() const noexcept
		{
			return {m_data};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr SizeType GetSize() noexcept
		{
			return Size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr bool IsEmpty() noexcept
		{
			return Size == 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr bool HasElements() noexcept
		{
			return !IsEmpty();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr DataSizeType GetDataSize() noexcept
		{
			return sizeof(StoredType) * Size;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ContainedType* GetData() const
		{
			return m_data;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ReferenceType operator[](const IndexType index) const noexcept
		{
			Assert((SizeType)index < Size);
			return m_data[(SizeType)index];
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS ReferenceType GetLastElement() const noexcept
		{
			Expect(GetSize() > 0);
			return operator[](GetSize() - 1);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS ReferenceType GetSecondLastElement() const noexcept
		{
			Expect(GetSize() > 1);
			return operator[](GetSize() - 2);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr IteratorType begin() const noexcept
		{
			return m_data;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr IteratorType end() const noexcept
		{
			return m_data + Size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ReverseIteratorType rbegin() const noexcept
		{
			return m_data + Size - 1;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ReverseIteratorType rend() const noexcept
		{
			return m_data - 1;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsValidIndex(const IndexType index) const noexcept
		{
			return index < Size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr IndexType GetIteratorIndex(const ConstPointerType it) const noexcept
		{
			Assert((it >= begin()) & (it <= end()));
			return static_cast<IndexType>(it - begin());
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator DynamicView() const
		{
			return DynamicView{m_data, Size};
		}

		[[nodiscard]] FORCE_INLINE constexpr DynamicView GetDynamicView() const
		{
			return DynamicView(m_data, Size);
		}

		template<
			typename OtherElementType,
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename OtherStoredType = OtherElementType,
			typename ElementType = ContainedType,
			typename = EnableIf<
				TypeTraits::IsBaseOf<ElementType, OtherElementType> && sizeof(ElementType) == sizeof(OtherElementType) &&
				alignof(ElementType) == alignof(OtherElementType)>>
		[[nodiscard]] FORCE_INLINE constexpr
		operator ArrayView<OtherElementType, OtherSizeType, OtherIndexType, OtherStoredType, Flags>() const noexcept
		{
			return ArrayView<OtherElementType, OtherSizeType, OtherIndexType, OtherStoredType, Flags>{GetData(), Size};
		}

		template<
			typename OtherElementType,
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename OtherStoredType = OtherElementType,
			typename ElementType = ContainedType,
			typename = EnableIf<
				TypeTraits::IsBaseOf<ElementType, OtherElementType> && sizeof(ElementType) == sizeof(OtherElementType) &&
				alignof(ElementType) == alignof(OtherElementType)>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS operator FixedArrayView<OtherElementType, Size>() const noexcept
		{
			return FixedArrayView<OtherElementType, Size>{GetData()};
		}

		template<typename ComparableType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS OptionalIteratorType Find(const ComparableType& element) const noexcept
		{
			return DynamicView(*this).Find(element);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView Find(const DynamicView other) const
		{
			return DynamicView(*this).Find(other);
		}

		template<typename ComparableType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const ComparableType& element) const noexcept
		{
			return Find(element).IsValid();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const DynamicView other) const noexcept
		{
			return Find(other).HasElements();
		}

		template<typename ComparableType, typename OtherSizeType = SizeType, typename OtherIndexType = IndexType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool
		ContainsAny(const ArrayView<ComparableType, OtherSizeType, OtherIndexType, ComparableType> elements) const noexcept
		{
			return DynamicView(*this).ContainsAny(elements);
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS OptionalIteratorType FindIf(const Callback callback) const noexcept
		{
			return DynamicView(*this).FindIf(callback);
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool ContainsIf(const Callback callback) const noexcept
		{
			return FindIf(callback).IsValid();
		}

		template<typename Callback>
		FORCE_INLINE void ForEach(const Callback callback) noexcept
		{
			for (ConstPointerType it = begin(), endIt = end(); it != endIt; ++it)
			{
				callback(*it);
			}
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool All(const Callback callback) const noexcept
		{
			return DynamicView(*this).All(callback);
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Any(const Callback callback) const noexcept
		{
			return DynamicView(*this).Any(callback);
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS OptionalIteratorType LastFromStart(const Callback callback) noexcept
		{
			return DynamicView(*this).LastFromStart(callback);
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS auto Count(const Callback callback) const noexcept
		{
			return DynamicView(*this).Count(callback);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsWithinBounds(const ConstPointerType it) const noexcept
		{
			return (it >= GetData()) & (it < end());
		}
		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		IsWithinBounds(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> otherView) const noexcept
		{
			return (otherView.GetData() >= GetData()) & (otherView.end() <= end());
		}
		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		Overlaps(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> otherView) const noexcept
		{
			return IsWithinBounds(otherView) | otherView.IsWithinBounds(GetDynamicView());
		}

		template<typename ElementType = ContainedType, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		Contains(const ArrayView<const ContainedType, SizeType, IndexType, const StoredType> otherView) const noexcept
		{
			return (otherView.GetData() >= GetData()) & (otherView.end() <= end());
		}
		template<typename ElementType = ContainedType, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		Overlaps(const ArrayView<const ContainedType, SizeType, IndexType, const StoredType> otherView) const noexcept
		{
			return Contains(otherView) | otherView.Contains(*this);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubView(const SizeType index, SizeType count) const noexcept
		{
			return DynamicView(*this).GetSubView(index, count);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubViewFrom(const IteratorType it) const noexcept
		{
			return DynamicView(*this).GetSubViewFrom(it);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubViewFrom(const SizeType index) const noexcept
		{
			return DynamicView(*this).GetSubViewFrom(begin() + index);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubViewUpTo(const IteratorType it) const noexcept
		{
			return DynamicView(*this).GetSubViewUpTo(it);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubViewUpTo(const SizeType index) const noexcept
		{
			return DynamicView(*this).GetSubViewUpTo(begin() + index);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView operator+(SizeType offset) const noexcept
		{
			return DynamicView(*this).operator+(offset);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView operator-(const SizeType offset) const noexcept
		{
			return DynamicView(*this).operator-(offset);
		}
		template<typename OtherType, typename OtherSizeType, typename OtherIndexType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator==(const ArrayView<OtherType, OtherSizeType, OtherIndexType> other) const
		{
			return DynamicView(*this).operator==(other);
		}
		template<typename OtherType, typename OtherSizeType, typename OtherIndexType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator!=(const ArrayView<OtherType, OtherSizeType, OtherIndexType> other) const
		{
			return !operator==(other);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator==(const FixedArrayView other) const
		{
			return this->operator==(ConstDynamicView(other));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator!=(const FixedArrayView other) const
		{
			return !operator==(other);
		}

		template<typename ElementType = ContainedType, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		FORCE_INLINE bool CopyFrom(const ArrayView<const ContainedType, SizeType, IndexType, const StoredType> other) const noexcept
		{
			return DynamicView(*this).CopyFrom(other);
		}

		template<typename ElementType = ContainedType, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		FORCE_INLINE bool CopyFromWithOverlap(const ArrayView<const ContainedType, SizeType, IndexType, const StoredType> other) const noexcept
		{
			return DynamicView(*this).CopyFromWithOverlap(other);
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<TypeTraits::IsDefaultConstructible<ElementType> && !TypeTraits::IsConst<ElementType>>
		DefaultConstruct() const noexcept
		{
			DynamicView(*this).DefaultConstruct();
		}

		template<typename... ConstructArgs, typename ElementType = ContainedType>
		FORCE_INLINE constexpr EnableIf<!TypeTraits::IsConst<ElementType>> InitializeAll(ConstructArgs&&... args) const noexcept
		{
			DynamicView(*this).InitializeAll(Forward<ConstructArgs>(args)...);
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<!TypeTraits::IsConst<ElementType>> ZeroInitialize() const noexcept
		{
			DynamicView(*this).ZeroInitialize();
		}
		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<!TypeTraits::IsConst<ElementType>> OneInitialize() const noexcept
		{
			DynamicView(*this).OneInitialize();
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<
			!TypeTraits::IsConst<ElementType> && (TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsCopyConstructible<ElementType>)>
		CopyConstruct(const ConstView other) const noexcept
		{
			DynamicView(*this).CopyConstruct(other);
		}
		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<
			!TypeTraits::IsConst<ElementType> && (TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsCopyConstructible<ElementType>)>
		CopyConstruct(const ConstDynamicView other) const noexcept
		{
			DynamicView(*this).CopyConstruct(other);
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<
			!TypeTraits::IsConst<ElementType> && (TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsCopyConstructible<ElementType>)>
		MoveConstruct(const View other) const noexcept
		{
			DynamicView(*this).MoveConstruct(other.GetDynamicView());
		}
		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<
			!TypeTraits::IsConst<ElementType> && (TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsCopyConstructible<ElementType>)>
		MoveConstruct(const DynamicView other) const noexcept
		{
			DynamicView(*this).MoveConstruct(other);
		}

		FORCE_INLINE void DestroyElements() noexcept
		{
			DynamicView(*this).DestroyElements();
		}

		template<
			typename ElementType = ContainedType,
			bool IsMoveConstructible = TypeTraits::IsMoveConstructible<ElementType>,
			typename... Args>
		FORCE_INLINE
			EnableIf<!TypeTraits::IsConst<ElementType> && IsMoveConstructible /* && (TypeTraits::IsConvertibleTo<Args, ElementType> && ...)*/>
			MoveConstructAll(Args&&... args) const noexcept
		{
			static_assert(sizeof...(args) <= Size);
			DynamicView(*this).MoveConstructAll(Forward<Args>(args)...);
		}

		template<
			typename ElementType = ContainedType,
			bool IsCopyConstructible = TypeTraits::IsCopyConstructible<ElementType>,
			typename... Args>
		FORCE_INLINE
			EnableIf<!TypeTraits::IsConst<ElementType> && IsCopyConstructible /* && (TypeTraits::IsConvertibleTo<Args, ElementType> && ...)*/>
			CopyConstructAll(const Args&... args) const noexcept
		{
			static_assert(sizeof...(args) <= Size);
			DynamicView(*this).CopyConstructAll(args...);
		}

		template<typename... Args>
		bool Serialize(const Serialization::Reader serializer, Args&... args);
		template<typename... Args>
		bool Serialize(Serialization::Writer serializer, Args&... args) const;
	protected:
		template<typename OtherContainedType, size OtherSize, typename OtherIndexType, typename OtherSizeType, uint8 OtherFlags>
		friend struct FixedArrayView;

		ContainedType (&m_data)[Size];
	};

	template<typename ContainedType, typename InternalIndexType, typename InternalSizeType, uint8 Flags_>
	struct TRIVIAL_ABI FixedArrayView<ContainedType, 0, InternalIndexType, InternalSizeType, Flags_>
	{
		inline static constexpr uint8 Flags = Flags_;
		inline static constexpr bool IsRestricted = (Flags & (uint8)ArrayViewFlags::Restrict) != 0;

		inline static constexpr size Size = 0;
		using SizeType = InternalSizeType;
		using IndexType = InternalIndexType;
		using DataSizeType = uint8;

		using StoredType = ContainedType;

		using View = FixedArrayView<ContainedType, Size, IndexType, SizeType, Flags>;
		using ConstView = FixedArrayView<const ContainedType, Size, IndexType, SizeType, Flags>;

		using DynamicView = ArrayView<ContainedType, SizeType, IndexType, StoredType, Flags>;
		using ConstDynamicView = ArrayView<const ContainedType, SizeType, IndexType, const StoredType, Flags>;
		using RestrictedView = FixedArrayView<ContainedType, Size, IndexType, SizeType, (uint8)ArrayViewFlags::Restrict>;
		using ConstRestrictedView = FixedArrayView<const ContainedType, Size, IndexType, SizeType, (uint8)ArrayViewFlags::Restrict>;

		using PointerType = typename DynamicView::PointerType;
		using ConstPointerType = typename DynamicView::ConstPointerType;
		using StoredPointerType = typename DynamicView::StoredPointerType;
		using ConstStoredPointerType = typename DynamicView::ConstStoredPointerType;

		using ReferenceType = typename DynamicView::ReferenceType;

		using IteratorType = typename DynamicView::IteratorType;
		using ConstIteratorType = typename DynamicView::ConstIteratorType;
		using iterator = typename DynamicView::iterator;
		using const_iterator = typename DynamicView::const_iterator;

		using OptionalIteratorType = typename DynamicView::OptionalIteratorType;
		using OptionalConstIteratorType = typename DynamicView::OptionalConstIteratorType;

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr SizeType GetSize() noexcept
		{
			return 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr bool IsEmpty() noexcept
		{
			return true;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr bool HasElements() noexcept
		{
			return false;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr SizeType GetDataSize() noexcept
		{
			return 0;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ContainedType* GetData() const
		{
			return nullptr;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr StoredPointerType begin() const noexcept
		{
			return nullptr;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr StoredPointerType end() const noexcept
		{
			return nullptr;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsValidIndex(const IndexType) const noexcept
		{
			return false;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr IndexType GetIteratorIndex(const ConstPointerType) const noexcept
		{
			ExpectUnreachable();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator DynamicView() const
		{
			return {};
		}
		[[nodiscard]] FORCE_INLINE constexpr DynamicView GetDynamicView() const
		{
			return {};
		}

		template<typename ComparableType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS OptionalIteratorType Find(const ComparableType&) const noexcept
		{
			return Invalid;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView Find(const DynamicView) const
		{
			return {};
		}

		template<typename ComparableType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const ComparableType&) const noexcept
		{
			return false;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const DynamicView) const noexcept
		{
			return false;
		}

		template<typename ComparableType, typename OtherSizeType = SizeType, typename OtherIndexType = IndexType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool
		ContainsAny(const ArrayView<ComparableType, OtherSizeType, OtherIndexType, ComparableType>) const noexcept
		{
			return false;
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS OptionalIteratorType FindIf(const Callback) const noexcept
		{
			return false;
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool ContainsIf(const Callback) const noexcept
		{
			return false;
		}

		template<typename Callback>
		FORCE_INLINE void ForEach(const Callback) noexcept
		{
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool All(const Callback) const noexcept
		{
			return false;
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Any(const Callback) const noexcept
		{
			return false;
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS OptionalIteratorType LastFromStart(const Callback) noexcept
		{
			return Invalid;
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS SizeType Count(const Callback) const noexcept
		{
			return 0;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsWithinBounds(const ConstPointerType) const noexcept
		{
			return false;
		}
		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		IsWithinBounds(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags>) const noexcept
		{
			return false;
		}
		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		Overlaps(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags>) const noexcept
		{
			return false;
		}

		template<typename ElementType = ContainedType, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		Contains(const ArrayView<const ContainedType, SizeType, IndexType, const StoredType>) const noexcept
		{
			return false;
		}
		template<typename ElementType = ContainedType, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		Overlaps(const ArrayView<const ContainedType, SizeType, IndexType, const StoredType>) const noexcept
		{
			return false;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubView(const SizeType, SizeType) const noexcept
		{
			return {};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubViewFrom(const IteratorType) const noexcept
		{
			return {};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubViewFrom(const SizeType) const noexcept
		{
			return {};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubViewUpTo(const IteratorType) const noexcept
		{
			return {};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView GetSubViewUpTo(const SizeType) const noexcept
		{
			return {};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView operator+(SizeType) const noexcept
		{
			return {};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DynamicView operator-(const SizeType) const noexcept
		{
			return {};
		}
		template<typename OtherType, typename OtherSizeType, typename OtherIndexType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator==(const ArrayView<OtherType, OtherSizeType, OtherIndexType> other) const
		{
			return other.IsEmpty();
		}
		template<typename OtherType, typename OtherSizeType, typename OtherIndexType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator!=(const ArrayView<OtherType, OtherSizeType, OtherIndexType> other) const
		{
			return !operator==(other);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator==(const FixedArrayView other) const
		{
			return this->operator==(ConstDynamicView(other));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator!=(const FixedArrayView other) const
		{
			return !operator==(other);
		}

		template<typename ElementType = ContainedType, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		FORCE_INLINE bool CopyFrom(const ArrayView<const ContainedType, SizeType, IndexType, const StoredType> other) const noexcept
		{
			return other.IsEmpty();
		}

		template<typename ElementType = ContainedType, typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		FORCE_INLINE bool CopyFromWithOverlap(const ArrayView<const ContainedType, SizeType, IndexType, const StoredType> other) const noexcept
		{
			return other.IsEmpty();
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<TypeTraits::IsDefaultConstructible<ElementType> && !TypeTraits::IsConst<ElementType>>
		DefaultConstruct() const noexcept
		{
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<!TypeTraits::IsConst<ElementType>> InitializeAll() const noexcept
		{
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<!TypeTraits::IsConst<ElementType>> ZeroInitialize() const noexcept
		{
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<!TypeTraits::IsConst<ElementType>> CopyConstruct(const DynamicView) const noexcept
		{
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<!TypeTraits::IsConst<ElementType>> MoveConstruct(const DynamicView) const noexcept
		{
		}
	};
}
