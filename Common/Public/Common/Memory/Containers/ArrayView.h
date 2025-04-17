#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>
#include <Common/Platform/IsConstantEvaluated.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Assert/Assert.h>
#include <Common/TypeTraits/IsConst.h>
#include <Common/TypeTraits/WithoutConst.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/UnderlyingType.h>
#include <Common/TypeTraits/ReturnType.h>
#include <Common/TypeTraits/IsTriviallyCopyable.h>
#include <Common/TypeTraits/IsDefaultConstructible.h>
#include <Common/TypeTraits/IsMoveConstructible.h>
#include <Common/TypeTraits/IsCopyConstructible.h>
#include <Common/TypeTraits/IsConvertibleTo.h>
#include <Common/TypeTraits/IsPrimitive.h>
#include <Common/TypeTraits/IsBaseOf.h>
#include <Common/TypeTraits/IsEqualityComparable.h>
#include <Common/TypeTraits/IsTriviallyDestructible.h>
#include <Common/TypeTraits/Select.h>
#include <Common/Memory/Move.h>
#include <Common/Memory/Forward.h>
#include <Common/Memory/Copy.h>
#include <Common/Memory/Set.h>
#include <Common/Memory/OptionalIterator.h>
#include <Common/Memory/AddressOf.h>
#include <Common/Memory/IsAligned.h>
#include <Common/Memory/Containers/ForwardDeclarations/ArrayView.h>
#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Math/Min.h>
#include <Common/Math/Max.h>

namespace ngine
{
	namespace Internal
	{
		template<bool Restrict, typename TypeIn>
		struct RestrictedPointer
		{
			using Type = TypeIn*;
		};

		template<typename TypeIn>
		struct RestrictedPointer<true, TypeIn>
		{
			using Type = TypeIn* __restrict;
		};

		template<bool Restrict, typename TypeIn>
		struct RestrictedReference
		{
			using Type = TypeIn&;
		};

		template<typename TypeIn>
		struct RestrictedReference<true, TypeIn>
		{
			using Type = TypeIn& __restrict;
		};
	}

	template<typename ContainedType, typename InternalSizeType, typename InternalIndexType, typename InternalStoredType, uint8 Flags_>
	struct TRIVIAL_ABI ArrayView
	{
		inline static constexpr uint8 Flags = Flags_;

		inline static constexpr bool IsRestricted = (Flags & (uint8)ArrayViewFlags::Restrict) != 0;
		using StoredType = InternalStoredType;
		using PointerType = typename Internal::RestrictedPointer<IsRestricted, StoredType>::Type;
		using ConstPointerType = typename Internal::RestrictedPointer<IsRestricted, const StoredType>::Type;
		using StoredPointerType = typename Internal::RestrictedPointer<IsRestricted, StoredType>::Type;
		using ConstStoredPointerType = typename Internal::RestrictedPointer<IsRestricted, const StoredType>::Type;
		using ReferenceType = typename Internal::RestrictedReference<IsRestricted, ContainedType>::Type;

		using SizeType = InternalSizeType;
		using DataSizeType = size; // Memory::NumericSize<Math::NumericLimits<SizeType>::Max * sizeof(ContainedType)>;
		using IndexType = InternalIndexType;

		static_assert(sizeof(SizeType) == sizeof(IndexType));

		using View = ArrayView<ContainedType, SizeType, IndexType, StoredType, Flags>;
		using ConstView = ArrayView<const ContainedType, SizeType, IndexType, const StoredType, Flags>;
		using RestrictedView = ArrayView<ContainedType, SizeType, IndexType, StoredType, (uint8)ArrayViewFlags::Restrict>;
		using ConstRestrictedView = ArrayView<const ContainedType, SizeType, IndexType, StoredType, (uint8)ArrayViewFlags::Restrict>;

		template<typename Type>
		using Iterator = ngine::Iterator<Type, false>;
		template<typename Type>
		using ReverseIterator = ngine::Iterator<Type, true>;

		using IteratorType = Iterator<StoredType>;
		using ReverseIteratorType = ReverseIterator<StoredType>;
		using ConstIteratorType = Iterator<const StoredType>;
		using ConstReverseIteratorType = ReverseIterator<const StoredType>;
		using iterator = IteratorType;
		using reverse_iterator = ReverseIteratorType;
		using const_iterator = ConstIteratorType;
		using const_reverse_iterator = ConstReverseIteratorType;

