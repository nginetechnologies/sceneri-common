#pragma once

#include <Common/Platform/Unused.h>
#include <Common/Platform/LifetimeBound.h>
#include <Common/Memory/Containers/ContainerCommon.h>
#include <Common/Memory/New.h>
#include <Common/Memory/AddressOf.h>
#include <Common/Memory/Containers/ForwardDeclarations/BitView.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Math/NumericLimits.h>
#include <Common/Math/Max.h>
#include <Common/Math/Min.h>
#include <Common/TypeTraits/IsTriviallyConstructible.h>
#include <Common/TypeTraits/IsTriviallyDestructible.h>
#include "ArrayView.h"

#include "ForwardDeclarations/VectorBase.h"
#include <Common/Memory/Containers/VectorFlags.h>

namespace ngine
{
	enum class ErasePredicateResult : uint8
	{
		Continue,
		Remove
	};

	namespace Internal
	{
		// Helper to make sure that TypeTraits::IsTriviallyDestructible<ContainedType> is only evaluated for trivially destructible allocators
		// Needed to make sure that Vector<T> can be used as a member of T, but disallowed for FlatVector<T>
		template<bool IsAllocatorTriviallyDestructible, typename ContainedType>
		struct TIsTriviallyDestructible
		{
			inline static constexpr bool Value = TypeTraits::IsTriviallyDestructible<ContainedType>;
		};
		template<typename ContainedType>
		struct TIsTriviallyDestructible<false, ContainedType>
		{
			inline static constexpr bool Value = false;
		};

		template<bool IsTriviallyDestructible, typename ContainedType, typename AllocatorType, uint8 Flags>
		struct VectorDestructorHelper
		{
			using SizeType = typename AllocatorType::SizeType;
			explicit VectorDestructorHelper(const Memory::ReserveType type, const SizeType capacity) noexcept
				: m_allocator(type, capacity)
			{
			}
			VectorDestructorHelper() = default;
			VectorDestructorHelper(VectorDestructorHelper&&) = default;
			VectorDestructorHelper(const VectorDestructorHelper&) = default;
			VectorDestructorHelper& operator=(VectorDestructorHelper&&) = default;
			VectorDestructorHelper& operator=(const VectorDestructorHelper&) = default;

			~VectorDestructorHelper()
			{
				if constexpr (!TypeTraits::IsTriviallyDestructible<ContainedType>)
				{
					for (ContainedType& element : static_cast<TVector<ContainedType, AllocatorType, Flags>&>(*this).GetView())
					{
						element.~ContainedType();
					}
				}
			}
		protected:
			AllocatorType m_allocator;
		};

		template<typename ContainedType, typename AllocatorType, uint8 Flags>
		struct VectorDestructorHelper<true, ContainedType, AllocatorType, Flags>
		{
			using SizeType = typename AllocatorType::SizeType;
			explicit VectorDestructorHelper(const Memory::ReserveType type, const SizeType capacity) noexcept
				: m_allocator(type, capacity)
			{
			}
			VectorDestructorHelper() = default;
			VectorDestructorHelper(VectorDestructorHelper&&) = default;
			VectorDestructorHelper(const VectorDestructorHelper&) = default;
			VectorDestructorHelper& operator=(VectorDestructorHelper&&) = default;
			VectorDestructorHelper& operator=(const VectorDestructorHelper&) = default;
		protected:
			AllocatorType m_allocator;
		};
	}

