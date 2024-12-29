#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Memory/Allocators/FixedAllocator.h>
#include <Common/Memory/Containers/FlatVector.h>
#include <Common/Memory/ReferenceWrapper.h>
#include <Common/Memory/Align.h>
#include <Common/Platform/Pure.h>

namespace ngine::Memory
{
	template<size ValueIn>
	struct MostSignificantBit
	{
		template<typename SizeType>
		static constexpr uint8 Get(SizeType value) noexcept
		{
			uint8 index = 0u;
			while (value >>= 1)
			{
				index++;
			}

			return index;
		}

		inline static constexpr size Value = Get(ValueIn);
	};

	template<size Size, size MaximumFragmentCount, size Alignment = 8>
	struct FixedPool
	{
		static_assert((Size % Alignment) == 0, "Pool size must be aligned!");
		using SizeType = Memory::NumericSize<Size>;
	protected:
		struct Fragment;

		struct alignas(Alignment) FragmentTail
		{
			SizeType m_allocatedSize;

			[[nodiscard]] Fragment* GetFragment() noexcept
			{
				return reinterpret_cast<Fragment*>(reinterpret_cast<uintptr>(this) - m_allocatedSize + sizeof(FragmentTail));
			}
		};

		struct alignas(Alignment) Fragment
		{
			inline static constexpr SizeType DefaultAllocatedSize = MostSignificantBit<Size>::Value + 1u;
			SizeType m_allocatedSize : DefaultAllocatedSize;
			SizeType m_isUsed : 1;

			[[nodiscard]] PURE_STATICS unsigned char* GetData() noexcept
			{
				return reinterpret_cast<unsigned char*>(reinterpret_cast<uintptr>(this) + sizeof(Fragment));
			}

			[[nodiscard]] PURE_STATICS SizeType GetDataSize() const noexcept
			{
				return m_allocatedSize - sizeof(Fragment) - sizeof(FragmentTail);
			}

			[[nodiscard]] PURE_STATICS unsigned char* GetDataEnd() noexcept
			{
				return GetData() + GetDataSize();
			}

			[[nodiscard]] PURE_STATICS unsigned char* GetEnd() noexcept
			{
				return reinterpret_cast<unsigned char*>(this) + m_allocatedSize;
			}

			[[nodiscard]] PURE_STATICS Fragment* GetNext() noexcept
			{
				return reinterpret_cast<Fragment*>(GetEnd());
			}

			[[nodiscard]] PURE_STATICS FragmentTail* GetTail() noexcept
			{
				return reinterpret_cast<FragmentTail*>(GetEnd() - sizeof(FragmentTail));
			}

			[[nodiscard]] PURE_STATICS FragmentTail* GetPreviousTail() noexcept
			{
				return reinterpret_cast<FragmentTail*>(reinterpret_cast<uintptr>(this) - sizeof(FragmentTail));
			}

			[[nodiscard]] PURE_STATICS Fragment* GetPrevious() noexcept
			{
				return GetPreviousTail()->GetFragment();
			}
		};
	public:
		using FragmentSizeType = Memory::NumericSize<MaximumFragmentCount>;

		FixedPool() noexcept
		{
			Fragment& fullFragment = *reinterpret_cast<Fragment*>(m_allocator.GetData());
			fullFragment.m_allocatedSize = m_allocator.GetCapacity();
			fullFragment.GetTail()->m_allocatedSize = fullFragment.m_allocatedSize;
			fullFragment.m_isUsed = 0;
			m_unusedFragments.EmplaceBack(fullFragment);

			VerifyIntegrity();
		}

		[[nodiscard]] RESTRICTED_RETURN void* Allocate(const size requestedAllocationSize) noexcept
		{
			const size allocationSize = (((requestedAllocationSize + Alignment - 1) / Alignment) * Alignment) + sizeof(Fragment) +
			                            sizeof(FragmentTail);

			for (typename decltype(m_unusedFragments)::iterator it = m_unusedFragments.begin(), end = m_unusedFragments.end(); it != end; ++it)
			{
				Fragment& unusedFragment = *it;

				if (unusedFragment.GetDataSize() > allocationSize)
				{
					unusedFragment.m_allocatedSize -= static_cast<SizeType>(allocationSize);
					unusedFragment.GetTail()->m_allocatedSize = unusedFragment.m_allocatedSize;

					Fragment& newFragment = *unusedFragment.GetNext();
					newFragment.m_isUsed = 1;
					newFragment.m_allocatedSize = (SizeType)allocationSize;
					newFragment.GetTail()->m_allocatedSize = (SizeType)allocationSize;

					Assert((reinterpret_cast<uintptr>(newFragment.GetData()) % Alignment) == 0);
					return newFragment.GetData();
				}
				else if (unusedFragment.m_allocatedSize == allocationSize)
				{
					m_unusedFragments.Remove(it);
					unusedFragment.m_isUsed = 1;

					Assert((reinterpret_cast<uintptr>(unusedFragment.GetData()) % Alignment) == 0);
					return unusedFragment.GetData();
				}
			}
			return nullptr;
		}

