#pragma once

#include <Common/Threading/Mutexes/SharedMutex.h>
#include <Common/Threading/AtomicInteger.h>
#include <Common/Threading/AtomicPtr.h>
#include <Common/Math/Primitives/Rectangle.h>
#include <Common/Math/Vector2.h>
#include <Common/Math/Vector3.h>
#include <Common/Math/IsEquivalentTo.h>
#include <Common/Math/Radius.h>
#include <Common/AtomicEnumFlags.h>

#include <algorithm>

namespace ngine
{
	template<typename ElementType>
	struct QuadTree;

	inline float Square(const float x)
	{
		return x * x;
	}

	template<typename ElementType_>
	struct QuadTreeNode
	{
		using ElementType = ElementType_;
		using StoredChildType = Threading::Atomic<QuadTreeNode*>;
		using ChildContainerType = Array<StoredChildType, 4>;

		QuadTreeNode(QuadTreeNode* pParent, const uint8 parentChildIndex, const Math::Vector2f centerCoordinate, const Math::Radiusf radius)
			: m_childContentArea(CalculateStaticContentArea(centerCoordinate, radius))
			, m_centerCoordinateAndRadiusSquared{centerCoordinate.x, centerCoordinate.y, radius.GetMeters() * radius.GetMeters()}
			, m_pParentNode(pParent)
			, m_packedInfo(parentChildIndex)
		{
		}
		~QuadTreeNode()
		{
			m_flags |= Flags::Deleted;
		}

		void Resize(const Math::Vector2f centerCoordinate, const Math::Radiusf radius)
		{
			const float radiusSquared = radius.GetMeters() * radius.GetMeters();
			Assert(!GetCenterCoordinate().IsEquivalentTo(centerCoordinate) || !Math::IsEquivalentTo(GetRadiusSquared(), radiusSquared));
			const Math::Rectanglef newArea = CalculateStaticContentArea(centerCoordinate, radius);

			m_centerCoordinateAndRadiusSquared = Math::Vector3f{centerCoordinate.x, centerCoordinate.y, radiusSquared};
			m_childContentArea = m_childContentArea.Merge(newArea);

			for (uint8 childIndex = 0; childIndex < 4; ++childIndex)
			{
				QuadTreeNode* pChildNode = m_children[childIndex];
				if ((pChildNode == nullptr) | (pChildNode == reinterpret_cast<QuadTreeNode*>(static_cast<uintptr>(0xDEADBEEF))))
				{
					continue;
				}
				const Math::Rectanglef childArea = GetChildContentArea(childIndex, newArea);
				pChildNode->Resize(childArea, radius * 0.5f);
			}
		}

		[[nodiscard]] PURE_STATICS bool IsFlaggedForDeletion() const
		{
			return m_flags.IsSet(Flags::FlaggedForDeletion);
		}
		[[nodiscard]] PURE_STATICS bool WasDeleted() const
		{
			return m_flags.IsSet(Flags::Deleted);
		}

		[[nodiscard]] bool FlagForDeletion()
		{
			Assert(IsEmpty());
			const EnumFlags<Flags> previousFlags = m_flags.FetchOr(Flags::FlaggedForDeletion);
			return !previousFlags.IsSet(Flags::FlaggedForDeletion);
		}

		[[nodiscard]] PURE_STATICS float GetRadiusSquared() const
		{
			return m_centerCoordinateAndRadiusSquared.z;
		}
		[[nodiscard]] PURE_STATICS Math::Vector2f GetCenterCoordinate() const
		{
			return {m_centerCoordinateAndRadiusSquared.x, m_centerCoordinateAndRadiusSquared.y};
		}
		[[nodiscard]] static Math::Rectanglef CalculateStaticContentArea(const Math::Vector2f centerCoordinate, const Math::Radiusf nodeRadius)
		{
			return {centerCoordinate - Math::Vector2f(nodeRadius.GetMeters()), Math::Vector2f(nodeRadius.GetMeters()) * 2.f};
		}
		[[nodiscard]] PURE_STATICS Math::Rectanglef GetContentArea() const
		{
			return CalculateStaticContentArea(GetCenterCoordinate(), Math::Radiusf::FromMeters(Math::Sqrt(GetRadiusSquared())));
		}
		[[nodiscard]] PURE_STATICS Math::Rectanglef GetChildContentArea() const
		{
			return m_childContentArea;
		}

