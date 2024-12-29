#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

#include <Common/Math/Ratio.h>
#include <Common/Math/Length.h>
#include <Common/Math/Mass.h>
#include <Common/Math/Density.h>
#include <Common/Math/Torque.h>
#include <Common/Math/RotationalSpeed.h>
#include <Common/Math/Speed.h>
#include <Common/Math/Acceleration.h>
#include <Common/Math/ClampedValue.h>
#include <Common/Math/Angle.h>
#include <Common/Math/Rotation2D.h>
#include <Common/Math/Color.h>

#include <Common/Memory/Containers/FlatString.h>
#include <Common/Memory/Containers/Format/String.h>

namespace ngine::Math
{
	template<typename T>
	bool TRatio<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = TRatio<T>(static_cast<T>(currentElement.GetDouble()));
		return true;
	}

	template<typename T>
	bool TRatio<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(m_ratio);
		return true;
	}

	template struct TRatio<float>;
	template struct TRatio<double>;

	template<typename T>
	bool Length<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = Length<T>::FromMeters(static_cast<T>(currentElement.GetDouble()));
		return true;
	}

	template<typename T>
	bool Length<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(GetMeters());
		return true;
	}

	template struct Length<float>;
	template struct Length<double>;

	template<typename T>
	bool Mass<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = Mass<T>::FromKilograms(static_cast<T>(currentElement.GetDouble()));
		return true;
	}

	template<typename T>
	bool Mass<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(GetKilograms());
		return true;
	}

	template struct Mass<float>;
	template struct Mass<double>;

	template<typename T>
	bool Density<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = Density<T>::FromKilogramsCubed(static_cast<T>(currentElement.GetDouble()));
		return true;
	}

	template<typename T>
	bool Density<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(GetKilogramsCubed());
		return true;
	}

	template struct Density<float>;
	template struct Density<double>;

	template<typename T>
	bool Torque<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = Torque<T>::FromNewtonMeters(static_cast<T>(currentElement.GetDouble()));
		return true;
	}

	template<typename T>
	bool Torque<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(GetNewtonMeters());
		return true;
	}

	template struct Torque<float>;
	template struct Torque<double>;

	template<typename T>
	inline bool TRotationalSpeed<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = TRotationalSpeed<T>::FromRadiansPerSecond(static_cast<T>(currentElement.GetDouble()));
		return true;
	}

	template<typename T>
	inline bool TRotationalSpeed<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(GetRadiansPerSecond());
		return true;
	}

	template struct TRotationalSpeed<float>;

	template<typename T>
	inline bool TSpeed<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = TSpeed<T>::FromMetersPerSecond(static_cast<T>(currentElement.GetDouble()));
		return true;
	}

	template<typename T>
	inline bool TSpeed<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(GetMetersPerSecond());
		return true;
	}

	template struct TSpeed<float>;

	template<typename T>
	inline bool TAcceleration<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = TAcceleration<T>::FromMetersPerSecondSquared(static_cast<T>(currentElement.GetDouble()));
		return true;
	}

	template<typename T>
	inline bool TAcceleration<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(GetMetersPerSecondSquared());
		return true;
	}

	template struct TAcceleration<float>;

	template<typename T>
	bool ClampedValue<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = static_cast<T>(currentElement.GetDouble());
		return true;
	}

	template<typename T>
	bool ClampedValue<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value((T) * this);
		return true;
	}

	template<>
	bool ClampedValue<uint64>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value((uint64_t) * this);
		return true;
	}
	template<>
	bool ClampedValue<int64>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value((int64_t) * this);
		return true;
	}

	template struct ClampedValue<float>;
	template struct ClampedValue<double>;
	template struct ClampedValue<uint8>;
	template struct ClampedValue<uint16>;
	template struct ClampedValue<uint32>;
	template struct ClampedValue<uint64>;
	template struct ClampedValue<int8>;
	template struct ClampedValue<int16>;
	template struct ClampedValue<int32>;
	template struct ClampedValue<int64>;

	template<typename T>
	bool TAngle<T>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsNumber());
		*this = TAngle<T>::FromDegrees(static_cast<T>(currentElement.GetDouble()));
		return true;
	}

	template<typename T>
	bool TAngle<T>::Serialize(Serialization::Writer serializer) const
	{
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(GetDegrees());
		return true;
	}

	template struct TAngle<float>;
	template struct TAngle<double>;
	template struct TRotation2D<float>;
	template struct TRotation2D<double>;

	template<>
	FlatString<10> TColor<uint8>::ToString() const
	{
		FlatString<10> formattedString;
		if (a != 255)
		{
			formattedString.Format("#{:02x}{:02x}{:02x}{:02x}", r, g, b, a);
		}
		else
		{
			if ((r == g) & (g == b))
			{
				formattedString.Format("#{:02x}", r);
			}
			else
			{
				formattedString.Format("#{:02x}{:02x}{:02x}", r, g, b);
			}
		}
		return formattedString;
	}
	template<>
	FlatString<10> TColor<float>::ToString() const
	{
		return TColor<uint8>(*this).ToString();
	}
	template<>
	FlatString<10> TColor<double>::ToString() const
	{
		return TColor<uint8>(*this).ToString();
	}

	template<>
	bool TColor<uint8>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		if (currentElement.IsArray())
		{
			switch (currentElement.Size())
			{
				case 1:
					r = g = b = static_cast<uint8>(currentElement[0].GetUint());
					a = 255;
					break;
				case 3:
					r = static_cast<uint8>(currentElement[0].GetUint());
					g = static_cast<uint8>(currentElement[1].GetUint());
					b = static_cast<uint8>(currentElement[2].GetUint());
					a = 255;
					break;
				case 4:
					r = static_cast<uint8>(currentElement[0].GetUint());
					g = static_cast<uint8>(currentElement[1].GetUint());
					b = static_cast<uint8>(currentElement[2].GetUint());
					a = static_cast<uint8>(currentElement[3].GetUint());
					break;
				default:
					Assert(false);
			}
			return true;
		}
		else if (currentElement.IsString())
		{
			if (const Optional<ColorByte> parsedColor = TColor<uint8>::TryParse(currentElement.GetString(), (uint8)currentElement.GetStringLength()))
			{
				*this = *parsedColor;
				return true;
			}
		}
		return false;
	}

	template<>
	bool TColor<double>::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		if (currentElement.IsArray())
		{
			switch (currentElement.Size())
			{
				case 1:
					r = g = b = currentElement[0].GetDouble();
					a = 1.0;
					break;
				case 3:
					r = currentElement[0].GetDouble();
					g = currentElement[1].GetDouble();
					b = currentElement[2].GetDouble();
					a = 1.0;
					break;
				case 4:
					r = currentElement[0].GetDouble();
					g = currentElement[1].GetDouble();
					b = currentElement[2].GetDouble();
					a = currentElement[3].GetDouble();
					break;
				default:
					Assert(false);
			}
			return true;
		}
		else if (currentElement.IsString())
		{
			if (const Optional<ColorByte> parsedColor = TColor<uint8>::TryParse(currentElement.GetString(), (uint8)currentElement.GetStringLength()))
			{
				const Math::Vector4d value =
					Math::Vector4d{(double)parsedColor->r, (double)parsedColor->g, (double)parsedColor->b, (double)parsedColor->a} / 255.0;
				*this = TColor(value.x, value.y, value.z, value.w);
				return true;
			}
		}
		return false;
	}

	template<>
	bool TColor<float>::Serialize(const Serialization::Reader serializer)
	{
		TColor<double> doubleColor{*this};
		const bool result = doubleColor.Serialize(serializer);
		*this = doubleColor;
		return result;
	}

	template<>
	bool TColor<uint8>::Serialize(Serialization::Writer serializer) const
	{
		FlatString<10> formattedString = ToString();
		return serializer.SerializeInPlace(formattedString);
	}

	template<>
	bool TColor<double>::Serialize(Serialization::Writer serializer) const
	{
		return Math::ColorByte{*this}.Serialize(serializer);
	}

	template<>
	bool TColor<float>::Serialize(Serialization::Writer serializer) const
	{
		return Math::ColorByte{*this}.Serialize(serializer);
	}
	template struct TColor<uint8>;
	template struct TColor<float>;
	template struct TColor<double>;
}
