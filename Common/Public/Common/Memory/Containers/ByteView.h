#pragma once

#include "ArrayView.h"
#include "ForwardDeclarations/ByteView.h"
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsConst.h>
#include <Common/TypeTraits/Select.h>
#include <Common/TypeTraits/IsEnum.h>
#include <Common/Memory/AddressOf.h>
#include <Common/Memory/GetAlignment.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	template<typename InternalByteType, typename InternalSizeType>
	struct TRIVIAL_ABI TByteView : protected ArrayView<InternalByteType, InternalSizeType>
	{
		using BaseType = ArrayView<InternalByteType, InternalSizeType>;
		using ConstView = TByteView<const InternalByteType, InternalSizeType>;
		using SizeType = typename BaseType::SizeType;

		FORCE_INLINE constexpr TByteView() = default;
		template<typename OtherElementType, typename OtherSizeType>
		FORCE_INLINE constexpr TByteView(OtherElementType* pData, const OtherSizeType count)
			: BaseType(reinterpret_cast<InternalByteType*>(pData), count * sizeof(OtherElementType))
		{
		}

		template<typename Type, typename SizeType>
		FORCE_INLINE TByteView(const TByteView<Type, SizeType> view) noexcept
			: BaseType(reinterpret_cast<InternalByteType*>(view.GetData()), (InternalSizeType)view.GetDataSize())
		{
		}
		template<typename Type, typename SizeType, typename IndexType>
		FORCE_INLINE TByteView(const ArrayView<Type, SizeType, IndexType> view) noexcept
			: BaseType(reinterpret_cast<InternalByteType*>(view.GetData()), (InternalSizeType)view.GetDataSize())
		{
		}
		template<typename Type, typename SizeType>
		FORCE_INLINE TByteView& operator=(const TByteView<Type, SizeType> view) noexcept
		{
			BaseType::operator=(BaseType(reinterpret_cast<InternalByteType*>(view.GetData()), (InternalSizeType)view.GetDataSize()));
			return *this;
		}
		template<typename Type, typename SizeType, typename IndexType>
		FORCE_INLINE TByteView& operator=(const ArrayView<Type, SizeType, IndexType> view) noexcept
		{
			BaseType::operator=(BaseType(reinterpret_cast<InternalByteType*>(view.GetData()), (InternalSizeType)view.GetDataSize()));
			return *this;
		}

		template<
			typename Type,
			typename InternalType = InternalByteType,
			typename = EnableIf<TypeTraits::IsConst<InternalType> || !TypeTraits::IsConst<Type>>>
		[[nodiscard]] FORCE_INLINE static TByteView Make(Type& value)
		{
			return TByteView{BaseType(reinterpret_cast<InternalByteType*>(Memory::GetAddressOf(value)), (InternalSizeType)sizeof(value))};
		}

		[[nodiscard]] FORCE_INLINE operator ConstView() const
		{
			return ConstView{reinterpret_cast<const InternalByteType*>(BaseType::GetData()), (InternalSizeType)BaseType::GetDataSize()};
		}

		using BaseType::GetData;
		using BaseType::GetDataSize;
		using BaseType::IsEmpty;
		using BaseType::HasElements;
		using BaseType::operator==;
		using BaseType::operator!=;
		using BaseType::operator[];
		using BaseType::begin;
		using BaseType::end;
		using BaseType::OneInitialize;
		using BaseType::ZeroInitialize;
		using BaseType::GetIteratorIndex;

		[[nodiscard]] FORCE_INLINE PURE_STATICS size GetAlignment() const
		{
			return Memory::GetAlignment(GetData());
		}

		TByteView& operator+=(const SizeType offset)
		{
			BaseType::operator+=(offset);
			return *this;
		}
		TByteView& operator++()
		{
			BaseType::operator++();
			return *this;
		}
		TByteView& operator++(int)
		{
			BaseType::operator++(0);
			return *this;
		}
		[[nodiscard]] PURE_STATICS TByteView operator+(const SizeType offset)
		{
			return TByteView(BaseType::operator+(offset));
		}
		TByteView& operator-=(const SizeType offset)
		{
			BaseType::operator-=(offset);
			return *this;
		}
		[[nodiscard]] PURE_STATICS TByteView operator-(const SizeType offset)
		{
			return TByteView(BaseType::operator-(offset));
		}
		TByteView& operator--()
		{
			BaseType::operator--();
			return *this;
		}
		TByteView& operator--(int)
		{
			BaseType::operator--(0);
			return *this;
		}

		template<typename Type, typename SizeType, typename IndexType>
		FORCE_INLINE bool CopyFrom(const ArrayView<Type, SizeType, IndexType> view) const
		{
			return BaseType::CopyFrom(ConstView(view));
		}
		template<typename OtherByteType, typename OtherSizeType>
		FORCE_INLINE bool CopyFrom(const TByteView<OtherByteType, OtherSizeType> view) const
		{
			return BaseType::CopyFrom(typename BaseType::ConstView{
				reinterpret_cast<const InternalByteType*>(view.GetData()),
				static_cast<InternalSizeType>(view.GetDataSize())
			});
		}
		template<typename Type, typename SizeType, typename IndexType>
		FORCE_INLINE bool CopyFromWithOverlap(const ArrayView<Type, SizeType, IndexType> view) const
		{
			return BaseType::CopyFromWithOverlap(ConstView(view));
		}
		template<typename OtherByteType, typename OtherSizeType>
		FORCE_INLINE bool CopyFromWithOverlap(const TByteView<OtherByteType, OtherSizeType> view) const
		{
			return BaseType::CopyFromWithOverlap(typename BaseType::ConstView{
				reinterpret_cast<const InternalByteType*>(view.GetData()),
				static_cast<InternalSizeType>(view.GetDataSize())
			});
		}

		template<typename OtherSizeType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr TByteView
		GetSubView(const OtherSizeType index, const OtherSizeType count) const noexcept
		{
			return BaseType::GetSubView(index, count);
		}
		template<typename OtherSizeType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr TByteView GetSubViewFrom(const OtherSizeType index) const noexcept
		{
			return BaseType::GetSubViewFrom(index);
		}
		template<typename OtherSizeType>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr TByteView GetSubViewUpTo(const OtherSizeType index) const noexcept
		{
			return BaseType::GetSubViewUpTo(index);
		}

		template<
			typename Type,
			typename ThisByteType = InternalByteType,
			typename StoredType = TypeTraits::Select<TypeTraits::IsConst<ThisByteType>, const Type, Type>>
		[[nodiscard]] PURE_STATICS Optional<StoredType*> Read() const
		{
			if constexpr (TypeTraits::IsEnum<Type>)
			{
				using UnderlyingEnumType =
					TypeTraits::Select<TypeTraits::IsConst<InternalByteType>, const UNDERLYING_TYPE(Type), UNDERLYING_TYPE(Type)>;
				return Optional<StoredType*>{
					reinterpret_cast<StoredType*>(reinterpret_cast<UnderlyingEnumType*>(GetData())),
					GetDataSize() >= sizeof(Type)
				};
			}
			else
			{
				return Optional<StoredType*>{reinterpret_cast<StoredType*>(GetData()), GetDataSize() >= sizeof(Type)};
			}
		}

		template<
			typename Type,
			typename ThisByteType = InternalByteType,
			typename StoredType = TypeTraits::Select<TypeTraits::IsConst<ThisByteType>, const Type, Type>>
		[[nodiscard]] Optional<StoredType*> ReadAndSkip()
		{
			Optional<StoredType*> element = Read<Type>();
			*this += sizeof(Type) * element.IsValid();
			return element;
		}

		template<
			typename Type,
			typename ThisByteType = InternalByteType,
			typename StoredType = TypeTraits::Select<TypeTraits::IsConst<ThisByteType>, const Type, Type>>
		[[nodiscard]] StoredType ReadAndSkipWithDefaultValue(TypeTraits::WithoutConst<Type>&& defaultValue)
		{
			if (Optional<StoredType*> element = ReadAndSkip<Type>())
			{
				return *element;
			}
			else
			{
				return Forward<StoredType>(defaultValue);
			}
		}

		template<typename Type, typename SizeType>
		FORCE_INLINE bool ReadIntoView(const ArrayView<Type, SizeType> data) const
		{
			const ConstByteView source = GetSubView(size(0), data.GetDataSize());
			ByteView target{data};
			return target.CopyFrom(source);
		}

		template<typename Type, typename SizeType>
		FORCE_INLINE bool ReadIntoViewAndSkip(const ArrayView<Type, SizeType> data)
		{
			const bool success = ReadIntoView(data);
			*this += data.GetDataSize() * success;
			return success;
		}

		template<typename Type, typename ThisByteType = InternalByteType>
		EnableIf<!TypeTraits::IsConst<ThisByteType>, bool> Write(const Type& value) const
		{
			ConstByteView source = ConstByteView::Make(value);
			return CopyFrom(source);
		}

		template<typename Type, typename ThisByteType = InternalByteType>
		EnableIf<!TypeTraits::IsConst<ThisByteType>, bool> WriteAndSkip(const Type& value)
		{
			const bool wasWritten = Write(value);
			*this += sizeof(Type);
			return wasWritten;
		}
	};
}
