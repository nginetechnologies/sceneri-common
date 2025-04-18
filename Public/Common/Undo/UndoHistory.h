#pragma once

#include <Common/Memory/Containers/Vector.h>
#include <Common/Function/Event.h>

namespace ngine::Undo
{
	template<typename EntryType>
	struct History
	{
		[[nodiscard]] inline bool CanUndo() const
		{
			return m_entries.IsWithinBounds(m_pNextEntry - 1);
		}

		[[nodiscard]] inline bool CanRedo() const
		{
			return m_entries.IsWithinBounds(m_pNextEntry);
		}

		[[nodiscard]] inline uint32 GetEntryCount() const
		{
			return m_entries.GetSize();
		}

		Event<void(void*), 24> OnEnableUndo;
		Event<void(void*), 24> OnDisableUndo;
		Event<void(void*), 24> OnEnableRedo;
		Event<void(void*), 24> OnDisableRedo;

		void AddEntry(EntryType&& entry)
		{
			const bool couldUndo = CanUndo();
			const bool couldRedo = CanRedo();

			EntryType* pEntry = &m_entries.Emplace(m_pNextEntry, Memory::Uninitialized, Forward<EntryType>(entry));
			m_pNextEntry = pEntry + 1;

			m_entries.Shrink(m_entries.GetIteratorIndex(m_pNextEntry));

			if (!couldUndo)
			{
				Assert(CanUndo());
				OnEnableUndo();
			}

			if (couldRedo)
			{
				Assert(!CanRedo());
				OnDisableRedo();
			}
		}

		EntryType& Undo()
		{
			const bool couldRedo = CanRedo();

			Assert(CanUndo());
			EntryType& undoType = *(m_pNextEntry - 1);
			m_pNextEntry--;

			if (!CanUndo())
			{
				OnDisableUndo();
			}

			if (!couldRedo)
			{
				Assert(CanRedo());
				OnEnableRedo();
			}

			return undoType;
		}

		[[nodiscard]] Optional<EntryType*> GetCurrentEntry() const
		{
			return {(m_pNextEntry - 1), m_pNextEntry != m_entries.begin()};
		}

		EntryType& Redo()
		{
			const bool couldUndo = CanUndo();

			Assert(CanRedo());
			EntryType& redoType = *m_pNextEntry;
			m_pNextEntry++;

			if (!CanRedo())
			{
				OnDisableRedo();
			}

			if (!couldUndo)
			{
				OnEnableUndo();
			}

			return redoType;
		}
	protected:
		Vector<EntryType> m_entries;
		EntryType* m_pNextEntry = m_entries.end();
	};
}
