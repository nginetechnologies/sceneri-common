#include "Common/Math/Vector2.h"
#include "Common/Math/Vector3.h"
#include "Common/Math/Vector4.h"
#include "Common/Math/WorldCoordinate.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <Common/Memory/Containers/BitView.h>

#include <Common/Math/Vector2/Abs.h>
#include <Common/Math/Vector2/Quantize.h>
#include <Common/Math/Vector3/Quantize.h>
#include <Common/TypeTraits/InnermostType.h>

#include <Common/Math/Angle.h>
#include <Common/Math/Ratio.h>

namespace ngine::Math
{
	template<typename T>
	template<typename S, typename>
	bool TVector4<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		using ActualType = TypeTraits::InnermostType<T>;
		Assert(currentElement.IsArray());
		switch (currentElement.Size())
		{
			case 4:
				x = static_cast<ActualType>(currentElement[0].GetDouble());
				y = static_cast<ActualType>(currentElement[1].GetDouble());
				z = static_cast<ActualType>(currentElement[2].GetDouble());
				w = static_cast<ActualType>(currentElement[3].GetDouble());
				break;
			case 3:
				x = static_cast<ActualType>(currentElement[0].GetDouble());
				y = static_cast<ActualType>(currentElement[1].GetDouble());
				z = static_cast<ActualType>(currentElement[2].GetDouble());
				w = 0;
				break;
			case 2:
				x = static_cast<ActualType>(currentElement[0].GetDouble());
				y = static_cast<ActualType>(currentElement[1].GetDouble());
				z = 0;
				break;
			case 1:
				x = y = z = static_cast<ActualType>(currentElement[0].GetDouble());
				break;
			case 0:
				x = y = z = 0;
				break;
			default:
				ExpectUnreachable();
		}

