#pragma once

#include <Common/Function/ForwardDeclarations/FunctionPointer.h>
#include <Common/TypeTraits/WithoutReference.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/StaticUnreachable.h>
#include <Common/Memory/Forward.h>
#include <Common/Assert/Assert.h>

namespace ngine
{
	template<typename ReturnType, typename... ArgumentTypes>
	struct FunctionPointer<ReturnType(ArgumentTypes...)>
	{
		using FunctionType = ReturnType (*)(ArgumentTypes...);

		constexpr FunctionPointer() = default;
		template<typename Callback>
		constexpr inline FunctionPointer(Callback&& function) noexcept
			: m_function(Forward<Callback>(function))
		{
			using FunctionValueType = TypeTraits::WithoutReference<Callback>;
			constexpr size functionSize = sizeof(FunctionValueType);

			static_assert(functionSize <= sizeof(m_function), "Attempt to assign function which surpassed the allocated storage");
		}
		constexpr inline FunctionPointer(const FunctionType function) noexcept
			: m_function(function)
		{
		}

		template<typename ObjectType>
		constexpr FunctionPointer(ObjectType&, ReturnType (ObjectType::*)(ArgumentTypes...))
		{
			static_unreachable("Not enough space to assign member function pointer!");
		}
		constexpr FunctionPointer(const FunctionPointer& other) noexcept
			: m_function(other.m_function)
		{
		}
		constexpr FunctionPointer(FunctionPointer& other) noexcept
			: m_function(other.m_function)
		{
		}
		constexpr FunctionPointer(FunctionPointer&& other) noexcept
			: m_function(other.m_function)
		{
			other.m_function = nullptr;
		}
		constexpr FunctionPointer& operator=(const FunctionPointer& other) noexcept
		{
			m_function = other.m_function;
			return *this;
		}
		constexpr FunctionPointer& operator=(FunctionPointer& other) noexcept
		{
			m_function = other.m_function;
			return *this;
		}
		constexpr FunctionPointer& operator=(FunctionPointer&& other) noexcept
		{
			m_function = other.m_function;
			other.m_function = nullptr;
			return *this;
		}

		template<typename ObjectType>
		inline void Bind(ObjectType&, ReturnType (ObjectType::*)(ArgumentTypes...))
		{
			static_unreachable("Not enough space to assign member function pointer!");
		}

		constexpr inline void Unbind() noexcept
		{
			m_function = nullptr;
		}

		constexpr ReturnType operator()(ArgumentTypes... argumentTypes) const noexcept
		{
			Expect(m_function != nullptr);
			return m_function(Forward<ArgumentTypes>(argumentTypes)...);
		}

		[[nodiscard]] bool IsValid() const noexcept
		{
			return m_function != nullptr;
		}

		[[nodiscard]] bool operator==(const FunctionPointer other) const
		{
			return m_function == other.m_function;
		}
		[[nodiscard]] bool operator!=(const FunctionPointer other) const
		{
			return m_function != other.m_function;
		}
		[[nodiscard]] bool operator==(const FunctionType other) const
		{
			return m_function == other;
		}
		[[nodiscard]] bool operator!=(const FunctionType other) const
		{
			return m_function != other;
		}
	protected:
		FunctionType m_function = nullptr;
	};
}