		using OptionalIteratorType = Optional<IteratorType>;
		using OptionalReverseIteratorType = Optional<ReverseIteratorType>;
		using OptionalConstIteratorType = Optional<ConstIteratorType>;
		using OptionalConstReverseIteratorType = Optional<ConstReverseIteratorType>;

		FORCE_INLINE constexpr ArrayView() = default;

		template<size Size>
		FORCE_INLINE constexpr ArrayView(ContainedType (&element)[Size]) noexcept
			: m_pBegin(element)
			, m_size(Size)
		{
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
		}
		template<size Size>
		FORCE_INLINE constexpr ArrayView& operator=(ContainedType (&element)[Size]) noexcept
		{
			m_pBegin = element;
			m_size = Size;
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
			return *this;
		}

		FORCE_INLINE explicit constexpr ArrayView(ReferenceType element) noexcept
			: m_pBegin(Memory::GetAddressOf(element))
			, m_size(1u)
		{
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
		}
		FORCE_INLINE explicit constexpr ArrayView(PointerType pElement) noexcept
			: m_pBegin(pElement)
			, m_size(1u)
		{
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
		}

		FORCE_INLINE constexpr ArrayView(const IteratorType pBegin, const ConstIteratorType pEnd) noexcept
			: m_pBegin(pBegin)
			, m_size(static_cast<SizeType>(pEnd - pBegin))
		{
			// Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
		}

		FORCE_INLINE constexpr ArrayView(const IteratorType pBegin, const SizeType count) noexcept
			: m_pBegin(pBegin)
			, m_size(count)
		{
			// Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
		}

		template<
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename ElementType = ContainedType,
			typename StoredElementType = StoredType,
			typename = EnableIf<TypeTraits::IsConst<ElementType>>>
		FORCE_INLINE constexpr ArrayView(const ArrayView<
																		 typename TypeTraits::WithoutConst<ElementType>,
																		 OtherSizeType,
																		 OtherIndexType,
																		 typename TypeTraits::WithoutConst<StoredElementType>,
																		 Flags>& otherView) noexcept
			: m_pBegin(otherView.GetData())
			, m_size((SizeType)otherView.GetSize())
		{
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
		}
		template<
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename ElementType = ContainedType,
			typename StoredElementType = StoredType,
			typename = EnableIf<TypeTraits::IsConst<ElementType>>>
		FORCE_INLINE constexpr ArrayView& operator=(const ArrayView<
																								typename TypeTraits::WithoutConst<ElementType>,
																								OtherSizeType,
																								OtherIndexType,
																								typename TypeTraits::WithoutConst<StoredElementType>,
																								Flags> otherView) noexcept
		{
			m_pBegin = otherView.m_pBegin;
			m_size = (SizeType)otherView.m_size;
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
			return *this;
		}
		template<
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename ElementType = ContainedType,
			typename StoredElementType = StoredType,
			typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		FORCE_INLINE constexpr ArrayView&
		operator=(const ArrayView<const ElementType, OtherSizeType, OtherIndexType, typename TypeTraits::WithoutConst<StoredElementType>, Flags>
		            otherView) noexcept
		{
			m_pBegin = otherView.m_pBegin;
			m_size = otherView.m_size;
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
			return *this;
		}

		template<typename OtherSizeType = SizeType, typename OtherIndexType = IndexType>
		FORCE_INLINE constexpr ArrayView(const ArrayView<ContainedType, OtherSizeType, OtherIndexType, StoredType, Flags>& otherView) noexcept
			: m_pBegin(otherView.GetData())
			, m_size(static_cast<SizeType>(otherView.GetSize()))
		{
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
		}
		template<typename OtherSizeType = SizeType, typename OtherIndexType = IndexType>
		FORCE_INLINE constexpr ArrayView& operator=(const ArrayView<ContainedType, OtherSizeType, OtherIndexType, StoredType, Flags> otherView
		) noexcept
		{
			m_pBegin = otherView.m_pBegin;
			m_size = static_cast<SizeType>(otherView.GetSize());
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
			return *this;
		}

