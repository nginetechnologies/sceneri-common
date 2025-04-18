#pragma once

#include "ForwardDeclarations/Spline.h"

#include <Common/Memory/Containers/Vector.h>

#include <Common/Math/Power.h>
#include <Common/Math/Ratio.h>
#include <Common/Math/Vector4.h>
#include <Common/Math/Transform.h>
#include <Common/Math/Primitives/Line.h>
#include <Common/Math/Primitives/BoundingBox.h>
#include <Common/Math/LinearInterpolate.h>
#include <Common/Math/Wrap.h>
#include <Common/Math/Clamp.h>
#include <Common/Math/MathAssert.h>

namespace ngine::Math
{
	template<typename CoordinateType_>
	struct Spline
	{
		Spline() = default;
		Spline(Spline&&) = default;
		Spline& operator=(Spline&&) = default;
		explicit Spline(const Spline& other)
			: m_points(other.m_points)
			, m_isClosed(other.m_isClosed)
		{
		}
		Spline& operator=(const Spline& other)
		{
			m_points.Clear();
			m_points.CopyFrom(m_points.begin(), other.m_points.GetView());
			m_isClosed = other.m_isClosed;
			return *this;
		}
		~Spline() = default;

		using CoordinateType = CoordinateType_;
		using CoordinateUnitType = typename CoordinateType::UnitType;
		using TransformType = Math::TTransform<CoordinateType, float>;
		using BoundingBoxType = Math::TBoundingBox<CoordinateType>;

		struct Point
		{
			CoordinateType position;
			CoordinateType backward;
			CoordinateType forward;
			CoordinateType up;

			bool isBezierCurve = true;
		};

		using SizeType = uint32;
		using Container = Vector<Point, SizeType>;
		using View = typename Container::View;
		using ConstView = typename Container::ConstView;

		FORCE_INLINE void ReservePoints(const SizeType pointCount)
		{
			m_points.Reserve(pointCount);
		}

		[[nodiscard]] FORCE_INLINE SizeType GetPointCount() const
		{
			return m_points.GetSize();
		}

		[[nodiscard]] FORCE_INLINE bool HasPoints() const
		{
			return m_points.HasElements();
		}

		[[nodiscard]] FORCE_INLINE bool IsEmpty() const
		{
			return m_points.IsEmpty();
		}

		[[nodiscard]] FORCE_INLINE ArrayView<Point, SizeType> GetPoints()
		{
			return m_points.GetView();
		}

		[[nodiscard]] FORCE_INLINE ArrayView<const Point, SizeType> GetPoints() const
		{
			return m_points.GetView();
		}

		[[nodiscard]] inline void SetClosed(bool isClosed)
		{
			m_isClosed = isClosed;

			CorrectBezierCurve();
		}

		[[nodiscard]] inline bool IsClosed() const
		{
			return m_isClosed;
		}

		inline void EmplacePoint(
			const CoordinateType point,
			const CoordinateType backward,
			const CoordinateType forward,
			const CoordinateType up,
			const bool isBezierCurve = true
		)
		{
			MathAssert(up.IsUnit());
			m_points.EmplaceBack(Point{point, backward, forward, up, isBezierCurve});
		}

		inline void EmplacePoint(const CoordinateType point, const CoordinateType up, const bool isBezierCurve = true)
		{
			MathAssert(up.IsUnit());
			m_points.EmplaceBack(Point{point, point, point, up, isBezierCurve});

			const ArrayView<Point, SizeType> points = m_points.GetView();
			CorrectBezierCurveAtPoint(points.end() - 1, m_isClosed, points);
		}

		inline void InsertPoint(
			const typename Container::iterator iterator, const CoordinateType point, const CoordinateType up, const bool isBezierCurve = true
		)
		{
			const uint32 index = m_points.GetIteratorIndex(iterator);
			if (index > m_points.GetSize() - 1)
			{
				EmplacePoint(point, up, isBezierCurve);
				return;
			}

			Point copy = m_points[index];
			Point nextCopy;

			m_points[index] = Point{point, point, point, up, isBezierCurve};

			for (uint32 i = index + 1; i < m_points.GetSize(); i++)
			{
				nextCopy = m_points[i];
				m_points[i] = copy;
				copy = nextCopy;
			}
			EmplacePoint(copy.position, copy.backward, copy.forward, copy.up, copy.isBezierCurve);
			CorrectBezierCurve();
		}