		void Deallocate(void* pData) noexcept
		{
			Fragment& fragment = *reinterpret_cast<Fragment*>(reinterpret_cast<uintptr>(pData) - sizeof(Fragment));
			Assert(m_allocator.GetView().IsWithinBounds(reinterpret_cast<unsigned char*>(&fragment)));

			Assert(fragment.m_isUsed);
			fragment.m_isUsed = false;
			Assert(fragment.GetTail()->GetFragment() == &fragment);

			Fragment* pNextFragment = fragment.GetNext();
			if (m_allocator.GetView().IsWithinBounds(reinterpret_cast<unsigned char*>(pNextFragment)) && !pNextFragment->m_isUsed)
			{
				fragment.m_allocatedSize += pNextFragment->m_allocatedSize;
				fragment.GetTail()->m_allocatedSize = fragment.m_allocatedSize;

				const OptionalIterator<ReferenceWrapper<Fragment>> nextFragmentIterator = m_unusedFragments.Find(*pNextFragment);
				*nextFragmentIterator = fragment;

				Fragment* pPreviousFragment = fragment.GetPrevious();
				if((reinterpret_cast<unsigned char*>(&fragment) != m_allocator.GetData()) &
				       m_allocator.GetView().IsWithinBounds(reinterpret_cast<unsigned char*>(pPreviousFragment)) &&
				   !pPreviousFragment->m_isUsed)
				{
					[[maybe_unused]] const bool wasRemoved = m_unusedFragments.RemoveFirstOccurrence(fragment);
					Assert(wasRemoved);

					pPreviousFragment->m_allocatedSize += fragment.m_allocatedSize;
					pPreviousFragment->GetTail()->m_allocatedSize = pPreviousFragment->m_allocatedSize;
				}
			}
			else
			{
				Fragment* pPreviousFragment = fragment.GetPrevious();
				if((reinterpret_cast<unsigned char*>(&fragment) != m_allocator.GetData()) &
				       m_allocator.GetView().IsWithinBounds(reinterpret_cast<unsigned char*>(pPreviousFragment)) &&
				   !pPreviousFragment->m_isUsed)
				{
					pPreviousFragment->m_allocatedSize += fragment.m_allocatedSize;
					pPreviousFragment->GetTail()->m_allocatedSize = pPreviousFragment->m_allocatedSize;
				}
				else
				{
					m_unusedFragments.EmplaceBack(fragment);
				}
			}
		}

		PURE_STATICS void VerifyIntegrity() noexcept
		{
			SizeType totalSize = 0u;
			FragmentSizeType fragmentCount = 0u;
			FragmentSizeType unusedFragmentCount = 0u;
			FragmentSizeType usedFragmentCount = 0u;

			for (Fragment* pFragment = reinterpret_cast<Fragment*>(m_allocator.GetData());
			     m_allocator.GetView().IsWithinBounds(reinterpret_cast<unsigned char*>(pFragment));)
			{
				Assert(pFragment->m_allocatedSize == pFragment->GetTail()->m_allocatedSize);
				totalSize += pFragment->m_allocatedSize;

				Fragment* pNext = pFragment->GetNext();
				if (m_allocator.GetView().IsWithinBounds(reinterpret_cast<unsigned char*>(pNext)))
				{
					Assert(pNext->GetTail()->GetFragment() == pNext);
					Assert(pNext->GetPrevious() == pFragment);
				}
				Assert(pFragment->GetTail()->GetFragment() == pFragment);

				if((reinterpret_cast<unsigned char*>(pFragment) != m_allocator.GetData()) &&
				   m_allocator.GetView().IsWithinBounds(reinterpret_cast<unsigned char*>(pFragment->GetPrevious())))
				{
					Assert(pFragment->GetPrevious() != pFragment);
				}

				[[maybe_unused]] const bool isInUnusedFragments = m_unusedFragments.Contains(*pFragment);
				Assert(isInUnusedFragments != (bool)pFragment->m_isUsed);

				fragmentCount++;
				unusedFragmentCount += !pFragment->m_isUsed;
				usedFragmentCount += (FragmentSizeType)pFragment->m_isUsed;
				pFragment = pNext;
			}

#if ENABLE_ASSERTS
			for (Fragment& unusedFragment : m_unusedFragments)
			{
				bool found = false;

				for (Fragment* pFragment = reinterpret_cast<Fragment*>(m_allocator.GetData());
				     m_allocator.GetView().IsWithinBounds(reinterpret_cast<unsigned char*>(pFragment));
				     pFragment = pFragment->GetNext())
				{
					if (&unusedFragment == pFragment)
					{
						Assert(!pFragment->m_isUsed);

						found = true;
						break;
					}
				}

				Assert(found);
			}
#endif

			Assert(totalSize == m_allocator.GetCapacity());
			Assert(unusedFragmentCount == m_unusedFragments.GetSize());
		}

		[[nodiscard]] PURE_STATICS FragmentSizeType GetUnusedFragmentCount() const noexcept
		{
			return m_unusedFragments.GetSize();
		}
	protected:
		Memory::FixedAllocator<unsigned char, Size, SizeType, SizeType, Alignment> m_allocator;
		FlatVector<ReferenceWrapper<Fragment>, MaximumFragmentCount> m_unusedFragments;
	};
}