	template<typename ContainedType, typename AllocatorType, uint8 Flags>
	struct TVector : public Internal::VectorDestructorHelper<
										 Internal::TIsTriviallyDestructible<TypeTraits::IsTriviallyDestructible<AllocatorType>, ContainedType>::Value,
										 ContainedType,
										 AllocatorType,
										 Flags>
	{
	protected:
		inline static constexpr bool SupportResize = (Flags & Memory::VectorFlags::AllowResize) != 0;
		inline static constexpr bool SupportReallocate = (Flags & Memory::VectorFlags::AllowReallocate) != 0;
	public:
		using BaseType = Internal::VectorDestructorHelper<
			Internal::TIsTriviallyDestructible<TypeTraits::IsTriviallyDestructible<AllocatorType>, ContainedType>::Value,
			ContainedType,
			AllocatorType,
			Flags>;
		using SizeType = typename AllocatorType::SizeType;
		using DataSizeType = typename AllocatorType::DataSizeType;
		using IndexType = typename AllocatorType::IndexType;

		using View = ArrayView<ContainedType, SizeType, IndexType, ContainedType>;
		using ConstView = ArrayView<const ContainedType, SizeType, IndexType, const ContainedType>;
		using RestrictedView = typename View::RestrictedView;
		using RestrictedConstView = typename ConstView::RestrictedView;

		using PointerType = typename View::PointerType;
		using ConstPointerType = typename View::ConstPointerType;
		using ReferenceType = typename View::ReferenceType;
		using IteratorType = typename View::IteratorType;
		using ReverseIteratorType = typename View::ReverseIteratorType;
		using ConstIteratorType = typename View::ConstIteratorType;
		using ConstReverseIteratorType = typename View::ConstReverseIteratorType;
		using iterator = typename View::iterator;
		using reverse_iterator = typename View::reverse_iterator;
		using const_iterator = typename View::const_iterator;
		using const_reverse_iterator = typename View::const_reverse_iterator;

		using OptionalIteratorType = typename View::OptionalIteratorType;
		using OptionalReverseIteratorType = typename View::OptionalReverseIteratorType;
		using OptionalConstIteratorType = typename View::OptionalConstIteratorType;
		using OptionalConstReverseIteratorType = typename View::OptionalConstReverseIteratorType;

		TVector() = default;
		using BaseType::BaseType;
		template<typename ElementType = ContainedType, typename = EnableIf<TypeTraits::IsDefaultConstructible<ElementType>>>
		explicit TVector(const Memory::ConstructWithSizeType, const Memory::DefaultConstructType, const SizeType size) noexcept
			: BaseType(Memory::Reserve, size)
			, m_size(size)
		{
			GetView().DefaultConstruct();
		}
		explicit TVector(const Memory::ConstructWithSizeType, const Memory::UninitializedType, const SizeType size) noexcept
			: BaseType(Memory::Reserve, size)
			, m_size(size)
		{
		}
		explicit TVector(const Memory::ConstructWithSizeType, const Memory::ZeroedType, const SizeType size) noexcept
			: BaseType(Memory::Reserve, size)
			, m_size(size)
		{
			GetView().ZeroInitialize();
		}
		template<typename... Args>
		explicit TVector(const Memory::ConstructWithSizeType, const Memory::InitializeAllType, const SizeType size, Args&&... args) noexcept
			: BaseType(Memory::Reserve, size)
			, m_size(size)
		{
			Assert(size <= Math::NumericLimits<SizeType>::Max);
			GetView().InitializeAll(Forward<Args>(args)...);
		}
		template<
			typename... Args,
			typename ElementType = ContainedType,
			typename = EnableIf<(TypeTraits::IsConvertibleTo<Args, ElementType> && ...)>>
		TVector(Args&&... args) noexcept
			: BaseType(Memory::Reserve, static_cast<SizeType>(sizeof...(args)))
			, m_size(static_cast<SizeType>(sizeof...(args)))
		{
			static_assert(sizeof...(args) <= Math::NumericLimits<SizeType>::Max);
			GetView().MoveConstructAll(Forward<Args>(args)...);
		}
		template<
			typename... Args,
			typename ElementType = ContainedType,
			typename = EnableIf<(TypeTraits::IsConvertibleTo<Args, ElementType> && ...)>>
		TVector(const Args&... args) noexcept
			: BaseType(Memory::Reserve, static_cast<SizeType>(sizeof...(args)))
			, m_size(static_cast<SizeType>(sizeof...(args)))
		{
			static_assert(sizeof...(args) <= Math::NumericLimits<SizeType>::Max);
			GetView().CopyConstructAll(args...);
		}
		template<
			typename ArrayElementType,
			typename ArraySizeType,
			typename ElementType = ContainedType,
			typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		TVector(const ArrayView<ArrayElementType, ArraySizeType> view) noexcept
			: BaseType(Memory::Reserve, static_cast<SizeType>(view.GetSize()))
			, m_size(static_cast<SizeType>(view.GetSize()))
		{
			Assert(view.GetSize() <= Math::NumericLimits<SizeType>::Max);
			GetView().CopyConstruct(view);
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<TypeTraits::IsCopyConstructible<ElementType>>>
		explicit TVector(const TVector& other)
			: TVector(other.GetView())
		{
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsCopyConstructible<ElementType>)>>
		TVector& operator=(const TVector& other)
		{
			static_assert(TypeTraits::IsCopyConstructible<ContainedType>);
			if constexpr (SupportResize)
			{
				Clear();
			}
			else
			{
				Assert(m_size == other.m_size);
			}
			if constexpr (!SupportReallocate)
			{
				Assert(GetCapacity() >= other.m_size);
			}

			ReserveOrAssert(other.GetSize(), Memory::ReserveExact);
			m_size = other.GetSize();
			GetView().CopyConstruct(other.GetView());

			return *this;
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		TVector(TVector&& other) noexcept
			: BaseType(static_cast<BaseType&&>(other))
			, m_size(other.m_size)
		{
			other.m_size = 0;
			static_assert(TypeTraits::IsMoveConstructible<ContainedType>);
		}
		// template <typename ElementType = ContainedType, typename = EnableIf<(TypeTraits::IsMoveConstructible<ElementType>)>>
		TVector& operator=(TVector&& other) noexcept
		{
			static_assert(TypeTraits::IsMoveConstructible<ContainedType>);
			m_size = other.m_size;
			other.m_size = 0;
			BaseType::operator=(static_cast<BaseType&&>(other));
			return *this;
		}
		template<
			typename OtherSizeType,
			typename OtherIndexType,
			typename ElementType = ContainedType,
			typename = EnableIf<(TypeTraits::IsCopyConstructible<
													 ElementType>)>> //, typename ElementType = ContainedType, bool AllowResize = SupportResize, typename =
		                                       // EnableIf<(TypeTraits::IsCopyConstructible<ElementType> && AllowResize)>>
		TVector& operator=(const ArrayView<const ContainedType, OtherSizeType, OtherIndexType> view) noexcept
		{
			Clear();
			ReserveOrAssert(view.GetSize(), Memory::ReserveExact);
			m_size = view.GetSize();
			GetView().CopyConstruct(view);
			return *this;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS SizeType GetSize() const noexcept
		{
			return m_size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS IndexType GetNextAvailableIndex() const noexcept
		{
			return static_cast<IndexType>(m_size);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool IsEmpty() const noexcept
		{
			return m_size == 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool HasElements() const noexcept
		{
			return m_size > 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS DataSizeType GetDataSize() const noexcept
		{
			return sizeof(ContainedType) * m_size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS SizeType GetCapacity() const noexcept
		{
			return BaseType::m_allocator.GetCapacity();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS SizeType GetRemainingCapacity() const noexcept
		{
			return GetCapacity() - m_size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool ReachedCapacity() const noexcept
		{
			return m_size == GetCapacity();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetTheoreticalCapacity() const noexcept
		{
			return BaseType::m_allocator.GetTheoreticalCapacity();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool ReachedTheoreticalCapacity() const noexcept
		{
			return m_size == GetTheoreticalCapacity();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS operator View() noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS operator ConstView() const noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr View GetView() noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView GetView() const noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr View GetSubView(const SizeType index, const SizeType count) noexcept LIFETIME_BOUND
		{
			return GetView().GetSubView(index, count);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView
		GetSubView(const SizeType index, const SizeType count) const noexcept LIFETIME_BOUND
		{
			return GetView().GetSubView(index, count);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr View GetAllocatedView() noexcept LIFETIME_BOUND
		{
			return BaseType::m_allocator.GetView();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView GetAllocatedView() const noexcept LIFETIME_BOUND
		{
			return BaseType::m_allocator.GetView();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS PointerType GetData() noexcept LIFETIME_BOUND
		{
			return BaseType::m_allocator.GetData();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS ConstPointerType GetData() const noexcept LIFETIME_BOUND
		{
			return BaseType::m_allocator.GetData();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ContainedType& operator[](const IndexType index) noexcept LIFETIME_BOUND
		{
			Expect((SizeType)index < m_size);
			return *(GetData() + (SizeType)index);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const ContainedType& operator[](const IndexType index) const noexcept LIFETIME_BOUND
		{
			Expect((SizeType)index < m_size);
			return *(GetData() + (SizeType)index);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr IteratorType begin() noexcept LIFETIME_BOUND
		{
			return GetData();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstIteratorType begin() const noexcept LIFETIME_BOUND
		{
			return GetData();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr IteratorType end() noexcept LIFETIME_BOUND
		{
			return GetData() + m_size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstIteratorType end() const noexcept LIFETIME_BOUND
		{
			return GetData() + m_size;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ReverseIteratorType rbegin() noexcept LIFETIME_BOUND
		{
			return GetData() + m_size - 1;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstReverseIteratorType rbegin() const noexcept LIFETIME_BOUND
		{
			return GetData() + m_size - 1;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ReverseIteratorType rend() noexcept LIFETIME_BOUND
		{
			return GetData() - 1;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstReverseIteratorType rend() const noexcept LIFETIME_BOUND
		{
			return GetData() - 1;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsValidIndex(const IndexType index) const noexcept
		{
			return index < m_size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS IndexType GetIteratorIndex(const ConstPointerType it) const noexcept
		{
			return GetView().GetIteratorIndex(it);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsWithinBounds(const ConstPointerType it) const noexcept
		{
			return GetView().IsWithinBounds(it);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsWithinBounds(const ConstView otherView) const noexcept
		{
			return GetView().IsWithinBounds(otherView);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool Overlaps(const ConstView otherView) const noexcept
		{
			return GetView().Overlaps(otherView);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS ContainedType& GetLastElement() noexcept LIFETIME_BOUND
		{
			return GetView().GetLastElement();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS const ContainedType& GetLastElement() const noexcept LIFETIME_BOUND
		{
			return GetView().GetLastElement();
		}

		template<typename... Args, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		bool EmplaceBackUnique(ContainedType&& element) noexcept LIFETIME_BOUND
		{
			if (!Contains(element))
			{
				Assert(m_size != GetTheoreticalCapacity());
				ReserveOrAssert(m_size + 1, Memory::ReserveExponential);

				PointerType pElement = BaseType::m_allocator.GetData() + m_size;
				new (pElement) ContainedType(Forward<ContainedType>(element));
				m_size++;
				return true;
			}
			return false;
		}

		template<typename... Args, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		ContainedType& EmplaceBack(Args&&... args) noexcept LIFETIME_BOUND
		{
			Assert(m_size != GetTheoreticalCapacity());
			ReserveOrAssert(m_size + 1, Memory::ReserveExponential);

			PointerType pElement = BaseType::m_allocator.GetData() + m_size;
			new (pElement) ContainedType(Forward<Args&&>(args)...);
			m_size++;
			return *pElement;
		}

		template<
			typename... Args,
			bool AllowResize = SupportResize,
			typename ConstructType = Memory::DefaultConstructType,
			typename = EnableIf<AllowResize>>
		ContainedType& Emplace(const ConstPointerType whereIt, ConstructType, Args&&... args) noexcept LIFETIME_BOUND
		{
			Assert(whereIt >= begin().Get() || (whereIt == nullptr && IsEmpty()));
			const SizeType index = static_cast<SizeType>(whereIt - BaseType::m_allocator.GetData());

			ReserveOrAssert((SizeType)Math::Max(index + 1, m_size + 1), Memory::ReserveExponential);

			if constexpr (TypeTraits::IsSame<ConstructType, Memory::DefaultConstructType>)
			{
				if constexpr (TypeTraits::IsTriviallyConstructible<ContainedType> || TypeTraits::IsDefaultConstructible<ContainedType>)
				{
					BaseType::m_allocator.GetView()
						.GetSubView(m_size, (SizeType)Math::Max((int64)index - (int64)m_size, (int64)0))
						.DefaultConstruct();
				}
				else
				{
					Assert((index <= m_size) | (index == m_size), "Can not default construct elements!");
				}
			}
			else if constexpr (TypeTraits::IsSame<ConstructType, Memory::UninitializedType>)
				;
			else
			{
				static_unreachable("Invalid construct type");
			}

			// Copy existing elements at index
			{
				View target = BaseType::m_allocator.GetView() + index + 1;

				const SizeType numMovedElements = m_size - index;
				target.CopyFromWithOverlap(GetSubView(index, numMovedElements));
			}

			PointerType pData = BaseType::m_allocator.GetData() + index;
			new (pData) ContainedType(Forward<Args&&>(args)...);
			m_size = (SizeType)Math::Max(index + 1, m_size + 1);

			return *pData;
		}

		template<
			typename ElementType,
			typename ViewSizeType,
			bool AllowResize = SupportResize,
			typename ConstructType = Memory::DefaultConstructType,
			typename = EnableIf<AllowResize>>
		ArrayView<ContainedType, SizeType> MoveEmplaceRange(
			const ConstPointerType whereIt, ConstructType, const ArrayView<ElementType, ViewSizeType> range
		) noexcept LIFETIME_BOUND
		{
			Assert(!Overlaps(range));

			Assert(whereIt >= begin().Get() || (whereIt == nullptr && IsEmpty()));
			const SizeType index = static_cast<SizeType>(whereIt - GetData());

			ReserveOrAssert((SizeType)Math::Max(index + 1 + range.GetSize(), m_size + range.GetSize()), Memory::ReserveExponential);

			if constexpr (TypeTraits::IsSame<ConstructType, Memory::DefaultConstructType>)
			{
				if constexpr (TypeTraits::IsTriviallyConstructible<ContainedType> || TypeTraits::IsDefaultConstructible<ContainedType>)
				{
					BaseType::m_allocator.GetView()
						.GetSubView(m_size, (SizeType)Math::Max((int64)index - (int64)m_size, (int64)0))
						.DefaultConstruct();
				}
				else
				{
					Assert((index <= m_size) | (index == m_size), "Can not default construct elements!");
				}
			}
			else if constexpr (TypeTraits::IsSame<ConstructType, Memory::UninitializedType>)
				;
			else
			{
				static_unreachable("Invalid construct type");
			}

			// Copy existing elements at index
			{
				View target = BaseType::m_allocator.GetView() + index + range.GetSize();

				const SizeType numMovedElements = Math::Max(m_size, index) - index;
				target.CopyFromWithOverlap(GetSubView(index, numMovedElements));
			}

			PointerType pData = BaseType::m_allocator.GetData();
			for (ElementType& newElement : range)
			{
				const SizeType rangeIndex = range.GetIteratorIndex(Memory::GetAddressOf(newElement));
				new (pData + index + rangeIndex) ContainedType(Move(newElement));
			}
			m_size = (SizeType)Math::Max(index + range.GetSize(), m_size + range.GetSize());
			return {pData + index, range.GetSize()};
		}

		template<typename ElementType, typename ViewSizeType, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		ArrayView<ContainedType, SizeType> MoveEmplaceRangeBack(const ArrayView<ElementType, ViewSizeType> range) noexcept LIFETIME_BOUND
		{
			Assert(!Overlaps(range));

			ReserveOrAssert(m_size + range.GetSize(), Memory::ReserveExponential);
			const SizeType index = GetSize();

			PointerType pData = BaseType::m_allocator.GetData();
			for (ElementType& newElement : range)
			{
				const SizeType rangeIndex = range.GetIteratorIndex(Memory::GetAddressOf(newElement));
				new (pData + index + rangeIndex) ContainedType(Move(newElement));
			}
			m_size += range.GetSize();
			return {pData + index, range.GetSize()};
		}

		template<
			typename ElementType,
			typename ViewSizeType,
			bool AllowResize = SupportResize,
			typename ConstructType = Memory::DefaultConstructType,
			typename = EnableIf<AllowResize>>
		ArrayView<ContainedType, SizeType> CopyEmplaceRange(
			const ConstPointerType whereIt, ConstructType, const ArrayView<const ElementType, ViewSizeType> range
		) noexcept LIFETIME_BOUND
		{
			Assert(!Overlaps(range));

			Assert(whereIt >= begin().Get() || (whereIt == nullptr && IsEmpty()));
			const SizeType index = static_cast<SizeType>(whereIt - GetData());

			ReserveOrAssert((SizeType)Math::Max(index + 1 + range.GetSize(), m_size + range.GetSize()), Memory::ReserveExponential);

			if constexpr (TypeTraits::IsSame<ConstructType, Memory::DefaultConstructType>)
			{
				if constexpr (TypeTraits::IsTriviallyConstructible<ContainedType> || TypeTraits::IsDefaultConstructible<ContainedType>)
				{
					BaseType::m_allocator.GetView()
						.GetSubView(m_size, (SizeType)Math::Max((int64)index - (int64)m_size, (int64)0))
						.DefaultConstruct();
				}
				else
				{
					Assert((index <= m_size) | (index == m_size), "Can not default construct elements!");
				}
			}
			else if constexpr (TypeTraits::IsSame<ConstructType, Memory::UninitializedType>)
				;
			else
			{
				static_unreachable("Invalid construct type");
			}

			// Copy existing elements at index
			{
				View target = BaseType::m_allocator.GetView() + index + range.GetSize();

				const SizeType numMovedElements = Math::Max(m_size, index) - index;
				target.CopyFromWithOverlap(GetSubView(index, numMovedElements));
			}

			PointerType pData = BaseType::m_allocator.GetData();
			for (const ElementType& newElement : range)
			{
				const SizeType rangeIndex = range.GetIteratorIndex(Memory::GetAddressOf(newElement));
				new (pData + index + rangeIndex) ContainedType(newElement);
			}
			m_size = (SizeType)Math::Max(index + range.GetSize(), m_size + range.GetSize());
			return {pData + index, range.GetSize()};
		}

		template<typename ElementType, typename ViewSizeType, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		ArrayView<ContainedType, SizeType> CopyEmplaceRangeBack(const ArrayView<const ElementType, ViewSizeType> range) noexcept LIFETIME_BOUND
		{
			Assert(!Overlaps(range));

			ReserveOrAssert(m_size + range.GetSize(), Memory::ReserveExponential);
			const SizeType index = GetSize();

			PointerType pData = BaseType::m_allocator.GetData();
			for (const ElementType& newElement : range)
			{
				const SizeType rangeIndex = range.GetIteratorIndex(Memory::GetAddressOf(newElement));
				new (pData + index + rangeIndex) ContainedType(newElement);
			}
			m_size += range.GetSize();
			return {pData + index, range.GetSize()};
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void PopFront() noexcept
		{
			Expect(m_size > 0);
			Remove(begin());
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		ContainedType PopAndGetFront() noexcept
		{
			Expect(m_size > 0);
			PointerType elementIt = begin();
			ContainedType element = Move(*elementIt);
			Remove(elementIt);
			return element;
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void PopBack() noexcept
		{
			Expect(m_size > 0);
			if constexpr (!TypeTraits::IsTriviallyDestructible<ContainedType>)
			{
				PointerType elementIt = end() - 1;
				elementIt->~ContainedType();
			}
			m_size--;
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		ContainedType PopAndGetBack() noexcept
		{
			Expect(m_size > 0);
			PointerType elementIt = end() - 1;
			ContainedType element = Move(*elementIt);
			m_size--;
			return element;
		}

		void MoveElement(const ConstPointerType whereIt, ReferenceType existingElement)
		{
			Assert(whereIt != Memory::GetAddressOf(existingElement));
			const SizeType index = static_cast<SizeType>(whereIt - BaseType::m_allocator.GetData());
			ReserveOrAssert(m_size + index + 1, Memory::ReserveExponential);

			ContainedType temporary = Move(*whereIt);
			*whereIt = Move(existingElement);
			existingElement = Move(temporary);
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void MoveFrom(const ConstPointerType whereIt, TVector& other) noexcept
		{
			Assert(BaseType::m_allocator.GetView().IsWithinBounds(whereIt) | (whereIt == BaseType::m_allocator.GetView().end()));
			const SizeType index = static_cast<SizeType>(whereIt - BaseType::m_allocator.GetData());

			Assert(m_size != GetTheoreticalCapacity());
			ReserveOrAssert(m_size + index + other.GetSize(), Memory::ReserveExponential);

			// Copy existing elements at index
			{
				View target = BaseType::m_allocator.GetView() + index + other.GetSize();
				const SizeType numMovedElements = m_size - index;

				target.CopyFromWithOverlap(GetSubView(index, numMovedElements));
			}

			m_size += other.GetSize();
			GetSubView(index, other.GetSize()).MoveConstruct(other.GetView());
			other.m_size = 0;
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void CopyFrom(const ConstPointerType whereIt, const ConstView view) noexcept
		{
			Assert(whereIt >= BaseType::m_allocator.GetData());
			const SizeType index = static_cast<SizeType>(whereIt - BaseType::m_allocator.GetData());

			Assert(m_size != GetTheoreticalCapacity());
			ReserveOrAssert(m_size + index + view.GetSize(), Memory::ReserveExponential);

			// Copy existing elements at index
			{
				View target = BaseType::m_allocator.GetView() + index + view.GetSize();
				const SizeType numMovedElements = m_size - index;
				target.CopyFromWithOverlap(GetSubView(index, numMovedElements));
			}

			m_size += view.GetSize();

			GetSubView(index, view.GetSize()).CopyConstruct(view);
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void Clear() noexcept
		{
			const View view = GetView();
			m_size = 0;
			if constexpr (!TypeTraits::IsTriviallyDestructible<ContainedType>)
			{
				for (ContainedType& element : view)
				{
					element.~ContainedType();
				}
			}
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void Remove(const ConstPointerType elementIt) noexcept
		{
			Assert(elementIt >= BaseType::m_allocator.GetData());
			Assert(elementIt < end());

			const SizeType index = GetIteratorIndex(elementIt);
			const SizeType numItemsToMove = m_size - index - 1;
			m_size--;

			if constexpr (!TypeTraits::IsTriviallyDestructible<ContainedType>)
			{
				elementIt->~ContainedType();
			}

			View memoryView = GetView() + index;
			memoryView.CopyFromWithOverlap(ConstView{memoryView.GetData() + 1, numItemsToMove});
		}

		FORCE_INLINE void RemoveAt(const IndexType index) noexcept
		{
			Remove(begin() + (SizeType)index);
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		FORCE_INLINE void RemoveLastElement() noexcept
		{
			Remove(end() - 1);
		}

		template<bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void Remove(const ConstView view) noexcept
		{
			Assert(GetView().IsWithinBounds(view) || view.IsEmpty());

			if constexpr (!TypeTraits::IsTriviallyDestructible<ContainedType>)
			{
				for (const ContainedType& element : view)
				{
					element.~ContainedType();
				}
			}

			const SizeType startIndex = static_cast<SizeType>(view.GetData() - GetData());
			const SizeType endIndex = startIndex + view.GetSize();
			const SizeType numItemsToMove = m_size - endIndex;
			View memoryView = GetView() + startIndex;
			memoryView.CopyFromWithOverlap(ConstView{memoryView.GetData() + view.GetSize(), numItemsToMove});

			m_size -= view.GetSize();
		}

		template<typename ComparableType, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void RemoveAllOccurrences(const ComparableType& comparedElement) noexcept
		{
			for (PointerType it = GetData(), endIt = end(); it != endIt;)
			{
				if (*it == comparedElement)
				{
					endIt--;
					Remove(it);
				}
				else
				{
					++it;
				}
			}
		}

		template<typename Callback, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void RemoveAllOccurrencesPredicate(const Callback callback) noexcept
		{
			for (PointerType it = GetData(), endIt = end(); it != endIt;)
			{
				if (callback(*it) == ErasePredicateResult::Remove)
				{
					endIt--;
					Remove(it);
				}
				else
				{
					++it;
				}
			}
		}

		template<typename ComparableType, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		bool RemoveFirstOccurrence(const ComparableType& comparedElement) noexcept
		{
			for (PointerType it = GetData(), endIt = end(); it != endIt; ++it)
			{
				if (*it == comparedElement)
				{
					Remove(it);
					return true;
				}
			}

			return false;
		}

		template<typename Callback, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		bool RemoveFirstOccurrencePredicate(Callback callback) noexcept
		{
			for (PointerType it = GetData(), endIt = end(); it != endIt; ++it)
			{
				if (callback(*it) == ErasePredicateResult::Remove)
				{
					Remove(it);
					return true;
				}
			}

			return false;
		}

		template<typename ComparableType, typename ViewSizeType, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void RemoveAllOccurrencesNotInRange(const ArrayView<ComparableType, ViewSizeType> elements) noexcept
		{
			for (const ComparableType& element : elements)
			{
				for (PointerType it = GetData(), endIt = end(); it != endIt;)
				{
					if (*it != element)
					{
						endIt--;
						Remove(it);
					}
					else
					{
						++it;
					}
				}
			}
		}

		template<typename ComparableType, typename ViewSizeType, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void RemoveAllOccurrencesInRange(const ArrayView<ComparableType, ViewSizeType> elements) noexcept
		{
			for (const ComparableType& element : elements)
			{
				RemoveAllOccurrences(element);
			}
		}

		template<typename ComparableType, typename ViewSizeType, bool AllowResize = SupportResize, typename = EnableIf<AllowResize>>
		void RemoveFirstOccurrenceInRange(const ArrayView<ComparableType, ViewSizeType> elements) noexcept
		{
			for (const ComparableType& element : elements)
			{
				RemoveFirstOccurrence(element);
			}
		}

		template<
			bool AllowReallocate = SupportReallocate,
			typename ReserveStrategyType = Memory::ReserveExactType,
			typename = EnableIf<AllowReallocate>>
		void Reserve(SizeType desiredCapacity, const ReserveStrategyType = Memory::ReserveExact) noexcept
		{
			Assert(desiredCapacity <= GetTheoreticalCapacity(), "Can't reserve more than an allocator's theoretical capacity!");
			if (GetCapacity() < desiredCapacity)
			{
				if constexpr (TypeTraits::IsSame<ReserveStrategyType, Memory::ReserveExponentialType>)
				{
					// TODO: Try setting this to 2-ish, not sure why ours is so much higher than std on most compilers
					desiredCapacity = Math::Min(SizeType(desiredCapacity * (SizeType)4), GetTheoreticalCapacity());
				}
				BaseType::m_allocator.Allocate(desiredCapacity);
			}
		}
		template<
			bool AllowReallocate = SupportReallocate,
			typename ReserveStrategyType = Memory::ReserveExactType,
			typename = EnableIf<AllowReallocate>>
		void
		ReserveAdditionalCapacity(const SizeType additionalCapacity, const ReserveStrategyType reserveStrategy = Memory::ReserveExact) noexcept
		{
			Reserve(m_size + additionalCapacity, reserveStrategy);
		}

		//! Grow or shrink the vector to the specified size, depending on current size
		template<typename ElementType = ContainedType, bool AllowResize = SupportResize, typename ConstructType = Memory::DefaultConstructType>
		EnableIf<AllowResize> Resize(const SizeType size, const ConstructType constructType = Memory::DefaultConstruct) noexcept
		{
			if constexpr (TypeTraits::IsSame<ConstructType, Memory::DefaultConstructType> || TypeTraits::IsSame<ConstructType, Memory::UninitializedType>)
			{
				if (m_size < size)
				{
					Grow(size, constructType);
				}
				else
				{
					Shrink(size);
				}
			}
			else if constexpr (TypeTraits::IsSame<ConstructType, Memory::ZeroedType>)
			{
				const SizeType previousSize = m_size;
				Resize(size, Memory::Uninitialized);
				GetView().GetSubViewFrom(previousSize).ZeroInitialize();
			}
			else
			{
				static_unreachable("Invalid construct type");
			}
		}

		//! Grows the vector to the specified size
		//! Assumes that the requested size is always larger or equal to the current size
		template<typename ElementType = ContainedType, bool AllowResize = SupportResize, typename ConstructType = Memory::DefaultConstructType>
		EnableIf<AllowResize> Grow(const SizeType size, const ConstructType = Memory::DefaultConstruct) noexcept
		{
			Expect(size >= m_size);
			if constexpr (TypeTraits::IsSame<ConstructType, Memory::DefaultConstructType>)
			{
				static_assert(TypeTraits::IsDefaultConstructible<ElementType>, "Type must be default constructible!");
				ReserveOrAssert(size, Memory::ReserveExact);
				Assert(size <= GetCapacity());
				PointerType pData = BaseType::m_allocator.GetData();
				View(pData + m_size, pData + size).DefaultConstruct();
				m_size = size;
			}
			else if constexpr (TypeTraits::IsSame<ConstructType, Memory::UninitializedType>)
			{
				ReserveOrAssert(size, Memory::ReserveExact);
				Assert(size <= GetCapacity());
				m_size = size;
			}
			else if constexpr (TypeTraits::IsSame<ConstructType, Memory::ZeroedType>)
			{
				ReserveOrAssert(size, Memory::ReserveExact);
				Assert(size <= GetCapacity());
				PointerType pData = BaseType::m_allocator.GetData();
				View(pData + m_size, pData + size).ZeroInitialize();
				m_size = size;
			}
			else
			{
				static_unreachable("Invalid construct type");
			}
		}

		//! Shrinks the vector to the specified size
		//! Note: this does not reallocate to a smaller size
		//! Assumes that the requested size is always smaller or equal to the current size
		template<bool AllowResize = SupportResize>
		EnableIf<AllowResize> Shrink(const SizeType size) noexcept
		{
			Assert(size <= m_size);
			PointerType pData = BaseType::m_allocator.GetData();
			if constexpr (!TypeTraits::IsTriviallyDestructible<ContainedType>)
			{
				for (ContainedType& element : View(pData + size, pData + m_size))
				{
					element.~ContainedType();
				}
			}
			m_size = size;
		}

		void Swap(TVector&& other) noexcept
		{
			TVector temp = Move(*this);
			*this = Move(other);
			other = Move(temp);
		}

		template<typename ComparableType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS OptionalIteratorType Find(const ComparableType& element) noexcept LIFETIME_BOUND
		{
			return GetView().Find(element);
		}

		template<typename ComparableType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS OptionalConstIteratorType Find(const ComparableType& element) const noexcept LIFETIME_BOUND
		{
			return GetView().Find(element);
		}

		template<typename ComparableType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS Optional<IndexType> FindIndex(const ComparableType& element) const noexcept
		{
			const OptionalConstIteratorType iterator = Find(element);
			return iterator.IsValid() ? Optional<IndexType>(GetIteratorIndex(iterator)) : Invalid;
		}

		template<typename ComparableType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool Contains(const ComparableType& element) const noexcept
		{
			return GetView().Contains(element);
		}
		template<typename ComparableType, typename OtherSizeType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool ContainsAny(const ArrayView<ComparableType, OtherSizeType> elements) const noexcept
		{
			return GetView().ContainsAny(elements);
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS OptionalIteratorType FindIf(const Callback callback) noexcept LIFETIME_BOUND
		{
			return GetView().FindIf(callback);
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS OptionalConstIteratorType FindIf(const Callback callback) const noexcept LIFETIME_BOUND
		{
			return GetView().FindIf(callback);
		}

		template<typename Callback>
		[[nodiscard]] FORCE_INLINE PURE_STATICS bool ContainsIf(const Callback callback) const noexcept
		{
			return GetView().ContainsIf(callback);
		}

		[[nodiscard]] PURE_STATICS bool operator==(const ConstView other) const
		{
			return GetView() == other;
		}
		[[nodiscard]] PURE_STATICS bool operator!=(const ConstView other) const
		{
			return GetView() != other;
		}

		[[nodiscard]] constexpr uint32 CalculateCompressedDataSize() const;
		bool Compress(BitView& target) const;
		bool Decompress(ConstBitView& source);
	protected:
		template<typename ReserveStrategyType = Memory::ReserveExactType>
		inline void ReserveOrAssert(const SizeType desiredCapacity, const ReserveStrategyType reserveStrategy = Memory::ReserveExact) noexcept
		{
			if constexpr (SupportReallocate)
			{
				Reserve(desiredCapacity, reserveStrategy);
			}
			else
			{
				UNUSED(desiredCapacity);
				Assert(desiredCapacity <= GetCapacity());
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr bool CanReallocate() noexcept
		{
			return (Flags & Memory::VectorFlags::AllowReallocate) != 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr bool CanResize() noexcept
		{
			return (Flags & Memory::VectorFlags::AllowResize) != 0;
		}
	protected:
		SizeType m_size = 0;
	};
}