		void UpdatePoint(
			const typename Container::iterator iterator, const CoordinateType relativeCoordinate, const CoordinateType relativeUpDirection
		)
		{
			Point& point = *iterator;
			point.position = relativeCoordinate;
			point.up = relativeUpDirection;
			OnPointModified(&point);
		}

		void UpdateLastPoint(const CoordinateType relativeCoordinate, const CoordinateType relativeUpDirection)
		{
			Point& lastPoint = GetPoints().GetLastElement();
			lastPoint.position = relativeCoordinate;
			lastPoint.up = relativeUpDirection;
			OnPointModified(&lastPoint);
		}

		inline void RemovePoint(const typename Container::iterator iterator)
		{
			MathAssert(m_points.GetSize() != 0);
			m_points.Remove(iterator);

			CorrectBezierCurve();
		}

		inline void OnPointModified(const typename Container::iterator iterator)
		{
			CorrectBezierCurveAtPoint(iterator, m_isClosed, m_points.GetView());
		}

		inline void Clear()
		{
			m_points.Clear();
		}

		[[nodiscard]] inline static CoordinateType
		GetBezierPositionBetweenPoints(const Point& __restrict point, const Point& __restrict nextPoint, const Math::Ratiof time)
		{
			const Math::Ratiof inverseTime = time.GetInverted();

			const Math::Vector4f factors{
				Math::Power(inverseTime, 3),
				3 * time * Math::Power(inverseTime, 2),
				3 * Math::Power(time, 2) * inverseTime,
				Math::Power(time, 3)
			};
			return point.position * factors.x + point.forward * factors.y + nextPoint.backward * factors.z + nextPoint.position * factors.w;
		}

		[[nodiscard]] inline static Math::Vector3f
		GetBezierDirectionBetweenPoints(const Point& __restrict point, const Point& __restrict nextPoint, const Math::Ratiof time)
		{
			const Math::Ratiof inverseTime = time.GetInverted();
			const Math::Vector4f
				factors{-(inverseTime * inverseTime), inverseTime * (inverseTime - 2 * time), time * (2 * inverseTime - time), time * time};
			const CoordinateType tangent = point.position * factors.x + point.forward * factors.y + nextPoint.backward * factors.z +
			                               nextPoint.position * factors.w;
			return tangent.GetNormalizedSafe(Math::Forward);
		}

		[[nodiscard]] inline static Math::Vector3f
		GetBezierNormalBetweenPoints(const Point& __restrict point, const Point& __restrict nextPoint, const Math::Ratiof time)
		{
			constexpr float distanceOffset = 0.001f;
			const CoordinateType previousPosition = GetBezierPositionBetweenPoints(point, nextPoint, time - distanceOffset);
			const CoordinateType nextPosition = GetBezierPositionBetweenPoints(point, nextPoint, time + distanceOffset);

			const CoordinateType distance = (nextPosition - previousPosition).GetNormalizedSafe(Math::Forward);

			const CoordinateType up = Math::LinearInterpolate(point.up, nextPoint.up, time).GetNormalizedSafe(Math::Up);
			return distance.Cross(up).GetNormalizedSafe(Math::Right);
		}

		[[nodiscard]] inline static Math::Matrix3x3f
		GetBezierRotationBetweenPoints(const Point& __restrict point, const Point& __restrict nextPoint, const Math::Ratiof time)
		{
			const Math::Vector3f direction = GetBezierDirectionBetweenPoints(point, nextPoint, time);
			Math::Vector3f normal = GetBezierNormalBetweenPoints(point, nextPoint, time);
			const Math::Vector3f bitangent = normal.Cross(direction).GetNormalizedSafe(Math::Up);
			// Ensure that normal is perpendicular to both
			normal = direction.Cross(bitangent).GetNormalizedSafe(Math::Right);
			return Math::Matrix3x3f{normal, direction, bitangent};
		}

		[[nodiscard]] inline static TransformType
		GetBezierTransformBetweenPoints(const Point& __restrict point, const Point& __restrict nextPoint, const Math::Ratiof time)
		{
			const Math::Matrix3x3f rotation = GetBezierRotationBetweenPoints(point, nextPoint, time);
			const CoordinateType location = GetBezierPositionBetweenPoints(point, nextPoint, time);
			return TransformType{rotation, location};
		}