		template<
			typename OtherElementType,
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename OtherStoredType = OtherElementType,
			typename ElementType = ContainedType,
			typename = EnableIf<
				TypeTraits::IsBaseOf<ElementType, OtherElementType> &&
				!TypeTraits::IsSame<TypeTraits::WithoutConst<ElementType>, TypeTraits::WithoutConst<OtherElementType>> &&
				sizeof(ElementType) == sizeof(OtherElementType) && alignof(ElementType) == alignof(OtherElementType)>>
		FORCE_INLINE constexpr ArrayView(const ArrayView<OtherElementType, OtherSizeType, OtherIndexType, OtherStoredType, Flags> otherView
		) noexcept
			: m_pBegin(otherView.GetData())
			, m_size(static_cast<SizeType>(otherView.GetSize()))
		{
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
		}
		template<
			typename OtherElementType,
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename OtherStoredType = OtherElementType,
			typename ElementType = ContainedType,
			typename = EnableIf<
				TypeTraits::IsBaseOf<ElementType, OtherElementType> &&
				!TypeTraits::IsSame<TypeTraits::WithoutConst<ElementType>, TypeTraits::WithoutConst<OtherElementType>> &&
				sizeof(ElementType) == sizeof(OtherElementType) && alignof(ElementType) == alignof(OtherElementType)>>
		FORCE_INLINE constexpr ArrayView&
		operator=(const ArrayView<OtherElementType, OtherSizeType, OtherIndexType, OtherStoredType, Flags> otherView) noexcept
		{
			m_pBegin = otherView.m_pBegin;
			m_size = static_cast<SizeType>(otherView.GetSize());
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
			return *this;
		}

