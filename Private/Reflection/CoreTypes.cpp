#include <Common/Reflection/CoreTypes.h>

#include <Common/Math/Serialization/Angle3.h>

#include <Common/Math/Acceleration.h>
#include <Common/Math/Angle.h>
#include <Common/Math/Density.h>
#include <Common/Math/Length.h>
#include <Common/Math/Mass.h>
#include <Common/Math/Radius.h>
#include <Common/Math/Ratio.h>
#include <Common/Math/RotationalSpeed.h>
#include <Common/Math/Speed.h>
#include <Common/Math/Torque.h>
#include <Common/Math/Color.h>
#include <Common/Math/Quaternion.h>
#include <Common/Math/Rotation2D.h>
#include <Common/Math/Transform2D.h>
#include <Common/Math/Transform.h>

#include <Common/Memory/Containers/Compression/ArrayView.h>
#include <Common/Memory/Containers/Compression/Vector.h>

#include <Common/Reflection/Type.h>
#include <Common/Reflection/Registry.inl>

namespace ngine::Reflection
{
	[[maybe_unused]] const bool wasBoolTypeRegistered = Reflection::Registry::RegisterType<bool>();
	[[maybe_unused]] const bool wasFloatTypeRegistered = Reflection::Registry::RegisterType<float>();
	[[maybe_unused]] const bool wasDoubleTypeRegistered = Reflection::Registry::RegisterType<double>();
	[[maybe_unused]] const bool wasIntegerTypeRegistered = Reflection::Registry::RegisterType<int32>();
	[[maybe_unused]] const bool wasInteger64TypeRegistered = Reflection::Registry::RegisterType<int64>();
	[[maybe_unused]] const bool wasNullptrTypeRegistered = Reflection::Registry::RegisterType<nullptr_type>();
	[[maybe_unused]] const bool wasAnyTypeRegistered = Reflection::Registry::RegisterType<Any>();

	template<>
	struct ReflectedType<Math::Accelerationf>
	{
		static constexpr auto Type =
			Reflection::Reflect<Math::Accelerationf>(Math::Accelerationf::TypeGuid, MAKE_UNICODE_LITERAL("Acceleration"));
	};
	[[maybe_unused]] const bool wasAccelerationTypeRegistered = Reflection::Registry::RegisterType<Math::Accelerationf>();

