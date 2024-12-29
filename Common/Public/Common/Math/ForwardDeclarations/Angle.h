#pragma once

namespace ngine::Math
{
	template<typename T>
	struct TAngle;
	using Anglef = TAngle<float>;
	using Angled = TAngle<double>;

	template<typename T>
	struct TEulerAngles;
	using EulerAnglesf = TEulerAngles<float>;
	using EulerAnglesd = TEulerAngles<double>;
}