		[[nodiscard]] inline static CoordinateUnitType
		CalculateLengthBetweenPoints(const Point& __restrict point, const Point& __restrict nextPoint, const uint32 numBezierSubdivisions = 32)
		{
			const Math::Ratiof subdividedSplice(Math::MultiplicativeInverse((float)numBezierSubdivisions));
			CoordinateUnitType splineLength = 0.f;

			CoordinateType startPosition = GetBezierPositionBetweenPoints(point, nextPoint, 0.f);

			Math::Ratiof splice = subdividedSplice;
			for (uint32 i = 0; i < numBezierSubdivisions; ++i, splice += subdividedSplice)
			{
				const CoordinateType endPosition = GetBezierPositionBetweenPoints(point, nextPoint, splice);
				splineLength += (startPosition - endPosition).GetLength();
				startPosition = endPosition;
			}

			return splineLength;
		}

		[[nodiscard]] uint32 CalculateSegmentCount(const uint32 numBezierSubdivisions = 32) const
		{
			const bool isClosed = m_isClosed;
			const ArrayView<const Point, SizeType> points = m_points.GetView();

			uint32 segmentCount = 0;

			for (typename Container::const_iterator it = points.begin(), endIt = it + GetIterationCount(isClosed, points); it != endIt; ++it)
			{
				const Point& __restrict point = *it;
				segmentCount += point.isBezierCurve ? numBezierSubdivisions : 1;
			}

			return segmentCount;
		}

		template<typename CallbackType>
		void IterateAdjustedSplinePoints(CallbackType&& callback, const uint32 numBezierSubdivisions = 32) const
		{
			const Math::Ratiof subdividedSplice(Math::MultiplicativeInverse((float)numBezierSubdivisions));

			const bool isClosed = m_isClosed;
			const ArrayView<const Point, SizeType> points = m_points.GetView();
			for (typename Container::const_iterator it = points.begin(), endIt = it + GetIterationCount(isClosed, points); it != endIt; ++it)
			{
				const Point& __restrict point = *it;
				const Point& __restrict nextPoint = *WrapIterator(it + 1, points, isClosed);

				if (point.isBezierCurve)
				{
					CoordinateType startPosition = GetBezierPositionBetweenPoints(point, nextPoint, 0.f);
					Math::Vector3f startDirection = GetBezierDirectionBetweenPoints(point, nextPoint, 0.f);
					Math::Vector3f startNormal = GetBezierNormalBetweenPoints(point, nextPoint, 0.f);
					if (startDirection.IsEquivalentTo(startNormal))
					{
						startNormal = Math::Up;
					}

					Math::Ratiof splice = subdividedSplice;
					for (uint32 i = 0; i < numBezierSubdivisions; ++i, splice += subdividedSplice)
					{
						const CoordinateType endPosition = GetBezierPositionBetweenPoints(point, nextPoint, splice);
						const Math::Vector3f endDirection = GetBezierDirectionBetweenPoints(point, nextPoint, splice);
						Math::Vector3f endNormal = GetBezierNormalBetweenPoints(point, nextPoint, splice);
						if (endDirection.IsEquivalentTo(endNormal))
						{
							endNormal = Math::Up;
						}

						callback(point, nextPoint, startPosition, endPosition, startDirection, startNormal);
						startPosition = endPosition;
						startDirection = endDirection;
						startNormal = endNormal;
					}
				}
				else
				{
					const Math::Vector3f startDirection = GetBezierDirectionBetweenPoints(point, nextPoint, 0.f);
					const Math::Vector3f startNormal = GetBezierNormalBetweenPoints(point, nextPoint, 0.f);
					callback(point, nextPoint, point.position, nextPoint.position, startDirection, startNormal);
				}
			}
		}

