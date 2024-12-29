#pragma once

#include "Angle.h"
#include "Vector3.h"

#include <Common/Math/Vector3/Sin.h>
#include <Common/Math/Vector3/Tan.h>
#include <Common/Math/Vector3/Cos.h>
#include <Common/Math/Vector3/SinCos.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TAngle3 : public TVector3<TAngle<T>>
	{
		inline static constexpr Guid TypeGuid = "f1c999fc-2805-4aa7-b271-ec18dc00731e"_guid;

		using ContainedType = TAngle<T>;
		using BaseType = TVector3<TAngle<T>>;

		using BaseType::BaseType;

		FORCE_INLINE constexpr TAngle3(const Math::ZeroType) noexcept
			: BaseType(Zero, Zero, Zero)
		{
		}

		FORCE_INLINE constexpr TAngle3(const BaseType value) noexcept
			: BaseType(value)
		{
		}

		FORCE_INLINE constexpr TAngle3(const ContainedType _x, const ContainedType _y, const ContainedType _z) noexcept
			: BaseType(_x, _y, _z)
		{
		}

		[[nodiscard]] FORCE_INLINE static constexpr TAngle3 FromDegrees(const TVector3<T> value) noexcept
		{
			return {TAngle<T>::FromDegrees(value.x), TAngle<T>::FromDegrees(value.y), TAngle<T>::FromDegrees(value.z)};
		}

		[[nodiscard]] FORCE_INLINE static constexpr TAngle3 FromRadians(const TVector3<T> value) noexcept
		{
			return {TAngle<T>::FromRadians(value.x), TAngle<T>::FromRadians(value.y), TAngle<T>::FromRadians(value.z)};
		}

		[[nodiscard]] FORCE_INLINE TAngle3 Cos() const noexcept
		{
			return TAngle3::FromRadians(Math::Cos(ToRawRadians()));
		}
		[[nodiscard]] FORCE_INLINE TAngle3 Sin() const noexcept
		{
			return TAngle3::FromRadians(Math::Sin(ToRawRadians()));
		}
		[[nodiscard]] FORCE_INLINE TAngle3 Tan() const noexcept
		{
			return TAngle3::FromRadians(Math::Tan(ToRawRadians()));
		}

		struct SinCosResult
		{
			TAngle3 sin;
			TAngle3 cos;
		};
		[[nodiscard]] FORCE_INLINE SinCosResult SinCos() const noexcept
		{
			SinCosResult result;
			TVector3<T> cos;
			result.sin = TAngle3::FromRadians(Math::SinCos(ToRawRadians(), cos));
			result.cos = TAngle3::FromRadians(cos);
			return result;
		}

		[[nodiscard]] FORCE_INLINE TVector3<T> ToRawRadians() const noexcept
		{
			return {BaseType::x.GetRadians(), BaseType::y.GetRadians(), BaseType::z.GetRadians()};
		}

		[[nodiscard]] FORCE_INLINE TVector3<T> ToRawDegrees() const noexcept
		{
			return {BaseType::x.GetDegrees(), BaseType::y.GetDegrees(), BaseType::z.GetDegrees()};
		}
	};

	using Angle3f = TAngle3<float>;

	template<typename T>
	struct TEulerAngles : public TAngle3<T>
	{
		using BaseType = TAngle3<T>;

		using BaseType::BaseType;

		[[nodiscard]] FORCE_INLINE static constexpr TEulerAngles FromRadians(const TVector3<T> value) noexcept
		{
			return {TAngle<T>::FromRadians(value.x), TAngle<T>::FromRadians(value.y), TAngle<T>::FromRadians(value.z)};
		}
		[[nodiscard]] FORCE_INLINE static constexpr TEulerAngles FromDegrees(const TVector3<T> value) noexcept
		{
			return {TAngle<T>::FromDegrees(value.x), TAngle<T>::FromDegrees(value.y), TAngle<T>::FromDegrees(value.z)};
		}

		[[nodiscard]] FORCE_INLINE TEulerAngles operator*(const T scalar) const noexcept
		{
			return {BaseType::x * scalar, BaseType::y * scalar, BaseType::z * scalar};
		}
	};

	using EulerAnglesf = TEulerAngles<float>;

	template<typename T>
	struct TYawPitchRoll : public TAngle3<T>
	{
		using BaseType = TAngle3<T>;
		inline static constexpr Guid TypeGuid = "{B707FA04-2CEC-4887-B267-06C442D1F5EB}"_guid;

		FORCE_INLINE TYawPitchRoll() = default;

		TYawPitchRoll(const BaseType value)
			: BaseType(value)
		{
		}
		TYawPitchRoll(const Math::ZeroType)
			: BaseType(Math::Zero)
		{
		}
		TYawPitchRoll(const TAngle<T> yaw, const TAngle<T> pitch, const TAngle<T> roll) noexcept
			: BaseType(yaw, pitch, roll)
		{
		}

		[[nodiscard]] FORCE_INLINE TYawPitchRoll Cos() const noexcept
		{
			const TAngle3 value = BaseType::Cos();
			return TYawPitchRoll{value.x, value.y, value.z};
		}
		[[nodiscard]] FORCE_INLINE TYawPitchRoll Sin() const noexcept
		{
			const TAngle3 value = BaseType::Sin();
			return TYawPitchRoll{value.x, value.y, value.z};
		}
		[[nodiscard]] FORCE_INLINE TYawPitchRoll Tan() const noexcept
		{
			const TAngle3 value = BaseType::Tan();
			return TYawPitchRoll{value.x, value.y, value.z};
		}

		struct SinCosResult
		{
			TYawPitchRoll sin;
			TYawPitchRoll cos;
		};
		[[nodiscard]] FORCE_INLINE SinCosResult SinCos() const noexcept
		{
			const typename BaseType::SinCosResult value = BaseType::SinCos();
			return SinCosResult{TYawPitchRoll{value.sin.x, value.sin.y, value.sin.z}, TYawPitchRoll{value.cos.x, value.cos.y, value.cos.z}};
		}

		[[nodiscard]] FORCE_INLINE TAngle<T> GetYaw() const
		{
			return BaseType::x;
		}

		[[nodiscard]] FORCE_INLINE TAngle<T> GetPitch() const
		{
			return BaseType::y;
		}

		[[nodiscard]] FORCE_INLINE TAngle<T> GetRoll() const
		{
			return BaseType::z;
		}

		[[nodiscard]] FORCE_INLINE TAngle<T>& Yaw()
		{
			return BaseType::x;
		}

		[[nodiscard]] FORCE_INLINE TAngle<T>& Pitch()
		{
			return BaseType::y;
		}

		[[nodiscard]] FORCE_INLINE TAngle<T>& Roll()
		{
			return BaseType::z;
		}

		[[nodiscard]] FORCE_INLINE TVector3<T> ToRawRadians() const noexcept
		{
			return {BaseType::x.GetRadians(), BaseType::y.GetRadians(), BaseType::z.GetRadians()};
		}

		[[nodiscard]] FORCE_INLINE TVector3<T> ToRawDegrees() const noexcept
		{
			return {BaseType::x.GetDegrees(), BaseType::y.GetDegrees(), BaseType::z.GetDegrees()};
		}

		[[nodiscard]] FORCE_INLINE TYawPitchRoll operator*(const T scalar) const noexcept
		{
			return {BaseType::x * scalar, BaseType::y * scalar, BaseType::z * scalar};
		}

		[[nodiscard]] FORCE_INLINE TYawPitchRoll operator/(const T scalar) const noexcept
		{
			return {BaseType::x / scalar, BaseType::y / scalar, BaseType::z / scalar};
		}

		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const TYawPitchRoll other) const noexcept
		{
			return (BaseType::x == other.x) & (BaseType::y == other.y) & (BaseType::z == other.z);
		}

		[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const TYawPitchRoll other) const noexcept
		{
			return !operator==(other);
		}
	};

	using YawPitchRollf = TYawPitchRoll<float>;
}