		[[nodiscard]] PURE_STATICS uint8 GetChildIndexAtCoordinate(const Math::Vector2f coordinate) const
		{
			const Math::Vector2f centerCoordinate = GetCenterCoordinate();
			return ((coordinate.x > centerCoordinate.x) * 2) | (coordinate.y > centerCoordinate.y);
		}
		[[nodiscard]] PURE_STATICS static Math::TVector2<uint8> GetChildRelativeCoordinate(const uint8 childIndex)
		{
			const uint8 x = childIndex / 2u;
			return {x, (uint8)(childIndex - x * 2)};
		}
		[[nodiscard]] PURE_STATICS static Math::Rectanglef GetChildContentArea(const uint8 childIndex, const Math::Rectanglef contentArea)
		{
			const Math::Vector2f childSize = contentArea.GetSize() * 0.5f;
			const Math::Vector2f newPosition = contentArea.GetPosition() + childSize * (Math::Vector2f)GetChildRelativeCoordinate(childIndex);
			return Math::Rectanglef{newPosition, childSize};
		}
		[[nodiscard]] PURE_STATICS Math::Rectanglef GetChildContentArea(const uint8 childIndex) const
		{
			const Math::Rectanglef contentArea = GetContentArea();
			return GetChildContentArea(childIndex, contentArea);
		}

		[[nodiscard]] bool ReserveChild(const uint8 index, QuadTreeNode*& pExpected)
		{
			return m_children[index].CompareExchangeStrong(pExpected, reinterpret_cast<QuadTreeNode*>(static_cast<uintptr>(0xDEADBEEF)));
		}

		template<typename... Args>
		void AddChild(const uint8 index, QuadTreeNode& node)
		{
			Assert(!IsFlaggedForDeletion());
			QuadTreeNode* pExpectedChild = reinterpret_cast<QuadTreeNode*>(static_cast<uintptr>(0xDEADBEEF));
			[[maybe_unused]] const bool wasExchanged = m_children[index].CompareExchangeStrong(pExpectedChild, &node);
			Assert(wasExchanged);
			m_packedInfo.FetchAddChildCount();
		}

		enum class RemovalResult
		{
			Done,
			RemovedLastElement
		};

		[[nodiscard]] inline RemovalResult RemoveChild(const uint8 index, QuadTreeNode& expectedChild)
		{
			Assert(!WasDeleted());
			Assert(!IsFlaggedForDeletion());

			QuadTreeNode* pExpectedChild = &expectedChild;
			const bool wasExchanged = m_children[index].CompareExchangeStrong(pExpectedChild, nullptr);
			Expect(wasExchanged);
			const uint8 remainingChildCount = m_packedInfo.FetchSubChildCount();
			return remainingChildCount == 0 ? RemovalResult::RemovedLastElement : RemovalResult::Done;
		}

		void MergeContentArea(const Math::Rectanglef contentArea)
		{
			for (QuadTreeNode* pNode = this; pNode != nullptr; pNode = pNode->GetParent())
			{
				pNode->m_childContentArea = pNode->m_childContentArea.Merge(contentArea);
			}
		}

		void EmplaceElement(ElementType&& element, const float depth, const Math::Rectanglef contentArea)
		{
			Assert(!WasDeleted());
			Assert(!IsFlaggedForDeletion());

			Threading::UniqueLock lock(m_elementMutex);
			StoredElement* __restrict pIt = std::lower_bound(
				m_elements.begin().Get(),
				m_elements.end().Get(),
				depth,
				[](const StoredElement& existingElement, const float newDepth) -> bool
				{
					return newDepth > existingElement.m_depth;
				}
			);
			m_elements.Emplace(pIt, Memory::Uninitialized, StoredElement{Forward<ElementType>(element), depth});
			MergeContentArea(contentArea);
		}