		[[nodiscard]] CoordinateUnitType CalculateSplineLength(const uint32 numBezierSubdivisions = 32) const
		{
			const Math::Ratiof subdividedSplice(Math::MultiplicativeInverse((float)numBezierSubdivisions));
			CoordinateUnitType splineLength = 0.f;

			const bool isClosed = m_isClosed;
			const ArrayView<const Point, SizeType> points = m_points.GetView();
			for (typename Container::const_iterator it = points.begin(), endIt = it + GetIterationCount(isClosed, points); it != endIt; ++it)
			{
				const Point& __restrict point = *it;
				const Point& __restrict nextPoint = *WrapIterator(it + 1, points, isClosed);

				if (point.isBezierCurve)
				{
					CoordinateType startPosition = GetBezierPositionBetweenPoints(point, nextPoint, 0.f);

					Math::Ratiof splice = subdividedSplice;
					for (uint32 i = 0; i < numBezierSubdivisions; ++i, splice += subdividedSplice)
					{
						const CoordinateType endPosition = GetBezierPositionBetweenPoints(point, nextPoint, splice);
						splineLength += (startPosition - endPosition).GetLength();
						startPosition = endPosition;
					}
				}
				else
				{
					splineLength += (point.position - nextPoint.position).GetLength();
				}
			}

			return splineLength;
		}

		CoordinateType CalculateClosestPoint(
			const CoordinateType comparedPoint, uint32& pointIndexOut, Math::Ratiof& ratioOut, const uint32 numBezierSubdivisions = 32
		) const
		{
			const bool isClosed = m_isClosed;
			const ArrayView<const Point, SizeType> points = m_points.GetView();
			MathAssert(points.GetSize() >= 2);

			const Math::Ratiof subdividedSplice(Math::MultiplicativeInverse((float)numBezierSubdivisions));

			CoordinateType closestPoint = comparedPoint;
			CoordinateUnitType closestPointDistanceSquared = Math::NumericLimits<CoordinateUnitType>::Max;
			uint32 closestPointIndex = pointIndexOut;
			Math::Ratiof closestPointRatio = ratioOut;

			for (typename Container::const_iterator it = points.begin(), endIt = it + GetIterationCount(isClosed, points); it != endIt; ++it)
			{
				const Point& __restrict point = *it;
				const Point& __restrict nextPoint = *WrapIterator(it + 1, points, isClosed);

				if (point.isBezierCurve)
				{
					CoordinateType startPosition = GetBezierPositionBetweenPoints(point, nextPoint, 0.f);

					Math::Ratiof splice = subdividedSplice;
					for (uint32 i = 0; i < numBezierSubdivisions; ++i, splice += subdividedSplice)
					{
						const CoordinateType endPosition = GetBezierPositionBetweenPoints(point, nextPoint, splice);
						const Math::TLine<CoordinateType> line = {startPosition, endPosition};
						const Math::Ratiof pointRatio = line.GetClosestPointRatio(comparedPoint);
						const CoordinateType coordinateOnLine = line.GetPointAtRatio(pointRatio);
						const CoordinateUnitType distanceSquared = (coordinateOnLine - comparedPoint).GetLengthSquared();
						if (distanceSquared < closestPointDistanceSquared)
						{
							closestPointDistanceSquared = distanceSquared;
							closestPoint = coordinateOnLine;
							closestPointIndex = points.GetIteratorIndex(it);
							closestPointRatio = subdividedSplice * (float)i + subdividedSplice * pointRatio;
						}
						startPosition = endPosition;
					}
				}
				else
				{
					const Math::TLine<CoordinateType> line = {point.position, nextPoint.position};
					const Math::Ratiof pointRatio = line.GetClosestPointRatio(comparedPoint);
					const CoordinateType coordinateOnLine = line.GetPointAtRatio(pointRatio);
					const CoordinateUnitType distanceSquared = (coordinateOnLine - comparedPoint).GetLengthSquared();
					if (distanceSquared < closestPointDistanceSquared)
					{
						closestPointDistanceSquared = distanceSquared;
						closestPoint = coordinateOnLine;
						closestPointIndex = points.GetIteratorIndex(it);
						closestPointRatio = pointRatio;
					}
				}
			}

			pointIndexOut = closestPointIndex;
			ratioOut = closestPointRatio;

			return closestPoint;
		}

		void CalculateClosestIndexAndDistance(
			const CoordinateType position, SizeType& indexOut, CoordinateUnitType& lengthOut, const uint32 numBezierSubdivisions = 32
		) const
		{
			SizeType segmentIndex = 0;
			Math::Ratiof segmentRatio = 0_percent;
			CalculateClosestPoint(position, segmentIndex, segmentRatio, numBezierSubdivisions);

			const ArrayView<const Point, SizeType> points = m_points.GetView();

			float splinePosition = 0.f;
			for (uint32 currentSegmentIndex = 0; currentSegmentIndex < segmentIndex; ++currentSegmentIndex)
			{
				splinePosition += CalculateLengthBetweenPoints(points[currentSegmentIndex], points[currentSegmentIndex + 1]);
			}

			MathAssert(segmentIndex != GetPointCount() - 1);
			splinePosition += CalculateLengthBetweenPoints(points[segmentIndex], points[segmentIndex + 1]) * segmentRatio;

			indexOut = segmentIndex;
			lengthOut = splinePosition;
		}

