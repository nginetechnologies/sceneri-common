#pragma once

#include <Common/Memory/Containers/ByteView.h>
#include <Common/Memory/Containers/Array.h>
#include <Common/Memory/Tuple.h>
#include <Common/TypeTraits/IsTriviallyDestructible.h>
#include <Common/Platform/TrivialABI.h>

#include "DynamicInvoke.h"

namespace ngine::Scripting::VM
{
	struct TRIVIAL_ABI Registers
	{
		Registers() = default;
		constexpr Registers(Register R0, Register R1, Register R2, Register R3, Register R4, Register R5)
			: m_registers{R0, R1, R2, R3, R4, R5}
		{
		}

		template<typename TupleType, typename... ArgumentTypes>
		struct GetArgumentsStackType;

		template<typename TupleType>
		struct GetArgumentsStackType<TupleType>
		{
			using Type = TupleType;
		};

		template<typename TupleType, typename ArgumentType>
		struct GetArgumentsStackType<TupleType, ArgumentType>
		{
			using Type =
				TypeTraits::Select<(sizeof(ArgumentType) > sizeof(Register)), typename TupleType::template Amend<ArgumentType>, TupleType>;
		};

		template<typename TupleType, typename ArgumentType, typename... RemainingArgumentTypes>
		struct GetArgumentsStackType<TupleType, ArgumentType, RemainingArgumentTypes...>
		{
			using ElementTupleType =
				TypeTraits::Select<(sizeof(ArgumentType) > sizeof(Register)), typename TupleType::template Amend<ArgumentType>, TupleType>;
			using Type = typename GetArgumentsStackType<ElementTupleType, RemainingArgumentTypes...>::Type;
		};

		template<typename ReturnType, typename... ArgumentTypes>
		struct GetFunctionStackType
		{
			using ArgumentsTupleType = typename GetArgumentsStackType<Tuple<>, ArgumentTypes...>::Type;
			using Type = TypeTraits::
				Select<(sizeof(ReturnType) > sizeof(Register)), typename ArgumentsTupleType::template Amend<ReturnType>, ArgumentsTupleType>;
		};

		template<typename ReturnType, typename... ArgumentTypes>
		using StackType = typename GetFunctionStackType<ReturnType, ArgumentTypes...>::Type;

		template<size Index, typename ArgumentType>
		[[nodiscard]] FORCE_INLINE ArgumentType ExtractArgument() const
		{
			return DynamicInvoke::ExtractArgument<ArgumentType>(m_registers[Index]);
		}

		template<typename ArgumentType>
		[[nodiscard]] FORCE_INLINE ArgumentType ExtractArgument(const uint8 registerIndex) const
		{
			return DynamicInvoke::ExtractArgument<ArgumentType>(m_registers[registerIndex]);
		}

		template<size Index>
		FORCE_INLINE void ExtractArgumentIntoView(const ByteView targetData) const
		{
			static_assert(Index < RegisterCount);
			if (targetData.GetDataSize() <= sizeof(Register))
			{
				targetData.CopyFrom(ConstByteView::Make(m_registers[Index]));
			}
			else
			{
				ByteType* pData = ExtractArgument<Index, ByteType*>();
				targetData.CopyFrom(ConstByteView{pData, targetData.GetDataSize()});
			}
		}

		FORCE_INLINE void ExtractArgumentIntoView(const uint8 registerIndex, const ByteView targetData) const
		{
			Assert(registerIndex < RegisterCount);
			if (targetData.GetDataSize() <= sizeof(Register))
			{
				targetData.CopyFrom(ConstByteView::Make(m_registers[registerIndex]));
			}
			else
			{
				ByteType* pData = ExtractArgument<ByteType*>(registerIndex);
				targetData.CopyFrom(ConstByteView{pData, targetData.GetDataSize()});
			}
		}

		template<size Index, typename ArgumentType>
		FORCE_INLINE void PushArgument(ArgumentType argument)
		{
			static_assert(Index < RegisterCount);
			m_registers[Index] = DynamicInvoke::LoadArgument<ArgumentType>(Forward<ArgumentType>(argument));
		}
		template<size Index>
		FORCE_INLINE void PushDynamicArgument(const ConstByteView data)
		{
			static_assert(Index < RegisterCount);
			m_registers[Index] = DynamicInvoke::LoadDynamicArgument(data);
		}

