#pragma once

#include "Matrix3x4.h"
#include "WorldCoordinate.h"
#include "Vector4.h"

#include "ForwardDeclarations/Matrix4x4.h"

#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI alignas(alignof(Vectorization::Packed<T, 4>)) TMatrix4x4
	{
	public:
		using Vector4Type = TVector4<T>;
		using Vector3Type = TVector3<T>;

		constexpr TMatrix4x4() noexcept
		{
		}

		constexpr FORCE_INLINE TMatrix4x4(IdentityType) noexcept
			: m_rows{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}
		{
		}

		constexpr FORCE_INLINE TMatrix4x4(ZeroType) noexcept
			: m_rows{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
		{
		}

		constexpr FORCE_INLINE TMatrix4x4(
			const T m00,
			const T m01,
			const T m02,
			const T m03,
			const T m10,
			const T m11,
			const T m12,
			const T m13,
			const T m20,
			const T m21,
			const T m22,
			const T m23,
			const T m30,
			const T m31,
			const T m32,
			const T m33
		) noexcept
			: m_rows{{m00, m01, m02, m03}, {m10, m11, m12, m13}, {m20, m21, m22, m23}, {m30, m31, m32, m33}}
		{
		}

		constexpr FORCE_INLINE TMatrix4x4(const TMatrix3x4<T>& matrix) noexcept
			: m_rows{
					{matrix.m_right.x, matrix.m_forward.x, matrix.m_up.x, matrix.m_location.x},
					{matrix.m_right.y, matrix.m_forward.y, matrix.m_up.y, matrix.m_location.y},
					{matrix.m_right.z, matrix.m_forward.z, matrix.m_up.z, matrix.m_location.z},
					{0.f, 0.f, 0.f, 1.f}
				}
		{
		}

		template<typename OtherType = T>
		constexpr FORCE_INLINE TMatrix4x4(const TMatrix4x4<OtherType>& matrix) noexcept
			: m_rows{(Vector4Type)matrix.m_rows[0], (Vector4Type)matrix.m_rows[1], (Vector4Type)matrix.m_rows[2], (Vector4Type)matrix.m_rows[3]}
		{
		}

		constexpr FORCE_INLINE
		TMatrix4x4(const Vector4Type row0, const Vector4Type row1, const Vector4Type row2, const Vector4Type row3) noexcept
			: m_rows{row0, row1, row2, row3}
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix4x4 GetInverted() const noexcept
		{
			const float* m = &reinterpret_cast<const float&>(m_rows[0].x);

			const auto getCofactor = [](float m0, float m1, float m2, float m3, float m4, float m5, float m6, float m7, float m8) -> float
			{
				return m0 * (m4 * m8 - m5 * m7) - m1 * (m3 * m8 - m5 * m6) + m2 * (m3 * m7 - m4 * m6);
			};

			// get cofactors of minor matrices
			const float cofactor0 = getCofactor(m[5], m[6], m[7], m[9], m[10], m[11], m[13], m[14], m[15]);
			const float cofactor1 = getCofactor(m[4], m[6], m[7], m[8], m[10], m[11], m[12], m[14], m[15]);
			const float cofactor2 = getCofactor(m[4], m[5], m[7], m[8], m[9], m[11], m[12], m[13], m[15]);
			const float cofactor3 = getCofactor(m[4], m[5], m[6], m[8], m[9], m[10], m[12], m[13], m[14]);

			// get determinant
			const float determinant = m[0] * cofactor0 - m[1] * cofactor1 + m[2] * cofactor2 - m[3] * cofactor3;

			// get rest of cofactors for adj(M)
			const float cofactor4 = getCofactor(m[1], m[2], m[3], m[9], m[10], m[11], m[13], m[14], m[15]);
			const float cofactor5 = getCofactor(m[0], m[2], m[3], m[8], m[10], m[11], m[12], m[14], m[15]);
			const float cofactor6 = getCofactor(m[0], m[1], m[3], m[8], m[9], m[11], m[12], m[13], m[15]);
			const float cofactor7 = getCofactor(m[0], m[1], m[2], m[8], m[9], m[10], m[12], m[13], m[14]);

			const float cofactor8 = getCofactor(m[1], m[2], m[3], m[5], m[6], m[7], m[13], m[14], m[15]);
			const float cofactor9 = getCofactor(m[0], m[2], m[3], m[4], m[6], m[7], m[12], m[14], m[15]);
			const float cofactor10 = getCofactor(m[0], m[1], m[3], m[4], m[5], m[7], m[12], m[13], m[15]);
			const float cofactor11 = getCofactor(m[0], m[1], m[2], m[4], m[5], m[6], m[12], m[13], m[14]);

			const float cofactor12 = getCofactor(m[1], m[2], m[3], m[5], m[6], m[7], m[9], m[10], m[11]);
			const float cofactor13 = getCofactor(m[0], m[2], m[3], m[4], m[6], m[7], m[8], m[10], m[11]);
			const float cofactor14 = getCofactor(m[0], m[1], m[3], m[4], m[5], m[7], m[8], m[9], m[11]);
			const float cofactor15 = getCofactor(m[0], m[1], m[2], m[4], m[5], m[6], m[8], m[9], m[10]);

			// build inverse matrix = adj(M) / det(M)
			// adjugate of M is the transpose of the cofactor matrix of M
			const float invDeterminant = Math::MultiplicativeInverse(determinant);

			return TMatrix4x4{
				invDeterminant * cofactor0,
				-invDeterminant * cofactor4,
				invDeterminant * cofactor8,
				-invDeterminant * cofactor12,

				-invDeterminant * cofactor1,
				invDeterminant * cofactor5,
				-invDeterminant * cofactor9,
				invDeterminant * cofactor13,

				invDeterminant * cofactor2,
				-invDeterminant * cofactor6,
				invDeterminant * cofactor10,
				-invDeterminant * cofactor14,

				-invDeterminant * cofactor3,
				invDeterminant * cofactor7,
				-invDeterminant * cofactor11,
				invDeterminant * cofactor15
			};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static TMatrix4x4
		CreateLookAt(const WorldCoordinate eye, const WorldCoordinate center, const Vector3Type up) noexcept
		{
			const Vector3Type back = (eye - center).GetNormalized();
			const Vector3Type x = (up.Cross(back)).GetNormalized();
			const Vector3Type y = back.Cross(x);
			const Vector3Type negativeEyeDot = -Vector3Type{eye.Dot(x), eye.Dot(y), eye.Dot(back)};

			return TMatrix4x4{
				Vector4Type{x.x, y.x, back.x, 0},
				Vector4Type{x.y, y.y, back.y, 0},
				Vector4Type{x.z, y.z, back.z, 0},
				Vector4Type{negativeEyeDot.x, negativeEyeDot.y, negativeEyeDot.z, 1.f},
			};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static TMatrix4x4
		CreatePerspective(const TAngle<T> verticalFieldOfView, const T aspect, const T nearPlane, const T farPlane) noexcept
		{
			const TAngle<T> tanHalfVerticalFov = (verticalFieldOfView / static_cast<T>(2)).Tan();
			const T inverseHalfVerticalFovTan = 1.0f / tanHalfVerticalFov.GetRadians();

			/*constexpr TMatrix4x4 clip =
			{
			  1.f, 0.f, 0.f, 0.f,
			  0.f, -1.f, 0.f, 0.f,
			  0.f, 0.f, 0.5f, 0.f,
			  0.f, 0.f, 0.5f, 1.f
			};*/

			constexpr TMatrix4x4 clip = {1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};

			return clip * TMatrix4x4{
											inverseHalfVerticalFovTan / aspect,
											0.f,
											0.f,
											0.f,
											0.f,
											inverseHalfVerticalFovTan,
											0.f,
											0.f,
											0.f,
											0.f,
											T(double(farPlane) / (double(nearPlane) - double(farPlane))),
											-1.f,
											0.f,
											0.f,
											T(double(nearPlane) * double(farPlane) / (double(nearPlane) - double(farPlane))),
											0.f
										};
		}

		[[nodiscard]] void InverseDepth()
		{
			m_rows[2].z = (1.0f + m_rows[2].z) * -1.f;
			m_rows[3].z *= -1.f;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static TMatrix4x4
		CreatePerspectiveInversedDepth(const TAngle<T> verticalFieldOfView, const T aspect, const T nearPlane, const T farPlane) noexcept
		{
			TMatrix4x4 matrix = CreatePerspective(verticalFieldOfView, aspect, nearPlane, farPlane);
			matrix.InverseDepth();
			return matrix;
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS static TMatrix4x4
		CreateOrthographic(const T left, const T right, const T bottom, const T top, const T nearPlane, const T farPlane)
		{
			return TMatrix4x4{
				T(2) / (right - left),
				0.f,
				0.f,
				0.f,
				0.f,
				T(2) / (top - bottom),
				0.f,
				0.f,
				0.f,
				0.f,
				-T(1) / (farPlane - nearPlane),
				0.f,
				-(right + left) / (right - left),
				-(top + bottom) / (top - bottom),
				-nearPlane / (farPlane - nearPlane),
				1.f
			};
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TMatrix4x4 GetTransposed() const noexcept
		{
			return {
				{m_rows[0].x, m_rows[1].x, m_rows[2].x, m_rows[3].x},
				{m_rows[0].y, m_rows[1].y, m_rows[2].y, m_rows[3].y},
				{m_rows[0].z, m_rows[1].z, m_rows[2].z, m_rows[3].z},
				{m_rows[0].w, m_rows[1].w, m_rows[2].w, m_rows[3].w}
			};
		}
		FORCE_INLINE constexpr void Transpose() noexcept
		{
			*this = GetTransposed();
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TMatrix4x4 operator*(const T scalar) const noexcept
		{
			return {m_rows[0] * scalar, m_rows[1] * scalar, m_rows[2] * scalar, m_rows[3] * scalar};
		};

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr TMatrix4x4 operator*(const Vector4Type factor) const noexcept
		{
			return {m_rows[0] * factor.x, m_rows[1] * factor.y, m_rows[2] * factor.z, m_rows[3] * factor.w};
		};

		Vector4Type m_rows[4];
	};

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix4x4<T>
	operator*(const TMatrix3x4<T>& left, const TMatrix4x4<T>& right) noexcept
	{
		TMatrix4x4<T> m(Identity);
		m.m_rows[0][0] = left.m_right.x * right.m_rows[0][0] + left.m_forward.x * right.m_rows[1][0] + left.m_up.x * right.m_rows[2][0] +
		                 left.m_location.x * right.m_rows[3][0];
		m.m_rows[1][0] = left.m_right.y * right.m_rows[0][0] + left.m_forward.y * right.m_rows[1][0] + left.m_up.y * right.m_rows[2][0] +
		                 left.m_location.y * right.m_rows[3][0];
		m.m_rows[2][0] = left.m_right.z * right.m_rows[0][0] + left.m_forward.z * right.m_rows[1][0] + left.m_up.z * right.m_rows[2][0] +
		                 left.m_location.z * right.m_rows[3][0];
		m.m_rows[3][0] = right.m_rows[3][0];
		m.m_rows[0][1] = left.m_right.x * right.m_rows[0][1] + left.m_forward.x * right.m_rows[1][1] + left.m_up.x * right.m_rows[2][1] +
		                 left.m_location.x * right.m_rows[3][1];
		m.m_rows[1][1] = left.m_right.y * right.m_rows[0][1] + left.m_forward.y * right.m_rows[1][1] + left.m_up.y * right.m_rows[2][1] +
		                 left.m_location.y * right.m_rows[3][1];
		m.m_rows[2][1] = left.m_right.z * right.m_rows[0][1] + left.m_forward.z * right.m_rows[1][1] + left.m_up.z * right.m_rows[2][1] +
		                 left.m_location.z * right.m_rows[3][1];
		m.m_rows[3][1] = right.m_rows[3][1];
		m.m_rows[0][2] = left.m_right.x * right.m_rows[0][2] + left.m_forward.x * right.m_rows[1][2] + left.m_up.x * right.m_rows[2][2] +
		                 left.m_location.x * right.m_rows[3][2];
		m.m_rows[1][2] = left.m_right.y * right.m_rows[0][2] + left.m_forward.y * right.m_rows[1][2] + left.m_up.y * right.m_rows[2][2] +
		                 left.m_location.y * right.m_rows[3][2];
		m.m_rows[2][2] = left.m_right.z * right.m_rows[0][2] + left.m_forward.z * right.m_rows[1][2] + left.m_up.z * right.m_rows[2][2] +
		                 left.m_location.z * right.m_rows[3][2];
		m.m_rows[3][2] = right.m_rows[3][2];
		m.m_rows[0][3] = left.m_right.x * right.m_rows[0][3] + left.m_forward.x * right.m_rows[1][3] + left.m_up.x * right.m_rows[2][3] +
		                 left.m_location.x * right.m_rows[3][3];
		m.m_rows[1][3] = left.m_right.y * right.m_rows[0][3] + left.m_forward.y * right.m_rows[1][3] + left.m_up.y * right.m_rows[2][3] +
		                 left.m_location.y * right.m_rows[3][3];
		m.m_rows[2][3] = left.m_right.z * right.m_rows[0][3] + left.m_forward.z * right.m_rows[1][3] + left.m_up.z * right.m_rows[2][3] +
		                 left.m_location.z * right.m_rows[3][3];
		m.m_rows[3][3] = right.m_rows[3][3];
		return m;
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix4x4<T>
	operator*(const TMatrix4x4<T>& left, const TMatrix3x4<T>& right) noexcept
	{
		TMatrix4x4<T> m(Identity);
		m.m_rows[0][0] = left.m_rows[0][0] * right.m_right.x + left.m_rows[0][1] * right.m_right.y + left.m_rows[0][2] * right.m_right.z;
		m.m_rows[1][0] = left.m_rows[1][0] * right.m_right.x + left.m_rows[1][1] * right.m_right.y + left.m_rows[1][2] * right.m_right.z;
		m.m_rows[2][0] = left.m_rows[2][0] * right.m_right.x + left.m_rows[2][1] * right.m_right.y + left.m_rows[2][2] * right.m_right.z;
		m.m_rows[3][0] = left.m_rows[3][0] * right.m_right.x + left.m_rows[3][1] * right.m_right.y + left.m_rows[3][2] * right.m_right.z;
		m.m_rows[0][1] = left.m_rows[0][0] * right.m_forward.x + left.m_rows[0][1] * right.m_forward.y + left.m_rows[0][2] * right.m_forward.z;
		m.m_rows[1][1] = left.m_rows[1][0] * right.m_forward.x + left.m_rows[1][1] * right.m_forward.y + left.m_rows[1][2] * right.m_forward.z;
		m.m_rows[2][1] = left.m_rows[2][0] * right.m_forward.x + left.m_rows[2][1] * right.m_forward.y + left.m_rows[2][2] * right.m_forward.z;
		m.m_rows[3][1] = left.m_rows[3][0] * right.m_forward.x + left.m_rows[3][1] * right.m_forward.y + left.m_rows[3][2] * right.m_forward.z;
		m.m_rows[0][2] = left.m_rows[0][0] * right.m_up.x + left.m_rows[0][1] * right.m_up.y + left.m_rows[0][2] * right.m_up.z;
		m.m_rows[1][2] = left.m_rows[1][0] * right.m_up.x + left.m_rows[1][1] * right.m_up.y + left.m_rows[1][2] * right.m_up.z;
		m.m_rows[2][2] = left.m_rows[2][0] * right.m_up.x + left.m_rows[2][1] * right.m_up.y + left.m_rows[2][2] * right.m_up.z;
		m.m_rows[3][2] = left.m_rows[3][0] * right.m_up.x + left.m_rows[3][1] * right.m_up.y + left.m_rows[3][2] * right.m_up.z;
		m.m_rows[0][3] = left.m_rows[0][0] * right.m_location.x + left.m_rows[0][1] * right.m_location.y +
		                 left.m_rows[0][2] * right.m_location.z + left.m_rows[0][3];
		m.m_rows[1][3] = left.m_rows[1][0] * right.m_location.x + left.m_rows[1][1] * right.m_location.y +
		                 left.m_rows[1][2] * right.m_location.z + left.m_rows[1][3];
		m.m_rows[2][3] = left.m_rows[2][0] * right.m_location.x + left.m_rows[2][1] * right.m_location.y +
		                 left.m_rows[2][2] * right.m_location.z + left.m_rows[2][3];
		m.m_rows[3][3] = left.m_rows[3][0] * right.m_location.x + left.m_rows[3][1] * right.m_location.y +
		                 left.m_rows[3][2] * right.m_location.z + left.m_rows[3][3];
		return m;
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TMatrix4x4<T>
	operator*(const TMatrix4x4<T>& left, const TMatrix4x4<T>& right) noexcept
	{
		TMatrix4x4<T> m(Identity);
		m.m_rows[0][0] = left.m_rows[0][0] * right.m_rows[0][0] + left.m_rows[0][1] * right.m_rows[1][0] +
		                 left.m_rows[0][2] * right.m_rows[2][0] + left.m_rows[0][3] * right.m_rows[3][0];
		m.m_rows[1][0] = left.m_rows[1][0] * right.m_rows[0][0] + left.m_rows[1][1] * right.m_rows[1][0] +
		                 left.m_rows[1][2] * right.m_rows[2][0] + left.m_rows[1][3] * right.m_rows[3][0];
		m.m_rows[2][0] = left.m_rows[2][0] * right.m_rows[0][0] + left.m_rows[2][1] * right.m_rows[1][0] +
		                 left.m_rows[2][2] * right.m_rows[2][0] + left.m_rows[2][3] * right.m_rows[3][0];
		m.m_rows[3][0] = left.m_rows[3][0] * right.m_rows[0][0] + left.m_rows[3][1] * right.m_rows[1][0] +
		                 left.m_rows[3][2] * right.m_rows[2][0] + left.m_rows[3][3] * right.m_rows[3][0];
		m.m_rows[0][1] = left.m_rows[0][0] * right.m_rows[0][1] + left.m_rows[0][1] * right.m_rows[1][1] +
		                 left.m_rows[0][2] * right.m_rows[2][1] + left.m_rows[0][3] * right.m_rows[3][1];
		m.m_rows[1][1] = left.m_rows[1][0] * right.m_rows[0][1] + left.m_rows[1][1] * right.m_rows[1][1] +
		                 left.m_rows[1][2] * right.m_rows[2][1] + left.m_rows[1][3] * right.m_rows[3][1];
		m.m_rows[2][1] = left.m_rows[2][0] * right.m_rows[0][1] + left.m_rows[2][1] * right.m_rows[1][1] +
		                 left.m_rows[2][2] * right.m_rows[2][1] + left.m_rows[2][3] * right.m_rows[3][1];
		m.m_rows[3][1] = left.m_rows[3][0] * right.m_rows[0][1] + left.m_rows[3][1] * right.m_rows[1][1] +
		                 left.m_rows[3][2] * right.m_rows[2][1] + left.m_rows[3][3] * right.m_rows[3][1];
		m.m_rows[0][2] = left.m_rows[0][0] * right.m_rows[0][2] + left.m_rows[0][1] * right.m_rows[1][2] +
		                 left.m_rows[0][2] * right.m_rows[2][2] + left.m_rows[0][3] * right.m_rows[3][2];
		m.m_rows[1][2] = left.m_rows[1][0] * right.m_rows[0][2] + left.m_rows[1][1] * right.m_rows[1][2] +
		                 left.m_rows[1][2] * right.m_rows[2][2] + left.m_rows[1][3] * right.m_rows[3][2];
		m.m_rows[2][2] = left.m_rows[2][0] * right.m_rows[0][2] + left.m_rows[2][1] * right.m_rows[1][2] +
		                 left.m_rows[2][2] * right.m_rows[2][2] + left.m_rows[2][3] * right.m_rows[3][2];
		m.m_rows[3][2] = left.m_rows[3][0] * right.m_rows[0][2] + left.m_rows[3][1] * right.m_rows[1][2] +
		                 left.m_rows[3][2] * right.m_rows[2][2] + left.m_rows[3][3] * right.m_rows[3][2];
		m.m_rows[0][3] = left.m_rows[0][0] * right.m_rows[0][3] + left.m_rows[0][1] * right.m_rows[1][3] +
		                 left.m_rows[0][2] * right.m_rows[2][3] + left.m_rows[0][3] * right.m_rows[3][3];
		m.m_rows[1][3] = left.m_rows[1][0] * right.m_rows[0][3] + left.m_rows[1][1] * right.m_rows[1][3] +
		                 left.m_rows[1][2] * right.m_rows[2][3] + left.m_rows[1][3] * right.m_rows[3][3];
		m.m_rows[2][3] = left.m_rows[2][0] * right.m_rows[0][3] + left.m_rows[2][1] * right.m_rows[1][3] +
		                 left.m_rows[2][2] * right.m_rows[2][3] + left.m_rows[2][3] * right.m_rows[3][3];
		m.m_rows[3][3] = left.m_rows[3][0] * right.m_rows[0][3] + left.m_rows[3][1] * right.m_rows[1][3] +
		                 left.m_rows[3][2] * right.m_rows[2][3] + left.m_rows[3][3] * right.m_rows[3][3];
		return m;
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector4<T> operator*(const TMatrix4x4<T>& matrix, const TVector4<T> vector) noexcept
	{
		return TVector4<T>{
			vector.x * matrix.m_rows[0][0] + vector.y * matrix.m_rows[0][1] + vector.z * matrix.m_rows[0][2] + vector.w * matrix.m_rows[0][3],
			vector.x * matrix.m_rows[1][0] + vector.y * matrix.m_rows[1][1] + vector.z * matrix.m_rows[1][2] + vector.w * matrix.m_rows[1][3],
			vector.x * matrix.m_rows[2][0] + vector.y * matrix.m_rows[2][1] + vector.z * matrix.m_rows[2][2] + vector.w * matrix.m_rows[2][3],
			vector.x * matrix.m_rows[3][0] + vector.y * matrix.m_rows[3][1] + vector.z * matrix.m_rows[3][2] + vector.w * matrix.m_rows[3][3]
		};
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS TVector4<T> operator*(const TVector4<T> vector, const TMatrix4x4<T>& matrix) noexcept
	{
		return TVector4<T>{
			vector.x * matrix.m_rows[0][0] + vector.y * matrix.m_rows[1][0] + vector.z * matrix.m_rows[2][0] + vector.w * matrix.m_rows[3][0],
			vector.x * matrix.m_rows[0][1] + vector.y * matrix.m_rows[1][1] + vector.z * matrix.m_rows[2][1] + vector.w * matrix.m_rows[3][1],
			vector.x * matrix.m_rows[0][2] + vector.y * matrix.m_rows[1][2] + vector.z * matrix.m_rows[2][2] + vector.w * matrix.m_rows[3][2],
			vector.x * matrix.m_rows[0][3] + vector.y * matrix.m_rows[1][3] + vector.z * matrix.m_rows[2][3] + vector.w * matrix.m_rows[3][3]
		};
	}
}