		[[nodiscard]] BoundingBoxType CalculateBoundingBox(const uint32 numBezierSubdivisions = 32) const
		{
			const Math::Ratiof subdividedSplice(Math::MultiplicativeInverse((float)numBezierSubdivisions));
			BoundingBoxType bounds{m_points.HasElements() ? m_points[0].position : Math::Zero};

			const bool isClosed = m_isClosed;
			const ArrayView<const Point, SizeType> points = m_points.GetView();
			for (typename Container::const_iterator it = points.begin(), endIt = it + GetIterationCount(isClosed, points); it != endIt; ++it)
			{
				const Point& __restrict point = *it;
				const Point& __restrict nextPoint = *WrapIterator(it + 1, points, isClosed);

				if (point.isBezierCurve)
				{
					CoordinateType startPosition = GetBezierPositionBetweenPoints(point, nextPoint, 0.f);
					bounds.Expand(startPosition);

					Math::Ratiof splice = subdividedSplice;
					for (uint32 i = 0; i < numBezierSubdivisions; ++i, splice += subdividedSplice)
					{
						const CoordinateType endPosition = GetBezierPositionBetweenPoints(point, nextPoint, splice);
						bounds.Expand(endPosition);
					}
				}
				else
				{
					bounds.Expand(point.position);
					bounds.Expand(nextPoint.position);
				}
			}

			return bounds;
		}

		[[nodiscard]] const Point& GetPoint(const SizeType segmentIndex) const
		{
			return *WrapIterator(m_points.begin() + segmentIndex, m_points.GetView(), m_isClosed);
		}

		[[nodiscard]] static FORCE_INLINE typename Container::const_iterator
		WrapIterator(const typename Container::const_iterator it, const typename Container::ConstView elements, const bool isClosed)
		{
			if (isClosed)
			{
				return Math::Wrap((const Point*)it, (const Point*)elements.begin(), (const Point*)elements.end() - 1);
			}
			else
			{
				return Math::Clamp((const Point*)it, (const Point*)elements.begin(), (const Point*)elements.end() - 1);
			}
		}
	protected:
		inline static void CorrectFirstPointBezierAngles(const typename Container::iterator it, const typename Container::ConstView elements)
		{
			MathAssert(it == elements.begin());
			Point& __restrict point = *it;
			point.backward = point.position;
			if (elements.GetSize() == 2)
			{
				const Point& __restrict nextPoint = *(it + 1);
				point.forward = point.position + (nextPoint.position - point.position) / 3;
			}
			else if (elements.GetSize() > 1)
			{
				const Point& __restrict nextPoint = *(it + 1);

				const CoordinateUnitType nextBackLength = (nextPoint.backward - point.position).GetLength();
				const CoordinateUnitType nextPositionDistance = (nextPoint.position - point.position).GetLength();

				MathAssert((nextBackLength != 0) & (nextPositionDistance != 0));
				const CoordinateUnitType divisor = nextBackLength / nextPositionDistance * 3;

				point.forward = point.position + (nextPoint.backward - point.position) / divisor;
			}
		}

		inline static void CorrectLastPointBezierAngles(const typename Container::iterator it, const typename Container::ConstView elements)
		{
			MathAssert(it == elements.end() - 1);
			Point& __restrict point = *it;
			point.forward = point.position;
			if (it > elements.begin())
			{
				const Point& __restrict previousPoint = *(it - 1);

				const CoordinateUnitType previousForwardLength = (previousPoint.forward - point.position).GetLength();
				const CoordinateUnitType previousPositionDistance = (previousPoint.position - point.position).GetLength();

				if (previousPositionDistance > 0.f)
				{
					const CoordinateUnitType divisor = previousForwardLength / previousPositionDistance * 3;
					if (divisor > 0.f)
					{
						point.backward = point.position + (previousPoint.forward - point.position) / divisor;
					}
					else
					{
						point.backward = point.position;
					}
				}
				else
				{
					point.backward = point.position;
				}
			}
		}

