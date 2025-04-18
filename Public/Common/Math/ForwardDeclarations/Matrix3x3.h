#pragma once

namespace ngine::Math
{
	template<typename T>
	struct TMatrix3x3;
	using Matrix3x3f = TMatrix3x3<float>;

	using WorldRotationMatrix = Matrix3x3f;
}
