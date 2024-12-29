#pragma once

#include <Common/Platform/LifetimeBound.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Memory/GetNumericSize.h>
#include "ContainerCommon.h"
#include "FixedArrayView.h"
#include <Common/Assert/Assert.h>
#include <Common/Memory/Forward.h>
#include <Common/Memory/Move.h>
#include <Common/Memory/New.h>
#include <Common/Memory/AddressOf.h>
#include <Common/TypeTraits/IsTriviallyCopyable.h>
#include <Common/TypeTraits/IsMoveConstructible.h>
#include <Common/TypeTraits/EnforceConvertibleTo.h>
#include "ForwardDeclarations/Array.h"

namespace ngine
{
	template<typename ContainedType, size Size_, typename InternalIndexType, typename InternalSizeType>
	struct TRIVIAL_ABI Array
	{
		using SizeType = InternalSizeType;
		using DataSizeType = Memory::NumericSize<Size_ * sizeof(ContainedType)>;
		using IndexType = InternalIndexType;

		inline static constexpr SizeType Size = Size_;

		using View = ArrayView<ContainedType, SizeType, IndexType, ContainedType>;
		using ConstView = ArrayView<const ContainedType, SizeType, IndexType, const ContainedType>;
		using RestrictedView = typename View::RestrictedView;
		using RestrictedConstView = typename ConstView::RestrictedView;

		using FixedView = FixedArrayView<ContainedType, Size, IndexType, SizeType>;
		using ConstFixedView = FixedArrayView<const ContainedType, Size, IndexType, SizeType>;

		using PointerType = typename View::PointerType;
		using ConstPointerType = typename View::ConstPointerType;
		using ReferenceType = typename View::ReferenceType;
		using iterator = typename View::iterator;
		using const_iterator = typename View::const_iterator;
		using IteratorType = typename View::IteratorType;
		using ConstIteratorType = typename View::ConstIteratorType;

		using OptionalIteratorType = typename View::OptionalIteratorType;
		using OptionalConstIteratorType = typename View::OptionalConstIteratorType;

		constexpr Array() = default;

		template<typename... ConstructArgs>
		constexpr Array(const Memory::InitializeAllType, ConstructArgs&&... args) noexcept
		{
			GetView().InitializeAll(Forward<ConstructArgs>(args)...);
		}
		constexpr Array(const Memory::ZeroedType) noexcept
		{
			GetView().ZeroInitialize();
		}
		template<typename... Args, typename = EnableIf<(TypeTraits::IsConvertibleTo<Args, ContainedType> && ...)>>
		constexpr Array(Args&&... args) noexcept
			: m_data{Forward<Args>(args)...}
		{
			static_assert(sizeof...(args) == Size, "All arguments must be initialized!");
		}

		explicit constexpr Array(const Array& other) noexcept = default;
		constexpr Array& operator=(const Array& other) noexcept = default;
		constexpr Array(Array&& other) noexcept = default;
		constexpr Array& operator=(Array&& other) noexcept = default;

		template<typename ElementType = ContainedType, typename = EnableIf<TypeTraits::IsCopyConstructible<ElementType>>>
		FORCE_INLINE constexpr Array(const ConstView view) noexcept
		{
			Assert(view.GetSize() <= GetSize());
			GetSubView(0, view.GetSize()).CopyConstruct(view);
			if constexpr (TypeTraits::IsDefaultConstructible<ElementType>)
			{
				for (ContainedType& otherElement : GetSubView(view.GetSize(), GetSize() - view.GetSize()))
				{
					new (Memory::GetAddressOf(otherElement)) ContainedType();
				}
			}
			else
			{
				Assert(view.GetSize() == GetSize());
			}
		}
		~Array() = default;

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetSize() const noexcept
		{
			return Size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr DataSizeType GetDataSize() const noexcept
		{
			return sizeof(ContainedType) * Size;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr PointerType GetData() noexcept LIFETIME_BOUND
		{
			return m_data;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstPointerType GetData() const noexcept LIFETIME_BOUND
		{
			return m_data;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator View() noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator ConstView() const noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator FixedView() noexcept LIFETIME_BOUND
		{
			return FixedView(m_data);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator ConstFixedView() const noexcept LIFETIME_BOUND
		{
			return ConstFixedView(m_data);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator RestrictedView() noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator RestrictedConstView() const noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
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
		operator ArrayView<OtherElementType, OtherSizeType, OtherIndexType, OtherStoredType>() const noexcept LIFETIME_BOUND
		{
			return ArrayView<OtherElementType, OtherSizeType, OtherIndexType, OtherStoredType>{GetData(), Size};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr FixedView GetView() noexcept LIFETIME_BOUND
		{
			return FixedView(m_data);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstFixedView GetView() const noexcept LIFETIME_BOUND
		{
			return ConstFixedView(m_data);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr View GetDynamicView() noexcept LIFETIME_BOUND
		{
			return View(m_data, Size);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView GetDynamicView() const noexcept LIFETIME_BOUND
		{
			return ConstView(m_data, Size);
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

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ContainedType& operator[](const IndexType index) noexcept LIFETIME_BOUND
		{
			Assert((SizeType)index < GetSize());
			return m_data[(SizeType)index];
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const ContainedType& operator[](const IndexType index) const noexcept LIFETIME_BOUND
		{
			Assert((SizeType)index < GetSize());
			return m_data[(SizeType)index];
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr PointerType begin() noexcept LIFETIME_BOUND
		{
			return Memory::GetAddressOf(m_data[0]);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstPointerType begin() const noexcept LIFETIME_BOUND
		{
			return Memory::GetAddressOf(m_data[0]);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr PointerType end() noexcept LIFETIME_BOUND
		{
			return begin() + Size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstPointerType end() const noexcept LIFETIME_BOUND
		{
			return begin() + Size;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsValidIndex(const IndexType index) const noexcept
		{
			return index < Size;
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
	protected:
		ContainedType m_data[Size];
	};

	template<typename ContainedType, typename InternalIndexType, typename InternalSizeType>
	struct Array<ContainedType, 0, InternalIndexType, InternalSizeType>
	{
		using SizeType = InternalSizeType;
		using IndexType = InternalIndexType;

		using View = ArrayView<ContainedType, SizeType, IndexType, ContainedType>;
		using ConstView = ArrayView<const ContainedType, SizeType, IndexType, const ContainedType>;
		using RestrictedView = typename View::RestrictedView;
		using RestrictedConstView = typename ConstView::RestrictedView;

		using FixedView = FixedArrayView<ContainedType, 0, IndexType, SizeType>;
		using ConstFixedView = FixedArrayView<const ContainedType, 0, IndexType, SizeType>;

		using PointerType = typename View::PointerType;
		using ConstPointerType = typename View::ConstPointerType;

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetSize() const noexcept
		{
			return 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetDataSize() const noexcept
		{
			return 0;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr PointerType GetData() noexcept
		{
			return nullptr;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstPointerType GetData() const noexcept
		{
			return nullptr;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr FixedView GetView() noexcept LIFETIME_BOUND
		{
			return {};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstFixedView GetView() const noexcept LIFETIME_BOUND
		{
			return {};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr View GetDynamicView() noexcept LIFETIME_BOUND
		{
			return {};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView GetDynamicView() const noexcept LIFETIME_BOUND
		{
			return {};
		}
	};

	template<class FirstType, class... RemainingTypes>
	Array(FirstType, RemainingTypes...)
		-> Array<typename TypeTraits::EnforceConvertibleTo<FirstType, RemainingTypes...>::Type, 1 + sizeof...(RemainingTypes)>;
}