		return true;
	}

	template<typename T>
	template<typename S, typename>
	bool TVector4<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);
		currentElement.Reserve(4, serializer.GetDocument().GetAllocator());

		const bool areAllEqual =
			false; //(Math::Abs(TVector2<T>{x, x} - TVector2<T>{y, z}) <= TVector2<T>{Math::NumericLimits<T>::Epsilon}).AreAllSet();
		if (areAllEqual)
		{
			if (x != T(0))
			{
				currentElement.PushBack(x, serializer.GetDocument().GetAllocator());
			}
		}
		else
		{
			currentElement.PushBack(x, serializer.GetDocument().GetAllocator());
			currentElement.PushBack(y, serializer.GetDocument().GetAllocator());
			currentElement.PushBack(z, serializer.GetDocument().GetAllocator());
			currentElement.PushBack(w, serializer.GetDocument().GetAllocator());
		}
		return true;
	}

	template bool TVector4<float>::Serialize(const Serialization::Reader);
	template bool TVector4<float>::Serialize(Serialization::Writer) const;
	template bool TVector4<int32>::Serialize(const Serialization::Reader);
	template bool TVector4<int32>::Serialize(Serialization::Writer) const;
	template bool TVector4<uint32>::Serialize(const Serialization::Reader);
	template bool TVector4<uint32>::Serialize(Serialization::Writer) const;

	template<typename T>
	template<typename S, typename>
	bool TVector3<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		using ActualType = TypeTraits::InnermostType<T>;
		Assert(currentElement.IsArray());
		switch (currentElement.Size())
		{
			case 3:
				x = static_cast<ActualType>(currentElement[0].GetDouble());
				y = static_cast<ActualType>(currentElement[1].GetDouble());
				z = static_cast<ActualType>(currentElement[2].GetDouble());
				break;
			case 2:
				x = static_cast<ActualType>(currentElement[0].GetDouble());
				y = static_cast<ActualType>(currentElement[1].GetDouble());
				z = 0;
				break;
			case 1:
				x = y = z = static_cast<ActualType>(currentElement[0].GetDouble());
				break;
			case 0:
				x = y = z = 0;
				break;
			default:
				ExpectUnreachable();
		}

		return true;
	}

	template<typename T>
	template<typename S, typename>
	bool TVector3<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);
		currentElement.Reserve(3, serializer.GetDocument().GetAllocator());

		const bool areAllEqual = (Math::Abs(TVector2<T>{x, x} - TVector2<T>{y, z}) <= TVector2<T>{Math::NumericLimits<T>::Epsilon}).AreAllSet();
		if (areAllEqual)
		{
			if (x != T(0))
			{
				currentElement.PushBack(x, serializer.GetDocument().GetAllocator());
			}
		}
		else
		{
			currentElement.PushBack(x, serializer.GetDocument().GetAllocator());
			currentElement.PushBack(y, serializer.GetDocument().GetAllocator());
			currentElement.PushBack(z, serializer.GetDocument().GetAllocator());
		}
		return true;
	}

	template<typename T>
	template<typename S, typename>
	bool TVector3<T>::Compress(BitView& target) const
	{
		if constexpr (TypeTraits::IsFloatingPoint<T>)
		{
			const Math::Range<UnitType> range = Math::Range<UnitType>::MakeStartToEnd(UnitType(-100000.0), UnitType(100000.0));
			constexpr uint32 bitCount = sizeof(UnitType) * 8;
			const Math::Vector3ui quantized = Math::Quantize(
				*this,
				Array<const Math::QuantizationMode, 3>{
					Math::QuantizationMode::Truncate,
					Math::QuantizationMode::Truncate,
					Math::QuantizationMode::Truncate
				}
					.GetView(),
				Array<const Math::Range<UnitType>, 3>{range, range, range}.GetView(),
				Array<const uint32, 3>{bitCount, bitCount, bitCount}
			);
			return target.PackAndSkip(ConstBitView::Make(quantized.x, Math::Range<size>::Make(0, bitCount))) &&
			       target.PackAndSkip(ConstBitView::Make(quantized.y, Math::Range<size>::Make(0, bitCount))) &&
			       target.PackAndSkip(ConstBitView::Make(quantized.z, Math::Range<size>::Make(0, bitCount)));
		}
		else
		{
		}
	}

	template<typename T>
	template<typename S, typename>
	bool TVector3<T>::Decompress(ConstBitView& source)
	{
		if constexpr (TypeTraits::IsFloatingPoint<T>)
		{
			Math::Vector3ui quantized;
			constexpr uint32 bitCount = sizeof(UnitType) * 8;
			const bool wasDecompressed = source.UnpackAndSkip(BitView::Make(quantized.x, Math::Range<size>::Make(0, bitCount))) &&
			                             source.UnpackAndSkip(BitView::Make(quantized.y, Math::Range<size>::Make(0, bitCount))) &&
			                             source.UnpackAndSkip(BitView::Make(quantized.z, Math::Range<size>::Make(0, bitCount)));
			const Math::Range<UnitType> range = Math::Range<UnitType>::MakeStartToEnd(UnitType(-100000.0), UnitType(100000.0));
			*this = Math::Dequantize(
				quantized,
				Array<const Math::Range<UnitType>, 3>{range, range, range}.GetView(),
				Array<const uint32, 3>{bitCount, bitCount, bitCount}
			);
			return wasDecompressed;
		}
		else
		{
		}
	}

	template bool TVector3<float>::Serialize(const Serialization::Reader);
	template bool TVector3<float>::Serialize(Serialization::Writer) const;
	template bool TVector3<float>::Compress(BitView&) const;
	template bool TVector3<float>::Decompress(ConstBitView&);

	template bool TVector3<int32>::Serialize(const Serialization::Reader);
	template bool TVector3<int32>::Serialize(Serialization::Writer) const;
	template bool TVector3<uint32>::Serialize(const Serialization::Reader);
	template bool TVector3<uint32>::Serialize(Serialization::Writer) const;

	template bool TVector3<TRatio<float>>::Serialize(const Serialization::Reader);
	template bool TVector3<TRatio<float>>::Serialize(Serialization::Writer) const;

	template<typename T>
	template<typename S, typename>
	bool TVector2<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		using ActualType = TypeTraits::InnermostType<T>;
		Assert(currentElement.IsArray());
		switch (currentElement.Size())
		{
			case 2:
				x = static_cast<ActualType>(currentElement[0].GetDouble());
				y = static_cast<ActualType>(currentElement[1].GetDouble());
				break;
			case 1:
				x = y = static_cast<ActualType>(currentElement[0].GetDouble());
				break;
			case 0:
				x = y = T(0);
				break;
			default:
				ExpectUnreachable();
		}

		return true;
	}

	template<typename T>
	template<typename S, typename>
	bool TVector2<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);
		currentElement.Reserve(2, serializer.GetDocument().GetAllocator());

		const bool areAllEqual = Math::Abs(x - y) <= Math::NumericLimits<T>::Epsilon;
		if (areAllEqual)
		{
			if (x != 0)
			{
				currentElement.PushBack(x, serializer.GetDocument().GetAllocator());
			}
		}
		else
		{
			currentElement.PushBack(x, serializer.GetDocument().GetAllocator());
			currentElement.PushBack(y, serializer.GetDocument().GetAllocator());
		}
		return true;
	}

	template<typename T>
	template<typename S, typename>
	bool TVector2<T>::Compress(BitView& target) const
	{
		if constexpr (TypeTraits::IsFloatingPoint<T>)
		{
			const Math::Range<UnitType> range = Math::Range<UnitType>::MakeStartToEnd(UnitType(-100000.0), UnitType(100000.0));
			const uint32 bitCount = Math::NumericLimits<UnitType>::NumBits;
			const Math::Vector2ui quantized = Math::Quantize(
				*this,
				Array<const Math::QuantizationMode, 2>{Math::QuantizationMode::Truncate, Math::QuantizationMode::Truncate}.GetView(),
				Array<const Math::Range<UnitType>, 2>{range, range}.GetView(),
				Array<const uint32, 2>{bitCount, bitCount}
			);
			return target.PackAndSkip(ConstBitView::Make(quantized.x, Math::Range<size>::Make(0, bitCount))) &&
			       target.PackAndSkip(ConstBitView::Make(quantized.y, Math::Range<size>::Make(0, bitCount)));
		}
		else
		{
		}
	}

	template<typename T>
	template<typename S, typename>
	bool TVector2<T>::Decompress(ConstBitView& source)
	{
		if constexpr (TypeTraits::IsFloatingPoint<T>)
		{
			Math::Vector2ui quantized;
			const uint32 bitCount = Math::NumericLimits<UnitType>::NumBits;
			const bool wasDecompressed = source.UnpackAndSkip(BitView::Make(quantized.x, Math::Range<size>::Make(0, bitCount))) &&
			                             source.UnpackAndSkip(BitView::Make(quantized.y, Math::Range<size>::Make(0, bitCount)));
			const Math::Range<UnitType> range = Math::Range<UnitType>::MakeStartToEnd(UnitType(-100000.0), UnitType(100000.0));
			*this = Math::Dequantize(
				quantized,
				Array<const Math::Range<UnitType>, 2>{range, range}.GetView(),
				Array<const uint32, 2>{bitCount, bitCount}
			);
			return wasDecompressed;
		}
		else
		{
		}
	}

	template bool TVector2<float>::Serialize(const Serialization::Reader);
	template bool TVector2<float>::Serialize(Serialization::Writer) const;
	template bool TVector2<float>::Compress(BitView&) const;
	template bool TVector2<float>::Decompress(ConstBitView&);

	template bool TVector2<int32>::Serialize(const Serialization::Reader);
	template bool TVector2<int32>::Serialize(Serialization::Writer) const;
	template bool TVector2<uint32>::Serialize(const Serialization::Reader);
	template bool TVector2<uint32>::Serialize(Serialization::Writer) const;

	template bool TVector2<TRatio<float>>::Serialize(const Serialization::Reader);
	template bool TVector2<TRatio<float>>::Serialize(Serialization::Writer) const;

	bool WorldCoordinate::Serialize(const Serialization::Reader serializer)
	{
		return BaseType::Serialize(serializer);
	}

	bool WorldCoordinate::Serialize(Serialization::Writer serializer) const
	{
		return BaseType::Serialize(serializer);
	}
}
