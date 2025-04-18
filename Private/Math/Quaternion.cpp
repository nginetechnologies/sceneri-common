#include "Common/Math/Quaternion.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <Common/Compression/Compress.h>
#include <Common/Compression/Decompress.h>
#include <Common/Compression/Bool.h>
#include <Common/Math/Vector3/Quantize.h>

namespace ngine::Math
{
	template<typename T>
	template<typename S, typename>
	bool TQuaternion<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsArray());
		Assert(currentElement.Size() == 3);
		Math::TEulerAngles<T> eulerAngles;
		eulerAngles.x = Math::TAngle<T>::FromDegrees(static_cast<T>(currentElement[0].GetDouble()));
		eulerAngles.y = Math::TAngle<T>::FromDegrees(static_cast<T>(currentElement[1].GetDouble()));
		eulerAngles.z = Math::TAngle<T>::FromDegrees(static_cast<T>(currentElement[2].GetDouble()));
		*this = TQuaternion<T>(eulerAngles);
		return true;
	}

	template<typename T>
	template<typename S, typename>
	bool TQuaternion<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(rapidjson::Type::kArrayType);
		currentElement.Reserve(3, serializer.GetDocument().GetAllocator());

		const TEulerAngles<T> eulerAngles = GetEulerAngles();
		currentElement.PushBack(static_cast<double>(eulerAngles.x.GetDegrees()), serializer.GetDocument().GetAllocator());
		currentElement.PushBack(static_cast<double>(eulerAngles.y.GetDegrees()), serializer.GetDocument().GetAllocator());
		currentElement.PushBack(static_cast<double>(eulerAngles.z.GetDegrees()), serializer.GetDocument().GetAllocator());
		return true;
	}

	template<typename T>
	bool TQuaternion<T>::Compress(BitView& target) const
	{
		ByteType maximumIndex{0};
		UnitType maximumValue{0};
		UnitType sign{1};

		const TQuaternion source = *this;
		for (uint8 i = 0; i < 4; ++i)
		{
			const UnitType value = source[i];
			const UnitType absoluteValue = Math::Abs(value);
			if (absoluteValue > maximumValue)
			{
				sign = Math::SignNonZero(value);
				maximumIndex = i;
				maximumValue = absoluteValue;
			}
		}

		// Write a bool indicating whether the maximum value is 1, meaning we only need to send the index
		const bool hasOneValue = false; // Math::IsEquivalentTo(maximumValue, UnitType(1), Math::NumericLimits<UnitType>::Epsilon);
		const bool wasBoolWritten = Compression::Compress(hasOneValue, target);
		// Now write 2 bits indicating the maximum index
		const bool wasMaximumIndexWritten = target.PackAndSkip(ConstBitView::Make(maximumIndex, Math::Range<size>::Make(0, 2)));
		if (hasOneValue)
		{
			return wasBoolWritten & wasMaximumIndexWritten;
		}

		Math::TVector3<UnitType> vector;
		switch (maximumIndex)
		{
			case 0:
				vector = {source.m_vector.y, source.m_vector.z, source.m_vector.w};
				break;
			case 1:
				vector = {source.m_vector.x, source.m_vector.z, source.m_vector.w};
				break;
			case 2:
				vector = {source.m_vector.x, source.m_vector.y, source.m_vector.w};
				break;
			case 3:
				vector = {source.m_vector.x, source.m_vector.y, source.m_vector.z};
				break;
		}
		vector = vector * sign;

		const Math::Range<UnitType> range = Math::Range<UnitType>::MakeStartToEnd(UnitType(-1.0), UnitType(1.0));
		constexpr uint32 axisBitCount = 12;
		const Math::Vector3ui quantizedAxis = Math::Quantize(
			vector,
			Array<const Math::QuantizationMode, 3>{
				Math::QuantizationMode::Truncate,
				Math::QuantizationMode::Truncate,
				Math::QuantizationMode::Truncate
			}
				.GetView(),
			Array<const Math::Range<UnitType>, 3>{range, range, range}.GetView(),
			Array<const uint32, 3>{axisBitCount, axisBitCount, axisBitCount}
		);
		return wasBoolWritten && wasMaximumIndexWritten &&
		       target.PackAndSkip(ConstBitView::Make(quantizedAxis.x, Math::Range<size>::Make(0, axisBitCount))) &&
		       target.PackAndSkip(ConstBitView::Make(quantizedAxis.y, Math::Range<size>::Make(0, axisBitCount))) &&
		       target.PackAndSkip(ConstBitView::Make(quantizedAxis.z, Math::Range<size>::Make(0, axisBitCount)));
	}

	template<typename T>
	bool TQuaternion<T>::Decompress(ConstBitView& source)
	{
		bool hasOneValue;
		const bool wasBoolRead = Compression::Decompress(hasOneValue, source);

		ByteType maximumIndex;
		const bool wasMaximumIndexRead = source.UnpackAndSkip(BitView::Make(maximumIndex, Math::Range<size>::Make(0, 2)));
		Expect(maximumIndex <= 3);

		if (hasOneValue)
		{
			switch (maximumIndex)
			{
				case 0:
					*this = {UnitType(1), 0, 0, 0};
					break;
				case 1:
					*this = {0, UnitType(1), 0, 0};
					break;
				case 2:
					*this = {0, 0, UnitType(1), 0};
					break;
				case 3:
					*this = {0, 0, 0, UnitType(1)};
					break;
			}
			return wasBoolRead & wasMaximumIndexRead;
		}
		else
		{
			Math::Vector3ui quantizedAxis;
			constexpr uint32 axisBitCount = 12;
			const bool wasDecompressed = source.UnpackAndSkip(BitView::Make(quantizedAxis.x, Math::Range<size>::Make(0, axisBitCount))) &&
			                             source.UnpackAndSkip(BitView::Make(quantizedAxis.y, Math::Range<size>::Make(0, axisBitCount))) &&
			                             source.UnpackAndSkip(BitView::Make(quantizedAxis.z, Math::Range<size>::Make(0, axisBitCount)));
			const Math::Range<UnitType> range = Math::Range<UnitType>::MakeStartToEnd(UnitType(-1.0), UnitType(1.0));
			const Math::TVector3<UnitType> axis = Math::Dequantize<UnitType>(
				quantizedAxis,
				Array<const Math::Range<UnitType>, 3>{range, range, range}.GetView(),
				Array<const uint32, 3>{axisBitCount, axisBitCount, axisBitCount}
			);

			const UnitType maximumAxis = Math::Sqrt(UnitType(1.0) - axis.GetLengthSquared());
			switch (maximumIndex)
			{
				case 0:
					*this = {maximumAxis, axis.x, axis.y, axis.z};
					break;
				case 1:
					*this = {axis.x, maximumAxis, axis.y, axis.z};
					break;
				case 2:
					*this = {axis.x, axis.y, maximumAxis, axis.z};
					break;
				case 3:
					*this = {axis.x, axis.y, axis.z, maximumAxis};
					break;
			}

			Normalize();
			return wasBoolRead & wasMaximumIndexRead & wasDecompressed;
		}
	}

	template struct TQuaternion<float>;

	template bool TQuaternion<float>::Serialize(const Serialization::Reader);
	template bool TQuaternion<float>::Serialize(Serialization::Writer) const;
}