		template<typename ComparableType>
		[[nodiscard]] RemovalResult RemoveElement(ComparableType& element)
		{
			Assert(!WasDeleted());
			Assert(!IsFlaggedForDeletion());
			Threading::UniqueLock lock(m_elementMutex);
			[[maybe_unused]] const bool wasRemoved = m_elements.RemoveFirstOccurrencePredicate(
				[&element](const StoredElement& existingElement)
				{
					return existingElement.m_element == element ? ErasePredicateResult::Remove : ErasePredicateResult::Continue;
				}
			);
			Assert(wasRemoved);

			return m_elements.HasElements() ? RemovalResult::RemovedLastElement : RemovalResult::Done;
		}

		void OnElementContentAreaChanged(const Math::Rectanglef newContentArea)
		{
			MergeContentArea(newContentArea);
		}

		[[nodiscard]] PURE_STATICS bool Contains(const Math::Vector2f coordinate) const
		{
			return GetContentArea().Contains(coordinate);
		}
		[[nodiscard]] PURE_STATICS bool IsEmpty() const
		{
			return m_elements.IsEmpty() & (m_packedInfo.GetChildCount() == 0);
		}

		struct StoredElement
		{
			ElementType m_element;
			float m_depth;
		};

		struct ElementView : public ArrayView<StoredElement>
		{
			using BaseType = ArrayView<StoredElement>;
			ElementView(Threading::SharedMutex& mutex, const BaseType view)
				: BaseType(view)
				, m_lock(mutex)
			{
				if (UNLIKELY_ERROR(!m_lock.IsLocked()))
				{
					BaseType::operator=({});
				}
			}
		private:
			Threading::SharedLock<Threading::SharedMutex> m_lock;
		};
		[[nodiscard]] ElementView GetElementView()
		{
			return ElementView{const_cast<Threading::SharedMutex&>(m_elementMutex), ArrayView<StoredElement>{m_elements.GetView()}};
		}
		struct ConstElementView : public ArrayView<const StoredElement>
		{
			using BaseType = ArrayView<const StoredElement>;
			ConstElementView(Threading::SharedMutex& mutex, const BaseType view)
				: BaseType(view)
				, m_lock(mutex)
			{
				if (UNLIKELY_ERROR(!m_lock.IsLocked()))
				{
					BaseType::operator=({});
				}
			}
		private:
			Threading::SharedLock<Threading::SharedMutex> m_lock;
		};
		[[nodiscard]] ConstElementView GetElementView() const
		{
			return ConstElementView{const_cast<Threading::SharedMutex&>(m_elementMutex), ArrayView<const StoredElement>{m_elements.GetView()}};
		}

		template<typename Callback>
		void IterateChildren(Callback&& callback)
		{
			Assert(!WasDeleted());
			Assert(!IsFlaggedForDeletion());

			for (QuadTreeNode* pNode : m_children)
			{
				if ((pNode == nullptr) | (pNode == reinterpret_cast<QuadTreeNode*>(static_cast<uintptr>(0xDEADBEEF))))
				{
					continue;
				}

				callback(*pNode);
			}
		}

		template<typename Callback>
		void IterateChildren(Callback&& callback) const
		{
			Assert(!WasDeleted());
			Assert(!IsFlaggedForDeletion());

			for (QuadTreeNode* pNode : m_children)
			{
				if ((pNode == nullptr) | (pNode == reinterpret_cast<QuadTreeNode*>(static_cast<uintptr>(0xDEADBEEF))))
				{
					continue;
				}

				callback(*pNode);
			}
		}

		[[nodiscard]] PURE_STATICS QuadTreeNode* GetParent() const
		{
			return m_pParentNode;
		}

		[[nodiscard]] PURE_STATICS uint8 GetIndex() const
		{
			return m_packedInfo.GetNodeIndex();
		}
	protected:
		struct PackedInfo
		{
			PackedInfo(const uint8 nodeIndex)
				: m_value(uint8(nodeIndex << (uint8)4))
			{
			}

