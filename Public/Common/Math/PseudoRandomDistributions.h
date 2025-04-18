#pragma once

#include <Common/Memory/Containers/FixedArrayView.h>
#include <Common/Math/Vector3.h>
#include <Common/Math/ForwardDeclarations/Vector3.h>

#include <Common/Math/SinCos.h>
#include <Common/Math/Acos.h>
#include <Common/Math/Angle.h>

namespace ngine::Math
{

	// https://stackoverflow.com/questions/9600801/evenly-distributing-n-points-on-a-sphere
	inline void UniformSphereDistributionUsingGoldenSpiral(uint32 N, ArrayView<Math::Vector3f> samples)
	{
		float den = Math::MultiplicativeInverse((float)N);
		float num = Math::PI.GetRadians() * (1.0f + Math::Sqrt(5.0f));

		for (uint32 i = 0; i < N; ++i)
		{
			float phi = Math::Acos(1.0f - 2.0f * ((float)i + 0.5f) * den);
			float theta = num * (float)i;

			float cosT;
			float sinT = Math::SinCos(theta, cosT);

			float cosP;
			float sinP = Math::SinCos(phi, cosP);

			samples[i] = {cosT * sinP, sinT * sinP, cosP};
		}
	}

	inline void UniformHemisphereDistributionUsingGoldenSpiral(uint32 N, ArrayView<Math::Vector3f> samples) //+Z oriented
	{
		float den = Math::MultiplicativeInverse((float)N);
		float num = Math::PI.GetRadians() * (1.0f + Math::Sqrt(5.0f));

		for (uint32 i = 0; i < N; ++i)
		{
			float phi = Math::Acos(1.0f - ((float)i + 0.5f) * den);
			float theta = num * (float)i;

			float cosT;
			float sinT = Math::SinCos(theta, cosT);

			float cosP;
			float sinP = Math::SinCos(phi, cosP);

			samples[i] = {cosT * sinP, sinT * sinP, cosP};
		}
	}
}
