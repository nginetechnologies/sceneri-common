#include "Assert/Assert.h"

namespace ngine::Internal
{
#if ENABLE_ASSERTS
	AssertEvents::~AssertEvents()
	{
		delete[] m_pListeners;
	}

	void NO_INLINE UNLIKELY_ERROR_SECTION COLD_FUNCTION NO_DEBUG
	AssertEvents::OnAssert(const char* file, const uint32 lineNumber, const bool isFirstTime)
	{
		for (uint32 i = 0; i < m_count; ++i)
		{
			const Listener& listener = m_pListeners[i];
			listener.function(file, lineNumber, isFirstTime, "Failed assertion", listener.pUserData);
		}
	}

	void NO_INLINE UNLIKELY_ERROR_SECTION COLD_FUNCTION NO_DEBUG
	AssertEvents::OnAssert(const char* file, const uint32 lineNumber, const bool isFirstTime, const char* message)
	{
		for (uint32 i = 0; i < m_count; ++i)
		{
			const Listener& listener = m_pListeners[i];
			listener.function(file, lineNumber, isFirstTime, message, listener.pUserData);
		}
	}

	NO_DEBUG /* static */ AssertEvents& AssertEvents::GetInstance()
	{
		static AssertEvents events;
		return events;
	}

	void AssertEvents::AddAssertListener(EventFunction&& function, void* pUserData)
	{
		if (m_count == m_capacity)
		{
			m_capacity++;
			Listener* pNewListeners = new Listener[m_capacity];
			if (m_pListeners != nullptr)
			{
				// Move existing functions to the new array
				for (uint32 i = 0; i < m_count; ++i)
				{
					pNewListeners[i] = m_pListeners[i];
				}

				delete[] m_pListeners;
			}

			m_pListeners = pNewListeners;
		}

		m_pListeners[m_count++] = Listener{Forward<EventFunction>(function), pUserData};
	}

	void AssertEvents::RemoveAssertListener(void* pUserData)
	{
		uint32 index = 0;
		for (; index < m_count; ++index)
		{
			if (m_pListeners[index].pUserData == pUserData)
			{
				break;
			}
		}

		if (index < m_count)
		{
			// Shift elements after the removed function
			for (uint32 i = index; i < m_count - 1; ++i)
			{
				m_pListeners[i] = m_pListeners[i + 1];
			}

			--m_count;
		}
	}
#endif
}
