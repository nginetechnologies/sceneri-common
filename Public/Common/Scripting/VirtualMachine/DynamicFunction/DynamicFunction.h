#pragma once

#include "DynamicInvoke.h"
#include "Registers.h"

#include <Common/Function/FunctionPointer.h>
#include <Common/Reflection/Type.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Scripting::VM
{
	struct TRIVIAL_ABI DynamicFunction
	{
		using FunctionPointer =
			ngine::FunctionPointer<ReturnValue(Register R0, Register R1, Register R2, Register R3, Register R4, Register R5)>;
		using RawFunctionPointer = FunctionPointer::FunctionType;

		FORCE_INLINE DynamicFunction() = default;
		FORCE_INLINE constexpr DynamicFunction(const RawFunctionPointer functionPointer)
			: m_functionPointer(functionPointer)
		{
		}
		template<auto Function>
		FORCE_INLINE static constexpr DynamicFunction Make()
		{
			return DynamicInvoke::Invoke<Function>;
		}
		//! Creates a dynamic function that retrieves a global or member variable
		template<auto Variable>
		FORCE_INLINE static constexpr DynamicFunction MakeGetVariable()
		{
			return DynamicInvoke::InvokeGetMemberVariable<Variable>;
		}

		constexpr FORCE_INLINE ReturnValue
		operator()(Register R0, Register R1, Register R2, Register R3, Register R4, Register R5) const noexcept
		{
			return m_functionPointer(R0, R1, R2, R3, R4, R5);
		}

		template<typename... Args>
		FORCE_INLINE void operator()(Args&&... args) const
		{
			Registers registers;
			registers.PushArguments(Forward<Args>(args)...);
			m_functionPointer(registers[0], registers[1], registers[2], registers[3], registers[4], registers[5]);
			registers.PopArguments<Args...>();
		}

		template<typename ReturnType, typename... Args>
		[[nodiscard]] FORCE_INLINE ReturnType Invoke(Args&&... args) const
		{
			Registers registers;
			registers.PushArguments<0, Args...>(Forward<Args>(args)...);
			const ReturnValue vectorizedReturnValue =
				m_functionPointer(registers[0], registers[1], registers[2], registers[3], registers[4], registers[5]);
			registers[0] = vectorizedReturnValue.x;
			registers[1] = vectorizedReturnValue.y;
			registers[2] = vectorizedReturnValue.z;
			registers[3] = vectorizedReturnValue.w;
			registers.PopArguments<Args...>();
			if constexpr (!TypeTraits::IsSame<ReturnType, void>)
			{
				ReturnType returnValue = Move(registers.ExtractReturnType<ReturnType>());
				registers.PopReturnType<ReturnType>();
				return Move(returnValue);
			}
		}

		[[nodiscard]] FORCE_INLINE bool IsValid() const
		{
			return m_functionPointer.IsValid();
		}
		[[nodiscard]] FORCE_INLINE bool operator==(const DynamicFunction& other) const
		{
			return m_functionPointer == other.m_functionPointer;
		}
		[[nodiscard]] FORCE_INLINE bool operator!=(const DynamicFunction& other) const
		{
			return m_functionPointer != other.m_functionPointer;
		}
	protected:
		FunctionPointer m_functionPointer;
	};
}

namespace ngine::Reflection
{
	template<>
	struct ReflectedType<Scripting::VM::DynamicFunction>
	{
		static constexpr auto Type =
			Reflection::Reflect<Scripting::VM::DynamicFunction>("c74641b5-08c2-4f44-90d3-aacf6b6f7b1b"_guid, MAKE_UNICODE_LITERAL("Function"));
	};
}
