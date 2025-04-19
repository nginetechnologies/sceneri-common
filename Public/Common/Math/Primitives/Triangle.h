#pragma once

#include <Common/Math/Vector2.h>
#include <Common/Math/Vector3.h>
#include <Common/Math/Tangents.h>
#include <Common/Math/Primitives/Line.h>
#include <Common/Math/Mod.h>
#include <Common/Math/MathAssert.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	struct WorldCoordinate;

	template<typename VectorType>
	struct TRIVIAL_ABI TTriangle
	{
		using UnitType = typename VectorType::UnitType;

		TTriangle() = default;
		constexpr TTriangle(const VectorType point0, const VectorType point1, const VectorType point2)
			: m_points{point0, point1, point2}
		{
		}

		[[nodiscard]] FORCE_INLINE constexpr VectorType& operator[](const uint8 index)
		{
			return m_points[index];
		}
		[[nodiscard]] FORCE_INLINE constexpr const VectorType operator[](const uint8 index) const
		{
			return m_points[index];
		}

		[[nodiscard]] FORCE_INLINE VectorType GetNormal() const
		{
			const VectorType u = m_points[1] - m_points[0];
			const VectorType v = m_points[2] - m_points[0];
			return u.Cross(v).GetNormalized();
		}

		[[nodiscard]] FORCE_INLINE UnitType GetArea2D() const
		{
			return Math::Abs(
							 m_points[0].x * (m_points[1].y - m_points[2].y) + m_points[1].x * (m_points[2].y - m_points[0].y) +
							 m_points[2].x * (m_points[0].y - m_points[1].y)
						 ) *
			       UnitType(0.5);
		}

		[[nodiscard]] FORCE_INLINE CompressedTangent CalculateTangents(const FixedArrayView<Math::Vector2f, 3> triangleUVs) const
		{
			const Math::Vector2f mappingU = triangleUVs[1] - triangleUVs[0];
			const Math::Vector2f mappingV = triangleUVs[2] - triangleUVs[0];

			const float uvArea = (mappingU.x * mappingV.y) - (mappingU.y * mappingV.x);
			const float orientationSign = Math::SignNonZero(uvArea);

			const Math::Vector3f positionU = m_points[1] - m_points[0];
			const Math::Vector3f positionV = m_points[2] - m_points[0];

			const Math::Vector3f tangent = (positionU * mappingV.y) - (positionV * mappingU.y);
			// const Math::Vector3f bitangent = (positionU * -mappingV.x) + (positionV * mappingU.x);
			const float tangentLength = tangent.GetLength();
			// const float bitangentLength = bitangent.GetLength();

			return CompressedTangent{
				(tangentLength > 0) ? (tangent / tangentLength) * orientationSign : Math::Zero,
				//(bitangentLength > 0) ? (bitangent / bitangentLength) * orientationSign : Math::Zero,
				orientationSign
			};
		}

		[[nodiscard]] FORCE_INLINE VectorType GetCenter() const
		{
			return (m_points[0] + m_points[1] + m_points[2]) * Math::MultiplicativeInverse(UnitType(3));
		}

		[[nodiscard]] FORCE_INLINE VectorType GetClosestPoint(const VectorType point) const
		{
			const VectorType diff = point - m_points[0];
			const VectorType edge0 = m_points[1] - m_points[0];
			const VectorType edge1 = m_points[2] - m_points[0];
			const UnitType a00 = edge0.Dot(edge0);
			
			const UnitType a01 = edge0.Dot(edge1);
			const UnitType a11 = edge1.Dot(edge1);
			const UnitType b0 = -diff.Dot(edge0);
			const UnitType b1 = -diff.Dot(edge1);

			const UnitType f00 = b0;
			const UnitType f10 = b0 + a00;
			const UnitType f01 = b0 + a01;

			TVector2<UnitType> p0, p1, p;
			UnitType dt1, h0, h1;

			// Compute the endpoints p0 and p1 of the segment.  The segment is
			// parameterized by L(z) = (1-z)*p0 + z*p1 for z in [0,1] and the
			// directional derivative of half the quadratic on the segment is
			// H(z) = Dot(p1-p0,gradient[Q](L(z))/2), where gradient[Q]/2 =
			// (F,G).  By design, F(L(z)) = 0 for cases (2), (4), (5), and
			// (6).  Cases (1) and (3) can correspond to no-intersection or
			// intersection of F = 0 with the triangle.
			if (f00 >= (UnitType)0)
			{
				if (f01 >= (UnitType)0)
				{
					// (1) p0 = (0,0), p1 = (0,1), H(z) = G(L(z))
					GetMinEdge02(a11, b1, p);
				}
				else
				{
					// (2) p0 = (0,t10), p1 = (t01,1-t01),
					// H(z) = (t11 - t10)*G(L(z))
					p0[0] = (UnitType)0;
					p0[1] = f00 / (f00 - f01);
					p1[0] = f01 / (f01 - f10);
					p1[1] = (UnitType)1 - p1[0];
					dt1 = p1[1] - p0[1];
					h0 = dt1 * (a11 * p0[1] + b1);
					if (h0 >= (UnitType)0)
					{
						GetMinEdge02(a11, b1, p);
					}
					else
					{
						h1 = dt1 * (a01 * p1[0] + a11 * p1[1] + b1);
						if (h1 <= (UnitType)0)
						{
							GetMinEdge12(a01, a11, b1, f10, f01, p);
						}
						else
						{
							GetMinInterior(p0, h0, p1, h1, p);
						}
					}
				}
			}
			else if (f01 <= (UnitType)0)
			{
				if (f10 <= (UnitType)0)
				{
					// (3) p0 = (1,0), p1 = (0,1), H(z) = G(L(z)) - F(L(z))
					GetMinEdge12(a01, a11, b1, f10, f01, p);
				}
				else
				{
					// (4) p0 = (t00,0), p1 = (t01,1-t01), H(z) = t11*G(L(z))
					p0[0] = f00 / (f00 - f10);
					p0[1] = (UnitType)0;
					p1[0] = f01 / (f01 - f10);
					p1[1] = (UnitType)1 - p1[0];
					h0 = p1[1] * (a01 * p0[0] + b1);
					if (h0 >= (UnitType)0)
					{
						p = p0; // GetMinEdge01
					}
					else
					{
						h1 = p1[1] * (a01 * p1[0] + a11 * p1[1] + b1);
						if (h1 <= (UnitType)0)
						{
							GetMinEdge12(a01, a11, b1, f10, f01, p);
						}
						else
						{
							GetMinInterior(p0, h0, p1, h1, p);
						}
					}
				}
			}
			else if (f10 <= (UnitType)0)
			{
				// (5) p0 = (0,t10), p1 = (t01,1-t01),
				// H(z) = (t11 - t10)*G(L(z))
				p0[0] = (UnitType)0;
				p0[1] = f00 / (f00 - f01);
				p1[0] = f01 / (f01 - f10);
				p1[1] = (UnitType)1 - p1[0];
				dt1 = p1[1] - p0[1];
				h0 = dt1 * (a11 * p0[1] + b1);
				if (h0 >= (UnitType)0)
				{
					GetMinEdge02(a11, b1, p);
				}
				else
				{
					h1 = dt1 * (a01 * p1[0] + a11 * p1[1] + b1);
					if (h1 <= (UnitType)0)
					{
						GetMinEdge12(a01, a11, b1, f10, f01, p);
					}
					else
					{
						GetMinInterior(p0, h0, p1, h1, p);
					}
				}
			}
			else
			{
				// (6) p0 = (t00,0), p1 = (0,t11), H(z) = t11*G(L(z))
				p0[0] = f00 / (f00 - f10);
				p0[1] = (UnitType)0;
				p1[0] = (UnitType)0;
				p1[1] = f00 / (f00 - f01);
				h0 = p1[1] * (a01 * p0[0] + b1);
				if (h0 >= (UnitType)0)
				{
					p = p0; // GetMinEdge01
				}
				else
				{
					h1 = p1[1] * (a11 * p1[1] + b1);
					if (h1 <= (UnitType)0)
					{
						GetMinEdge02(a11, b1, p);
					}
					else
					{
						GetMinInterior(p0, h0, p1, h1, p);
					}
				}
			}

			return m_points[0] + edge0 * p[0] + edge1 * p[1];
		}

		[[nodiscard]] FORCE_INLINE TLine<VectorType> GetEdge(const uint8 index) const
		{
			MathAssert(index < 3);
			return {m_points[index], m_points[Math::Mod(index + 1u, 3u)]};
		}
		[[nodiscard]] FORCE_INLINE Array<TLine<VectorType>, 3> GetEdges() const
		{
			return {
				TLine<VectorType>{m_points[0], m_points[1]},
				TLine<VectorType>{m_points[1], m_points[2]},
				TLine<VectorType>{m_points[2], m_points[0]}
			};
		}

		[[nodiscard]] FORCE_INLINE UnitType GetDistanceToPointSquared(const VectorType point) const
		{
			const VectorType v1 = m_points[1] - m_points[0];
			const VectorType v2 = m_points[2] - m_points[0];
			const VectorType normal = v1.Cross(v2).GetNormalized();
			const VectorType dist = point - m_points[0];
			const UnitType dotp = dist.Dot(normal);
			const VectorType intersect = normal * -dotp;
			return intersect.GetLengthSquared();
		}
	private:
		static void GetMinEdge02(const UnitType a11, const UnitType b1, Math::TVector2<UnitType>& p)
		{
			p[0] = (UnitType)0;
			if (b1 >= (UnitType)0)
			{
				p[1] = (UnitType)0;
			}
			else if (a11 + b1 <= (UnitType)0)
			{
				p[1] = (UnitType)1;
			}
			else
			{
				p[1] = -b1 / a11;
			}
		}

		static inline void GetMinEdge12(
			const UnitType a01, const UnitType a11, const UnitType b1, const UnitType f10, const UnitType f01, Math::TVector2<UnitType>& p
		)
		{
			UnitType h0 = a01 + b1 - f10;
			if (h0 >= (UnitType)0)
			{
				p[1] = (UnitType)0;
			}
			else
			{
				UnitType h1 = a11 + b1 - f01;
				if (h1 <= (UnitType)0)
				{
					p[1] = (UnitType)1;
				}
				else
				{
					p[1] = h0 / (h0 - h1);
				}
			}
			p[0] = (UnitType)1 - p[1];
		}

		static inline void GetMinInterior(
			const Math::TVector2<UnitType> p0,
			const UnitType h0,
			const Math::TVector2<UnitType> p1,
			const UnitType h1,
			Math::TVector2<UnitType>& p
		)
		{
			UnitType z = h0 / (h0 - h1);
			p = p0 * ((UnitType)1 - z) + p1 * z;
		}
	protected:
		VectorType m_points[3];
	};

	using Trianglef = TTriangle<TVector3<float>>;
	using WorldTriangle = TTriangle<WorldCoordinate>;
}