			[[nodiscard]] uint8 GetChildCount() const
			{
				return Threading::Atomics::Load(m_value) & 0b00000111;
			}

			[[nodiscard]] uint8 GetNodeIndex() const
			{
				return uint8(m_value >> 4);
			}

			uint8 FetchAddChildCount()
			{
				const uint8 previousValue = Threading::Atomics::FetchAdd(m_value, (uint8)1) & 0b00000111;
				Assert(previousValue <= 4);
				return previousValue;
			}

			uint8 FetchSubChildCount()
			{
				const uint8 previousValue = Threading::Atomics::FetchSubtract(m_value, (uint8)1) & 0b00000111;
				Assert(previousValue > 0);
				return previousValue;
			}
		protected:
			uint8 m_value;
		};
	protected:
		friend QuadTree<ElementType>;

		Math::Rectanglef m_childContentArea;
		Math::TVector3<float> m_centerCoordinateAndRadiusSquared;
		QuadTreeNode* const m_pParentNode = nullptr;
		mutable Threading::SharedMutex m_elementMutex;
		Vector<StoredElement> m_elements;
		ChildContainerType m_children;

		PackedInfo m_packedInfo;

		enum class Flags : uint8
		{
			FlaggedForDeletion = 1 << 0,
			Deleted = 1 << 1
		};
		AtomicEnumFlags<Flags> m_flags;
	};

	template<typename NodeType>
	struct QuadTree : public NodeType
	{
		using BaseType = NodeType;
		using ElementType = typename NodeType::ElementType;
		QuadTree(const Math::Radiusf radius)
			: NodeType(nullptr, 255, Math::Vector2f{0.f}, radius)
		{
		}

		[[nodiscard]] static NodeType&
		GetOrMakeIdealChildNode(ReferenceWrapper<NodeType> node, const float itemRadiusSquared, const Math::Rectanglef itemBounds)
		{
			const Math::Radiusf itemRadius = Math::Radiusf::FromMeters(Math::Sqrt(itemRadiusSquared));
			Math::Radiusf halfNodeRadius = Math::Radiusf::FromMeters(Math::Sqrt(node->GetRadiusSquared()) * 0.5f);
			Math::Rectanglef maskedContentArea = node->GetContentArea().Mask(itemBounds);
			// Assert(node->GetContentArea().Overlaps(itemBounds));
			// Assert(node->GetContentArea().Mask(itemBounds).HasSize());

			const Math::Vector2f itemLocation = itemBounds.GetCenterPosition();
			while (itemRadius < halfNodeRadius)
			{
				const uint8 childIndex = node->GetChildIndexAtCoordinate(itemLocation);

				using NodeBaseType = typename NodeType::BaseType;
				NodeBaseType* pChildNode = nullptr;
				if (node->ReserveChild(childIndex, pChildNode))
				{
					const Math::Rectanglef childNodeBounds = node->GetChildContentArea(childIndex);
					pChildNode = new NodeType(&*node, childIndex, childNodeBounds.GetCenterPosition(), halfNodeRadius);

					node->AddChild(childIndex, *pChildNode);
				}
				else if (pChildNode == reinterpret_cast<NodeType*>(static_cast<uintptr>(0xDEADBEEF)))
				{
					// Child was reserved and is being created, try again
					continue;
				}

				Assert(pChildNode != nullptr);
				node = static_cast<NodeType&>(*pChildNode);

				halfNodeRadius = Math::Radiusf::FromMeters(Math::Sqrt(node->GetRadiusSquared()) * 0.5f);
				maskedContentArea = node->GetContentArea().Mask(itemBounds);

				/*Assert(node->GetContentArea().Contains(itemLocation));
				Assert(node->GetContentArea().Overlaps(itemBounds));
				Assert(node->GetContentArea().Mask(itemBounds).HasSize());*/
			}

			return node;
		}
	};
}