	template<>
	struct ReflectedType<Math::Anglef>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Anglef>(Math::Anglef::TypeGuid, MAKE_UNICODE_LITERAL("Angle"));
	};
	[[maybe_unused]] const bool wasAngleTypeRegistered = Reflection::Registry::RegisterType<Math::Anglef>();

	template<>
	struct ReflectedType<Math::Densityf>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Densityf>(Math::Densityf::TypeGuid, MAKE_UNICODE_LITERAL("Density"));
	};
	[[maybe_unused]] const bool wasDensityTypeRegistered = Reflection::Registry::RegisterType<Math::Densityf>();

	template<>
	struct ReflectedType<Math::Lengthf>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Lengthf>(Math::Lengthf::TypeGuid, MAKE_UNICODE_LITERAL("Length"));
	};
	[[maybe_unused]] const bool wasLengthTypeRegistered = Reflection::Registry::RegisterType<Math::Lengthf>();

	template<>
	struct ReflectedType<Math::Massf>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Massf>(Math::Massf::TypeGuid, MAKE_UNICODE_LITERAL("Mass"));
	};
	[[maybe_unused]] const bool wasMassTypeRegistered = Reflection::Registry::RegisterType<Math::Massf>();

	template<>
	struct ReflectedType<Math::Ratiof>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Ratiof>(Math::Ratiof::TypeGuid, MAKE_UNICODE_LITERAL("Ratio"));
	};
	[[maybe_unused]] const bool wasRatioTypeRegistered = Reflection::Registry::RegisterType<Math::Ratiof>();

	template<>
	struct ReflectedType<Math::Radiusf>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Radiusf>(Math::Radiusf::TypeGuid, MAKE_UNICODE_LITERAL("Radius"));
	};
	[[maybe_unused]] const bool wasRadiusTypeRegistered = Reflection::Registry::RegisterType<Math::Radiusf>();

	template<>
	struct ReflectedType<Math::RotationalSpeedf>
	{
		static constexpr auto Type =
			Reflection::Reflect<Math::RotationalSpeedf>(Math::RotationalSpeedf::TypeGuid, MAKE_UNICODE_LITERAL("Rotational Speed"));
	};
	[[maybe_unused]] const bool wasRotationalSpeedTypeRegistered = Reflection::Registry::RegisterType<Math::RotationalSpeedf>();

	template<>
	struct ReflectedType<Math::Speedf>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Speedf>(Math::Speedf::TypeGuid, MAKE_UNICODE_LITERAL("Speed"));
	};
	[[maybe_unused]] const bool wasSpeedTypeRegistered = Reflection::Registry::RegisterType<Math::Speedf>();

	template<>
	struct ReflectedType<Math::Torquef>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Torquef>(Math::Torquef::TypeGuid, MAKE_UNICODE_LITERAL("Torque"));
	};
	[[maybe_unused]] const bool wasTorqueTypeRegistered = Reflection::Registry::RegisterType<Math::Torquef>();

	template<>
	struct ReflectedType<Math::Color>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Color>(Math::Color::TypeGuid, MAKE_UNICODE_LITERAL("Color"));
	};
	[[maybe_unused]] const bool wasColorTypeRegistered = Reflection::Registry::RegisterType<Math::Color>();

	template<>
	struct ReflectedType<Math::Vector2i>
	{
		static constexpr auto Type =
			Reflection::Reflect<Math::Vector2i>("f86e664b-5436-4698-8fc7-c8be9116af85"_guid, MAKE_UNICODE_LITERAL("Vector2i"));
	};
	[[maybe_unused]] const bool wasVector2iTypeRegistered = Reflection::Registry::RegisterType<Math::Vector2i>();

	template<>
	struct ReflectedType<Math::Vector2b>
	{
		static constexpr auto Type =
			Reflection::Reflect<Math::Vector2b>("c1760e9c-58c4-482d-ab36-99bd9d1fba53"_guid, MAKE_UNICODE_LITERAL("Vector2b"));
	};
	[[maybe_unused]] const bool wasVector2bTypeRegistered = Reflection::Registry::RegisterType<Math::Vector2b>();

	template<>
	struct ReflectedType<Math::Vector2f>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Vector2f>(Math::Vector2f::TypeGuid, MAKE_UNICODE_LITERAL("Vector2f"));
	};
	[[maybe_unused]] const bool wasVector2fTypeRegistered = Reflection::Registry::RegisterType<Math::Vector2f>();

	template<>
	struct ReflectedType<Math::Vector3i>
	{
		static constexpr auto Type =
			Reflection::Reflect<Math::Vector3i>("5704b02f-068b-40ed-a3cd-e4d294514dc9"_guid, MAKE_UNICODE_LITERAL("Vector3i"));
	};
	[[maybe_unused]] const bool wasVector3iTypeRegistered = Reflection::Registry::RegisterType<Math::Vector3i>();

	template<>
	struct ReflectedType<Math::Vector3b>
	{
		static constexpr auto Type =
			Reflection::Reflect<Math::Vector3b>("3dae0648-d28e-49d4-b1f9-30806a14f27d"_guid, MAKE_UNICODE_LITERAL("Vector3b"));
	};
	[[maybe_unused]] const bool wasVector3bTypeRegistered = Reflection::Registry::RegisterType<Math::Vector3b>();

	template<>
	struct ReflectedType<Math::Vector3f>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Vector3f>(Math::Vector3f::TypeGuid, MAKE_UNICODE_LITERAL("Vector3f"));
	};
	[[maybe_unused]] const bool wasVector3fTypeRegistered = Reflection::Registry::RegisterType<Math::Vector3f>();

	template<>
	struct ReflectedType<Math::Vector4i>
	{
		static constexpr auto Type =
			Reflection::Reflect<Math::Vector4i>("0be2039b-58fc-48ac-a5d8-2fd25956c386"_guid, MAKE_UNICODE_LITERAL("Vector4i"));
	};
	[[maybe_unused]] const bool wasVector4ifTypeRegistered = Reflection::Registry::RegisterType<Math::Vector4i>();

	template<>
	struct ReflectedType<Math::Vector4b>
	{
		static constexpr auto Type =
			Reflection::Reflect<Math::Vector4b>("26d6b1f0-207d-427d-8080-d62c8cc59e5f"_guid, MAKE_UNICODE_LITERAL("Vector4b"));
	};
	[[maybe_unused]] const bool wasVector4bTypeRegistered = Reflection::Registry::RegisterType<Math::Vector4b>();

	template<>
	struct ReflectedType<Math::Vector4f>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Vector4f>(Math::Vector4f::TypeGuid, MAKE_UNICODE_LITERAL("Vector4f"));
	};
	[[maybe_unused]] const bool wasVector4fTypeRegistered = Reflection::Registry::RegisterType<Math::Vector4f>();

	template<>
	struct ReflectedType<Math::Rotation2Df>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Rotation2Df>(Math::Rotation2Df::TypeGuid, MAKE_UNICODE_LITERAL("Rotation2D"));
	};
	[[maybe_unused]] const bool wasRotation2DTypeRegistered = Reflection::Registry::RegisterType<Math::Rotation2Df>();

	template<>
	struct ReflectedType<Math::Rotation3Df>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Rotation3Df>(Math::Rotation3Df::TypeGuid, MAKE_UNICODE_LITERAL("Rotation3D"));
	};
	[[maybe_unused]] const bool wasRotation3DTypeRegistered = Reflection::Registry::RegisterType<Math::Rotation3Df>();

	template<>
	struct ReflectedType<Math::Transform2Df>
	{
		static constexpr auto Type = Reflection::Reflect<Math::Transform2Df>(Math::Transform2Df::TypeGuid, MAKE_UNICODE_LITERAL("Transform2D"));
	};
	[[maybe_unused]] const bool wasTransform2DTypeRegistered = Reflection::Registry::RegisterType<Math::Transform2Df>();

	template<>
	struct ReflectedType<Math::Transform3Df>
	{
		static constexpr auto Type =
			Reflection::Reflect<Math::Transform3Df>(Math::WorldTransform::TypeGuid, MAKE_UNICODE_LITERAL("Transform3D"));
	};
	[[maybe_unused]] const bool wasTransform3DTypeRegistered = Reflection::Registry::RegisterType<Math::Transform3Df>();

	template<>
	struct ReflectedType<String>
	{
		static constexpr auto Type = Reflection::Reflect<String>("44f13895-44aa-40d5-9094-4523ca821aa4"_guid, MAKE_UNICODE_LITERAL("String"));
	};
	[[maybe_unused]] const bool wasStringTypeRegistered = Reflection::Registry::RegisterType<String>();

	template<>
	struct ReflectedType<UnicodeString>
	{
		static constexpr auto Type =
			Reflection::Reflect<UnicodeString>("5871a956-f4bf-4426-9b32-0c191d9c205d"_guid, MAKE_UNICODE_LITERAL("Unicode String"));
	};
	[[maybe_unused]] const bool wasUnicodeStringTypeRegistered = Reflection::Registry::RegisterType<UnicodeString>();

	template<>
	struct ReflectedType<ConstStringView>
	{
		static constexpr auto Type =
			Reflection::Reflect<ConstStringView>("40f4d72e-1790-44cc-8b8a-0b76154f1c95"_guid, MAKE_UNICODE_LITERAL("String View"));
	};
	[[maybe_unused]] const bool wasStringViewTypeRegistered = Reflection::Registry::RegisterType<ConstStringView>();

	template<>
	struct ReflectedType<Math::YawPitchRollf>
	{
		static constexpr auto Type =
			Reflection::Reflect<Math::YawPitchRollf>("b0a13e65-fb77-430a-83fa-fe13a82e2903"_guid, MAKE_UNICODE_LITERAL("Yaw Pitch Roll"));
	};
	[[maybe_unused]] const bool wasYawPitchRollTypeRegistered = Reflection::Registry::RegisterType<Math::YawPitchRollf>();

	template<>
	struct ReflectedType<Guid>
	{
		inline static constexpr auto Type =
			Reflection::Reflect<Guid>("{65874B56-5D2A-4F8B-9F31-8296FBDE246D}"_guid, MAKE_UNICODE_LITERAL("Guid"));
	};
	[[maybe_unused]] const bool wasGuidTypeRegistered = Reflection::Registry::RegisterType<Guid>();
}