		inline static void CorrectInbetweenFirstAndLastPointBezierAngles(
			const typename Container::iterator it, const bool isClosed, const typename Container::ConstView elements
		)
		{
			Point& __restrict point = *it;
			const Point& __restrict previousPoint = *WrapIterator(it - 1, elements, isClosed);
			const Point& __restrict nextPoint = *WrapIterator(it + 1, elements, isClosed);

			const CoordinateUnitType nextPreviousPositionDistance = (nextPoint.position - previousPoint.position).GetLength();
			const CoordinateUnitType previousPointDistance = (previousPoint.position - point.position).GetLength();
			const CoordinateUnitType nextPointDistance = (nextPoint.position - point.position).GetLength();

			Expect(nextPreviousPositionDistance != 0);
			const CoordinateUnitType backMultiplier = (previousPointDistance / nextPreviousPositionDistance / 3);
			Expect(nextPreviousPositionDistance != 0);
			const CoordinateUnitType forwardMultiplier = (nextPointDistance / nextPreviousPositionDistance / 3);

			point.backward = point.position + (previousPoint.position - nextPoint.position) * backMultiplier;
			point.forward = point.position + (nextPoint.position - previousPoint.position) * forwardMultiplier;
		}

		[[nodiscard]] FORCE_INLINE static bool
		IsInbetweenFirstAndLastPoint(const typename Container::iterator it, const typename Container::ConstView elements)
		{
			return ((it > elements.begin()) & (it < elements.end() - 1));
		}

		inline void CorrectBezierCurve()
		{
			const bool isClosed = m_isClosed;
			const ArrayView<Point, SizeType> points = m_points.GetView();
			for (Point& point : points)
			{
				CorrectBezierCurveAtPoint(&point, isClosed, points);
			}
		}

		static void
		CorrectBezierCurveAtPoint(const typename Container::iterator it, const bool isClosed, const typename Container::ConstView elements)
		{
			if (isClosed)
			{
				CorrectInbetweenFirstAndLastPointBezierAngles(it, isClosed, elements);
				return;
			}

			MathAssert(elements.IsWithinBounds(it));

			{
				const typename Container::iterator previousPointIt = it - 1;
				if (previousPointIt == elements.begin())
				{
					CorrectFirstPointBezierAngles(previousPointIt, elements);
				}
				else if (previousPointIt > elements.begin())
				{
					CorrectInbetweenFirstAndLastPointBezierAngles(previousPointIt, isClosed, elements);
				}
			}

			if (it == elements.begin())
			{
				CorrectFirstPointBezierAngles(it, elements);
			}
			else if (it == elements.end() - 1)
			{
				CorrectLastPointBezierAngles(it, elements);
			}
			else
			{
				CorrectInbetweenFirstAndLastPointBezierAngles(it, isClosed, elements);
			}

			{
				const typename Container::iterator nextPointIt = it + 1;
				if (nextPointIt == elements.end() - 1)
				{
					CorrectLastPointBezierAngles(nextPointIt, elements);
				}
				else if (nextPointIt < elements.end())
				{
					CorrectInbetweenFirstAndLastPointBezierAngles(nextPointIt, isClosed, elements);
				}
			}

			{
				const typename Container::iterator secondPreviousPointIt = it - 2;
				if (secondPreviousPointIt == elements.begin())
				{
					CorrectFirstPointBezierAngles(secondPreviousPointIt, elements);
				}
				else if (secondPreviousPointIt > elements.begin())
				{
					CorrectInbetweenFirstAndLastPointBezierAngles(secondPreviousPointIt, isClosed, elements);
				}
			}

			{
				const typename Container::iterator secondNextPointIt = it + 2;
				if (secondNextPointIt == elements.end() - 1)
				{
					CorrectLastPointBezierAngles(secondNextPointIt, elements);
				}
				else if (secondNextPointIt < elements.end())
				{
					CorrectInbetweenFirstAndLastPointBezierAngles(secondNextPointIt, isClosed, elements);
				}
			}
		}
	protected:
		[[nodiscard]] FORCE_INLINE static SizeType GetIterationCount(const bool isClosed, const typename Container::ConstView points)
		{
			if (isClosed)
			{
				return points.GetSize();
			}
			else
			{
				return Math::Max(points.GetSize(), 1u) - 1;
			}
		}
	protected:
		Container m_points;
		bool m_isClosed = false;
	};
}