		template<typename ArgumentType>
		FORCE_INLINE void PushArgument(const uint8 index, ArgumentType argument)
		{
			Assert(index < RegisterCount);
			m_registers[index] = DynamicInvoke::LoadArgument<ArgumentType>(Forward<ArgumentType>(argument));
		}

		FORCE_INLINE void PushDynamicArgument(const uint8 index, const ConstByteView data)
		{
			Assert(index < RegisterCount);
			m_registers[index] = DynamicInvoke::LoadDynamicArgument(data);
		}

		template<size Index, typename ArgumentType, typename... ArgumentTypes>
		FORCE_INLINE void PushArguments(ArgumentType argument, ArgumentTypes... arguments)
		{
			PushArgument<Index, ArgumentType>(Forward<ArgumentType>(argument));
			PushArguments<Index + 1, ArgumentTypes...>(Forward<ArgumentTypes>(arguments)...);
		}

		template<size Index>
		FORCE_INLINE void PushArguments()
		{
		}

		template<typename... Arguments>
		FORCE_INLINE void PushArguments(Arguments... arguments)
		{
			PushArguments<0, Arguments...>(Forward<Arguments>(arguments)...);
		}

		template<size Index, typename ArgumentType>
		[[nodiscard]] FORCE_INLINE void PopArgument()
		{
			static constexpr bool IsReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<ArgumentType>, ArgumentType>;
			using FilteredArgumentType =
				TypeTraits::Select<IsReference, ReferenceWrapper<TypeTraits::WithoutReference<ArgumentType>>, ArgumentType>;

			static_assert(Index < RegisterCount);
			if constexpr (sizeof(FilteredArgumentType) <= sizeof(Register))
			{
				if constexpr (!TypeTraits::IsTriviallyDestructible<ArgumentType>)
				{
					reinterpret_cast<ArgumentType&>(m_registers[Index]).~ArgumentType();
				}
			}
			else
			{
				delete ExtractArgument<Index, ArgumentType*>();
			}
		}

		template<size Index, typename ArgumentType, typename... ArgumentTypes>
		FORCE_INLINE void PopArguments()
		{
			PopArgument<Index, ArgumentType>();
			PopArguments<Index + 1, ArgumentTypes...>();
		}

		template<size Index>
		FORCE_INLINE void PopArguments()
		{
		}

		template<typename... Arguments>
		FORCE_INLINE void PopArguments()
		{
			PopArguments<0, Arguments...>();
		}

		template<typename ReturnType>
		[[nodiscard]] FORCE_INLINE ReturnType ExtractReturnType()
		{
			return DynamicInvoke::ExtractReturnType<ReturnType>(ReturnValue{m_registers[0], m_registers[1], m_registers[2], m_registers[3]});
		}

		template<typename ReturnType>
		[[nodiscard]] FORCE_INLINE void PopReturnType()
		{
			using FilteredReturnType = Tuple<
				TypeTraits::Select<TypeTraits::IsReference<ReturnType>, ReferenceWrapper<TypeTraits::WithoutReference<ReturnType>>, ReturnType>>;

			if constexpr (sizeof(FilteredReturnType) <= sizeof(ReturnValue))
			{
				reinterpret_cast<FilteredReturnType&>(m_registers[0]).~FilteredReturnType();
			}
			else
			{
				delete reinterpret_cast<FilteredReturnType*>(m_registers[0]);
			}
		}

		template<auto Function>
		FORCE_INLINE void Invoke()
		{
			const ReturnValue returnValue =
				DynamicInvoke::Invoke<Function>(m_registers[0], m_registers[1], m_registers[2], m_registers[3], m_registers[4], m_registers[5]);
			m_registers[0] = returnValue.x;
			m_registers[1] = returnValue.y;
			m_registers[2] = returnValue.z;
			m_registers[3] = returnValue.w;
		}

		template<typename Function>
		FORCE_INLINE void InvokeGeneric(Function&& function)
		{
			const ReturnValue returnValue =
				Forward<Function>(function)(m_registers[0], m_registers[1], m_registers[2], m_registers[3], m_registers[4], m_registers[5]);
			m_registers[0] = returnValue.x;
			m_registers[1] = returnValue.y;
			m_registers[2] = returnValue.z;
			m_registers[3] = returnValue.w;
		}

		[[nodiscard]] FORCE_INLINE Register operator[](const uint8 index) const
		{
			return m_registers[index];
		}

		[[nodiscard]] FORCE_INLINE Register& operator[](const uint8 index)
		{
			return m_registers[index];
		}

		Array<Register, RegisterCount> m_registers;
	};
}