		template<
			typename OtherElementType,
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename ElementType = ContainedType,
			typename = EnableIf<
				TypeTraits::IsBaseOf<ElementType, OtherElementType> &&
				(sizeof(ElementType) != sizeof(OtherElementType) || alignof(ElementType) != alignof(OtherElementType))>>
		FORCE_INLINE constexpr ArrayView(const ArrayView<OtherElementType, OtherSizeType, OtherIndexType, StoredType, Flags> otherView) noexcept
			: m_pBegin(otherView.GetData())
			, m_size(static_cast<SizeType>(otherView.GetSize()))
		{
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
		}
		template<
			typename OtherElementType,
			typename OtherSizeType,
			typename OtherIndexType = OtherSizeType,
			typename ElementType = ContainedType,
			typename = EnableIf<
				TypeTraits::IsBaseOf<ElementType, OtherElementType> &&
				(sizeof(ElementType) != sizeof(OtherElementType) || alignof(ElementType) != alignof(OtherElementType))>>
		FORCE_INLINE constexpr ArrayView&
		operator=(const ArrayView<OtherElementType, OtherSizeType, OtherIndexType, StoredType, Flags> otherView) noexcept
		{
			m_pBegin = otherView.m_pBegin;
			m_size = static_cast<SizeType>(otherView.GetSize());
			Assert(!IsConstantEvaluated() && Memory::IsAligned(m_pBegin, alignof(ContainedType)));
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS operator RestrictedView() const noexcept
		{
			return {GetData(), end()};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetSize() const noexcept
		{
			return static_cast<SizeType>(GetIteratorIndex(end()));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsEmpty() const noexcept
		{
			return m_pBegin == end();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool HasElements() const noexcept
		{
			return !IsEmpty();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DataSizeType GetDataSize() const noexcept
		{
			return sizeof(StoredType) * GetSize();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr PointerType GetData() const noexcept
		{
			return m_pBegin;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ReferenceType operator[](const IndexType index) const noexcept
		{
			StoredPointerType const pBegin = static_cast<StoredPointerType>(m_pBegin);
			Assert((SizeType)index < GetSize());
			return pBegin[(SizeType)index];
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
			return static_cast<StoredPointerType>(m_pBegin);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr IteratorType end() const noexcept
		{
			return static_cast<StoredPointerType>(m_pBegin) + m_size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ReverseIteratorType rbegin() const noexcept
		{
			return static_cast<StoredPointerType>(m_pBegin + m_size - 1);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ReverseIteratorType rend() const noexcept
		{
			return static_cast<StoredPointerType>(m_pBegin - 1);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsValidIndex(const IndexType index) const noexcept
		{
			return index < m_size;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr IndexType GetIteratorIndex(const ConstPointerType it) const noexcept
		{
			Assert((it >= begin().Get()) & (it <= end().Get()));
			return static_cast<IndexType>(it - begin().Get());
		}

		template<
			typename ComparableType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsEqualityComparable<const ElementType, const ComparableType>>>
		[[nodiscard]] PURE_STATICS OptionalIteratorType Find(const ComparableType& element) const noexcept
		{
			StoredPointerType it = begin();
			const StoredPointerType endIt = end();
			for (; it != endIt; ++it)
			{
				if (*it == element)
				{
					return OptionalIteratorType{it, endIt};
				}
			}

			return OptionalIteratorType(endIt, endIt);
		}

		template<
			typename ComparableType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsEqualityComparable<const ElementType, const ComparableType>>>
		[[nodiscard]] PURE_STATICS OptionalIteratorType FindLastOf(const ComparableType& element) const noexcept
		{
			StoredPointerType it = end() - 1;
			StoredPointerType lastIt = begin() - 1;

			for (; it != lastIt; --it)
			{
				if (*it == element)
				{
					return OptionalIteratorType{it, lastIt};
				}
			}

			return OptionalIteratorType(lastIt, lastIt);
		}

		template<
			typename ComparableType,
			typename ElementType = ContainedType,
			typename = EnableIf<TypeTraits::IsEqualityComparable<const ElementType, const ComparableType>>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const ComparableType& element) const noexcept
		{
			return Find(element).IsValid();
		}

		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] PURE_STATICS constexpr View
		FindFirstRange(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> other) const
		{
			if (GetSize() < other.GetSize())
			{
				return {};
			}

			SizeType index = 0;
			for (const ContainedType& element : *this)
			{
				if (element == other[index])
				{
					if (++index == other.GetSize())
					{
						return {begin() + GetIteratorIndex(Memory::GetAddressOf(element)) - other.GetSize() + 1, other.GetSize()};
					}
				}
				else
				{
					index = 0;
				}
			}

			return {};
		}

		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr View
		FindLastRange(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> other) const
		{
			if (GetSize() < other.GetSize())
			{
				return {};
			}

			StoredPointerType it = end() - 1;
			StoredPointerType lastIt = Math::Max(begin() - 1, begin() - other.GetSize());

			const ElementType& lastElement = other.GetLastElement();
			for (; it != lastIt; --it)
			{
				if (*it == lastElement)
				{
					View view{it - (other.GetSize() - 1), it + 1};
					if (view == other)
					{
						return view;
					}
				}
			}

			return {};
		}

		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] PURE_STATICS constexpr bool
		ContainsRange(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> other) const
		{
			return FindFirstRange(other).HasElements();
		}

		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] PURE_STATICS bool
		ContainsAny(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> elements) const noexcept
		{
			for (const ElementType& element : elements)
			{
				if (Contains(element))
				{
					return true;
				}
			}

			return false;
		}

		template<typename Callback>
		[[nodiscard]] PURE_STATICS OptionalIteratorType FindIf(const Callback callback) const noexcept
		{
			PointerType it = GetData(), endIt = end();
			for (; it != endIt; ++it)
			{
				if (callback(*it))
				{
					return OptionalIteratorType{it, endIt};
				}
			}

			return OptionalIteratorType(endIt, endIt);
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool ContainsIf(const Callback callback) const noexcept
		{
			return FindIf(callback).IsValid();
		}

		template<typename Callback>
		void ForEach(const Callback callback) noexcept
		{
			for (ConstPointerType it = GetData(), endIt = end(); it != endIt; ++it)
			{
				callback(*it);
			}
		}

		template<typename Callback>
		[[nodiscard]] PURE_STATICS bool All(const Callback callback) const noexcept
		{
			for (ConstPointerType it = GetData(), endIt = end(); it != endIt; ++it)
			{
				if (!callback(*it))
				{
					return false;
				}
			}

			return true;
		}

		template<typename Callback>
		[[nodiscard]] PURE_STATICS bool Any(const Callback callback) const noexcept
		{
			for (ConstPointerType it = GetData(), endIt = end(); it != endIt; ++it)
			{
				if (callback(*it))
				{
					return true;
				}
			}

			return false;
		}

		template<typename Callback>
		[[nodiscard]] PURE_STATICS ConstView FindFirstContiguousRange(const Callback callback) const noexcept
		{
			for (ConstPointerType it = GetData(), endIt = end(); it != endIt; ++it)
			{
				if (callback(*it))
				{
					ConstPointerType beginIt = it;
					for (++it; it < endIt; ++it)
					{
						if (!callback(*it))
						{
							return ConstView{beginIt, static_cast<SizeType>(it - beginIt)};
						}
					}
					return ConstView{beginIt, static_cast<SizeType>(endIt - beginIt)};
				}
			}
			return {};
		}

		template<typename Callback>
		[[nodiscard]] PURE_STATICS OptionalIteratorType LastFromStart(const Callback callback) const noexcept
		{
			PointerType it = GetData(), endIt = end();
			for (; it != endIt; ++it)
			{
				if (!callback(*it))
				{
					return OptionalIteratorType(it, endIt);
				}
			}

			return OptionalIteratorType(it - 1, endIt);
		}

		template<typename Callback>
		[[nodiscard]] PURE_STATICS auto Count(const Callback callback) const noexcept
		{
			using ReturnType = TypeTraits::ReturnType<Callback>;
			using CountType = TypeTraits::Select<TypeTraits::IsSame<ReturnType, bool>, SizeType, ReturnType>;
			CountType count = 0;
			for (ConstPointerType it = GetData(), endIt = end(); it != endIt; ++it)
			{
				count += callback(*it);
			}
			return count;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsWithinBounds(const ConstPointerType it) const noexcept
		{
			return (it >= GetData()) & (it < end());
		}
		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		IsWithinBounds(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> otherView) const noexcept
		{
			return (otherView.GetData() >= GetData()) & (otherView.end() <= end()) & otherView.HasElements() & HasElements();
		}

		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ArrayView
		Mask(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> otherView) const noexcept
		{
#if PLATFORM_APPLE
			// Intentionally not using Math::Max as it can break compilation with Apple Clang ARC for __strong types
			ContainedType* startIt = m_pBegin;
			ContainedType* otherStartIt = const_cast<ContainedType*>(otherView.begin().Get());
			startIt = startIt >= otherStartIt ? startIt : otherStartIt;

			ContainedType* endIt = end();
			ContainedType* otherEndIt = const_cast<ContainedType*>(otherView.end().Get());
			endIt = endIt <= otherEndIt ? endIt : otherEndIt;
#else
			ContainedType* const startIt =
				Math::Max(const_cast<ContainedType*>(otherView.begin().Get()), const_cast<ContainedType*>(begin().Get()));
			ContainedType* const endIt = Math::Min(const_cast<ContainedType*>(otherView.end().Get()), const_cast<ContainedType*>(end().Get()));
#endif

			return {startIt, SizeType(SizeType(endIt - startIt) * (endIt >= startIt))};
		}

		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		Overlaps(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> otherView) const noexcept
		{
			return Mask(otherView).HasElements();
		}

		template<
			typename ElementType,
			typename OtherSizeType,
			typename OtherIndexType,
			typename OtherStoredType,
			uint8 OtherFlags,
			typename = EnableIf<!TypeTraits::IsConst<ElementType>>>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		Contains(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> otherView) const noexcept
		{
			return (otherView.GetData() >= GetData()) & (otherView.end() <= end());
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ArrayView GetSubView(SizeType index, SizeType count) const noexcept
		{
			index = Math::Min(index, SizeType(Math::Max(m_size, SizeType(1u)) - 1ull));
			count = Math::Min(SizeType(m_size - index), count);
			Assert(m_pBegin + index + count <= end() || count == 0);
			return {m_pBegin + index, count};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ArrayView GetSubViewFrom(const IteratorType it) const noexcept
		{
			return {(PointerType)it, Math::Max((PointerType)it, (PointerType)end())};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ArrayView GetSubViewFrom(const SizeType index) const noexcept
		{
			return GetSubViewFrom(begin() + index);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ArrayView GetSubViewUpTo(const IteratorType it) const noexcept
		{
			return {Math::Min((StoredPointerType)it, (StoredPointerType)begin()), Math::Min((StoredPointerType)it, (StoredPointerType)end())};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ArrayView GetSubViewUpTo(const SizeType index) const noexcept
		{
			return GetSubViewUpTo(begin() + index);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ArrayView operator+(SizeType offset) const noexcept
		{
			offset = Math::Min(offset, m_size);
			return {begin() + offset, SizeType(m_size - offset)};
		}
		FORCE_INLINE constexpr ArrayView& operator+=(SizeType offset) noexcept
		{
			offset = Math::Min(offset, m_size);
			static_cast<StoredPointerType&>(m_pBegin) += offset;
			m_size -= offset;
			return *this;
		}
		FORCE_INLINE constexpr ArrayView& operator++() noexcept
		{
			return operator+=(1u);
		}
		FORCE_INLINE constexpr ArrayView& operator++(int) noexcept
		{
			return operator+=(1u);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ArrayView operator-(const SizeType offset) const noexcept
		{
			return {m_pBegin, SizeType(m_size - Math::Min(offset, m_size))};
		}
		FORCE_INLINE constexpr ArrayView& operator-=(const SizeType offset) noexcept
		{
			m_size -= Math::Min(offset, m_size);
			return *this;
		}
		FORCE_INLINE constexpr ArrayView& operator--() noexcept
		{
			return operator-=(1u);
		}
		FORCE_INLINE constexpr ArrayView& operator--(int) noexcept
		{
			return operator-=(1u);
		}
		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] PURE_STATICS constexpr inline bool
		operator==(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> otherView) const
		{
			if (GetSize() != otherView.GetSize())
			{
				return false;
			}

			for (SizeType i = 0, n = GetSize(); i < n; ++i)
			{
				if (this->operator[](i) != otherView[(OtherSizeType)i])
				{
					return false;
				}
			}

			return true;
		}
		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool
		operator!=(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> otherView) const
		{
			return !operator==(otherView);
		}

		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		FORCE_INLINE bool CopyFrom(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> other
		) const noexcept
		{
			Assert(!Overlaps(other));
			Assert((m_pBegin != nullptr) | IsEmpty());
			const size copiedElementCount = Math::Min(GetSize(), other.GetSize());
			Memory::CopyNonOverlappingElements<StoredType>(m_pBegin, other.GetData(), copiedElementCount);
			return copiedElementCount == other.GetSize();
		}

		template<typename ElementType, typename OtherSizeType, typename OtherIndexType, typename OtherStoredType, uint8 OtherFlags>
		FORCE_INLINE bool CopyFromWithOverlap(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> other
		) const noexcept
		{
			Assert((m_pBegin != nullptr) | IsEmpty());
			const size copiedElementCount = Math::Min(GetSize(), other.GetSize());
			Memory::CopyOverlappingElements(m_pBegin, other.GetData(), copiedElementCount);
			return copiedElementCount == other.GetSize();
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<TypeTraits::IsDefaultConstructible<ElementType> && !TypeTraits::IsConst<ElementType>>
		DefaultConstruct() const noexcept
		{
			if constexpr (TypeTraits::IsPrimitive<ElementType>)
			{
				ZeroInitialize();
			}
			else
			{
				for (ReferenceType element : *this)
				{
					new (Memory::GetAddressOf(element)) StoredType();
				}
			}
		}

		template<typename... ConstructArgs, typename ElementType = ContainedType>
		FORCE_INLINE constexpr EnableIf<!TypeTraits::IsConst<ElementType>> InitializeAll(ConstructArgs&&... args) const noexcept
		{
			for (ReferenceType element : *this)
			{
				element = StoredType(Forward<ConstructArgs>(args)...);
			}
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<!TypeTraits::IsConst<ElementType>> ZeroInitialize() const noexcept
		{
			Assert((m_pBegin != nullptr) | IsEmpty());
			Memory::Set(GetData(), 0, GetDataSize());
		}

		template<typename ElementType = ContainedType>
		FORCE_INLINE EnableIf<!TypeTraits::IsConst<ElementType>> OneInitialize() const noexcept
		{
			Assert((m_pBegin != nullptr) | IsEmpty());
			Memory::Set(GetData(), ~0, GetDataSize());
		}

		FORCE_INLINE void DestroyElements() noexcept
		{
			if constexpr (!TypeTraits::IsTriviallyDestructible<ContainedType>)
			{
				for (ContainedType& element : *this)
				{
					element.~ContainedType();
				}
			}
		}

		template<
			typename ElementType,
			typename OtherSizeType,
			typename OtherIndexType,
			typename OtherStoredType,
			uint8 OtherFlags,
			typename ThisElementType = ContainedType>
		FORCE_INLINE EnableIf<
			!TypeTraits::IsConst<ThisElementType> &&
			(TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsMoveConstructible<ElementType>)>
		CopyConstruct(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> other) const noexcept
		{
			Assert(GetSize() == other.GetSize());
			Assert(!Overlaps(other) | IsEmpty());
			if constexpr (TypeTraits::IsTriviallyCopyable<ElementType>)
			{
				CopyFrom(other);
			}
			else
			{
				PointerType pElement = m_pBegin;
				for (const ContainedType& otherElement : other)
				{
					new (pElement) StoredType(otherElement);
					++pElement;
				}
			}
		}

		template<
			typename ElementType,
			typename OtherSizeType,
			typename OtherIndexType,
			typename OtherStoredType,
			uint8 OtherFlags,
			typename ThisElementType = ContainedType>
		FORCE_INLINE EnableIf<
			!TypeTraits::IsConst<ThisElementType> &&
			(TypeTraits::IsTriviallyCopyable<ElementType> || TypeTraits::IsMoveConstructible<ElementType>)>
		MoveConstruct(const ArrayView<ElementType, OtherSizeType, OtherIndexType, OtherStoredType, OtherFlags> other) const noexcept
		{
			Assert(GetSize() == other.GetSize());
			Assert(!Overlaps(other) | IsEmpty());
			if constexpr (TypeTraits::IsTriviallyCopyable<ElementType>)
			{
				CopyFrom(other);
				other.ZeroInitialize();
			}
			else
			{
				PointerType pElement = m_pBegin;
				for (auto& otherElement : other)
				{
					new (pElement) StoredType(Move(otherElement));
					++pElement;
				}
			}
		}

		template<
			typename ElementType = ContainedType,
			bool IsMoveConstructible = TypeTraits::IsMoveConstructible<ElementType>,
			typename... Args>
		FORCE_INLINE
			EnableIf<!TypeTraits::IsConst<ElementType> && IsMoveConstructible /* && (TypeTraits::IsConvertibleTo<Args, ElementType> && ...)*/>
			MoveConstructAll(Args&&... args) const noexcept
		{
			Assert(sizeof...(args) <= GetSize());
			StoredPointerType pThisElement = GetData();
			(MoveConstructSingle(pThisElement, Forward<Args>(args)), ...);
		}

		template<
			typename ElementType = ContainedType,
			bool IsCopyConstructible = TypeTraits::IsCopyConstructible<ElementType>,
			typename... Args>
		FORCE_INLINE
			EnableIf<!TypeTraits::IsConst<ElementType> && IsCopyConstructible /* && (TypeTraits::IsConvertibleTo<Args, ElementType> && ...)*/>
			CopyConstructAll(const Args&... args) const noexcept
		{
			Assert(sizeof...(args) <= GetSize());
			StoredPointerType pThisElement = GetData();
			(CopyConstructSingle(pThisElement, args), ...);
		}

		[[nodiscard]] constexpr uint32 CalculateCompressedDataSize() const;
		bool Compress(BitView& target) const;

		template<typename... Args>
		bool Serialize(const Serialization::Reader serializer, Args&... args);
		template<typename... Args>
		bool Serialize(Serialization::Writer serializer, Args&... args) const;
	protected:
		template<typename ElementType = StoredType, bool IsMoveConstructible = TypeTraits::IsMoveConstructible<ElementType>>
		FORCE_INLINE static EnableIf<IsMoveConstructible> MoveConstructSingle(PointerType& pElement, StoredType&& element) noexcept
		{
			new (pElement) StoredType(Move(element));
			pElement++;
		}

		template<typename ElementType = StoredType, bool IsCopyConstructible = TypeTraits::IsCopyConstructible<ElementType>>
		FORCE_INLINE static EnableIf<IsCopyConstructible> CopyConstructSingle(PointerType& pElement, const StoredType& element) noexcept
		{
			new (pElement) StoredType(element);
			pElement++;
		}
	protected:
		template<typename OtherContainedType, typename OtherSizeType, typename OtherStoredType, typename OtherIndexType, uint8 OtherFlags>
		friend struct ArrayView;

		ContainedType* m_pBegin = nullptr;
		SizeType m_size = 0u;
	};

	template<typename IteratorType>
	ArrayView(IteratorType*, IteratorType*) -> ArrayView<IteratorType>;
}
