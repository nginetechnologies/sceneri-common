#pragma once

#include "Register.h"

#include <Common/Memory/ReferenceWrapper.h>
#include <Common/Memory/Containers/ByteView.h>
#include <Common/Memory/Allocators/Allocate.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/TypeTraits/Decay.h>
#include <Common/TypeTraits/Select.h>
#include <Common/TypeTraits/IsMemberFunction.h>
#include <Common/TypeTraits/IsFunctionPointer.h>
#include <Common/TypeTraits/IsMemberVariable.h>
#include <Common/TypeTraits/IsReference.h>
#include <Common/TypeTraits/MemberOwnerType.h>
#include <Common/TypeTraits/MemberType.h>
#include <Common/TypeTraits/GetFunctionSignature.h>
#include <Common/TypeTraits/HasFunctionCallOperator.h>

namespace ngine::Scripting::VM
{
	namespace DynamicInvoke
	{
		template<typename ArgumentType>
		[[nodiscard]] FORCE_INLINE static ArgumentType ExtractArgument(Register argumentRegister)
		{
			if constexpr (!TypeTraits::IsSame<TypeTraits::WithoutReference<ArgumentType>, ArgumentType>)
			{
				using ValueType = TypeTraits::WithoutReference<ArgumentType>;
				return ReferenceWrapper<ValueType>(*reinterpret_cast<ValueType*>(*reinterpret_cast<uintptr*>(&argumentRegister)));
			}
			else if constexpr (sizeof(ArgumentType) <= sizeof(argumentRegister))
			{
				return reinterpret_cast<ArgumentType&>(argumentRegister);
			}
			else
			{
				return Move(*ExtractArgument<ArgumentType*>(argumentRegister));
			}
		}

		template<uint8 Index, typename ArgumentType>
		[[nodiscard]] FORCE_INLINE static ArgumentType
		ExtractArgument(const Register R0, const Register R1, const Register R2, const Register R3, const Register R4, const Register R5)
		{
			static_assert(Index < RegisterCount);
			switch (Index)
			{
				case 0:
					return ExtractArgument<ArgumentType>(R0);
				case 1:
					return ExtractArgument<ArgumentType>(R1);
				case 2:
					return ExtractArgument<ArgumentType>(R2);
				case 3:
					return ExtractArgument<ArgumentType>(R3);
				case 4:
					return ExtractArgument<ArgumentType>(R4);
				case 5:
					return ExtractArgument<ArgumentType>(R5);
			}
			ExpectUnreachable();
		}

		template<size Index, typename... ArgumentTypes>
		struct ExtractArgs
		{
			template<typename... ExtractedArgs>
			FORCE_INLINE static auto Extract(
				const Register R0,
				const Register R1,
				const Register R2,
				const Register R3,
				const Register R4,
				const Register R5,
				ExtractedArgs&&... args
			) noexcept -> decltype(auto)
			{
				using TupleType = Tuple<ArgumentTypes...>;
				using ElementType = typename TupleType::template ElementType<Index - 1>;
				return ExtractArgs<Index - 1, ArgumentTypes...>::Extract(
					R0,
					R1,
					R2,
					R3,
					R4,
					R5,
					ExtractArgument<Index - 1, ElementType>(R0, R1, R2, R3, R4, R5),
					Forward<ExtractedArgs>(args)...
				);
			}
		};

		template<typename... ArgumentTypes>
		struct ExtractArgs<0, ArgumentTypes...>
		{
			template<typename... ExtractedArgs>
			FORCE_INLINE static Tuple<ArgumentTypes...>
			Extract(Register, Register, Register, Register, Register, Register, ExtractedArgs&&... args) noexcept
			{
				return Tuple<ArgumentTypes...>(Forward<ExtractedArgs>(args)...);
			};
		};

		template<typename... Arguments>
		[[nodiscard]] FORCE_INLINE static Tuple<Arguments...>
		ExtractArguments(const Register R0, const Register R1, const Register R2, const Register R3, const Register R4, const Register R5)
		{
			return ExtractArgs<sizeof...(Arguments), Arguments...>::Extract(R0, R1, R2, R3, R4, R5);
		}

		template<typename ReturnType>
		[[nodiscard]] FORCE_INLINE ReturnType ExtractReturnType(ReturnValue returnValue)
		{
			if constexpr (!TypeTraits::IsSame<TypeTraits::WithoutReference<ReturnType>, ReturnType>)
			{
				using ValueType = TypeTraits::WithoutReference<ReturnType>;
				return ReferenceWrapper<ValueType>(*reinterpret_cast<ValueType*>(*reinterpret_cast<uintptr*>(&returnValue)));
			}
			else if constexpr (sizeof(ReturnType) <= sizeof(returnValue))
			{
				return reinterpret_cast<ReturnType&>(returnValue);
			}
			else
			{
				return *ExtractReturnType<ReturnType*>(returnValue);
			}
		}

		//! Loads an argument and returns a register containing the value
		template<typename ArgumentType>
		[[nodiscard]] FORCE_INLINE static Register LoadArgument(ArgumentType argument)
		{
			static constexpr bool IsReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<ArgumentType>, ArgumentType>;
			using FilteredArgumentType =
				TypeTraits::Select<IsReference, ReferenceWrapper<TypeTraits::WithoutReference<ArgumentType>>, ArgumentType>;

			if constexpr (sizeof(FilteredArgumentType) <= sizeof(Register))
			{
				struct Union
				{
					Union()
					{
					}
					~Union()
					{
					}
					union
					{
						Register m_register;
						FilteredArgumentType m_argument;
					};
				};

				Union tempUnion;
				tempUnion.m_argument = Forward<ArgumentType>(argument);
				return tempUnion.m_register;
			}
			else
			{
				return LoadArgument(new ArgumentType(Forward<ArgumentType>(argument)));
			}
		}

		[[nodiscard]] FORCE_INLINE static Register LoadDynamicArgument(const ConstByteView data)
		{
			if (data.GetDataSize() <= sizeof(Register))
			{
				Register result;
				ByteView::Make(result).CopyFrom(data);
				return result;
			}
			else
			{
				ByteType* pData = static_cast<ByteType*>(Memory::Allocate(data.GetDataSize()));
				ByteView(pData, data.GetDataSize()).CopyFrom(data);
				return LoadArgument(pData);
			}
		}

		[[nodiscard]] FORCE_INLINE static Register LoadDynamicArgumentZeroed(const ConstByteView data)
		{
			if (data.GetDataSize() <= sizeof(Register))
			{
				Register tempRegister;
				Memory::Set(&tempRegister, 0, sizeof(Register));

				ByteView::Make(tempRegister).CopyFrom(data);
				return tempRegister;
			}
			else
			{
				ByteType* pData = static_cast<ByteType*>(Memory::Allocate(data.GetDataSize()));
				ByteView(pData, data.GetDataSize()).CopyFrom(data);
				return LoadArgument(pData);
			}
		}

		//! Loads an argument and returns a register containing the value
		//! The register is zeroed before applying the data to ensure the Register value is bitwise equal every time
		template<typename ArgumentType>
		[[nodiscard]] FORCE_INLINE static Register LoadArgumentZeroed(ArgumentType argument)
		{
			static constexpr bool IsReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<ArgumentType>, ArgumentType>;
			using FilteredArgumentType =
				TypeTraits::Select<IsReference, ReferenceWrapper<TypeTraits::WithoutReference<ArgumentType>>, ArgumentType>;

			if constexpr (sizeof(FilteredArgumentType) <= sizeof(Register))
			{
				struct Union
				{
					Union()
					{
					}
					~Union()
					{
					}
					union
					{
						Register m_register;
						FilteredArgumentType m_argument;
					};
				};

				Union tempUnion;
				if constexpr (sizeof(FilteredArgumentType) < sizeof(Register))
				{
					Memory::Set(&tempUnion, 0, sizeof(Union));
				}
				new (&tempUnion.m_argument) FilteredArgumentType(Forward<ArgumentType>(argument));
				return tempUnion.m_register;
			}
			else
			{
				return LoadArgument(new ArgumentType(Forward<ArgumentType>(argument)));
			}
		}

		[[nodiscard]] FORCE_INLINE static Register LoadArgument(Register argument)
		{
			return argument;
		}

		template<typename ArgumentType>
		[[nodiscard]] FORCE_INLINE static ReturnValue LoadReturnValue(ArgumentType argument)
		{
			static constexpr bool IsReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<ArgumentType>, ArgumentType>;
			using FilteredArgumentType =
				TypeTraits::Select<IsReference, ReferenceWrapper<TypeTraits::WithoutReference<ArgumentType>>, ArgumentType>;

			if constexpr (sizeof(FilteredArgumentType) <= sizeof(ReturnValue))
			{
				struct Union
				{
					Union()
					{
					}
					union
					{
						ReturnValue m_returnValue;
						FilteredArgumentType m_argument;
					};
				};

				Union tempUnion;
				tempUnion.m_argument = Forward<ArgumentType>(argument);
				return tempUnion.m_returnValue;
			}
			else
			{
				return LoadReturnValue(new ArgumentType(Forward<ArgumentType>(argument)));
			}
		}

		[[nodiscard]] FORCE_INLINE static ReturnValue LoadReturnValue(Register argument)
		{
			return ReturnValue{argument, {}, {}, {}};
		}

		[[nodiscard]] FORCE_INLINE static ReturnValue LoadReturnValue(ReturnValue argument)
		{
			return argument;
		}

		template<auto Function, size Index, typename ArgumentTypesTuple, typename... Vs>
		FORCE_INLINE static auto CallFunction(
			const Register R0, const Register R1, const Register R2, const Register R3, const Register R4, const Register R5, Vs&&... args
		) noexcept -> decltype(auto)
		{
			if constexpr (Index > 0)
			{
				using ElementType = typename ArgumentTypesTuple::template ElementType<Index - 1>;
				return CallFunction<Function, Index - 1, ArgumentTypesTuple>(
					R0,
					R1,
					R2,
					R3,
					R4,
					R5,
					ExtractArgument<Index - 1, ElementType>(R0, R1, R2, R3, R4, R5),
					Forward<Vs>(args)...
				);
			}
			else
			{
				return Function(Forward<Vs>(args)...);
			}
		}

		template<typename OwnerType, auto Function, size Index, typename ArgumentTypesTuple, typename... Vs>
		FORCE_INLINE static auto CallMemberFunction(
			OwnerType& object,
			const Register R0,
			const Register R1,
			const Register R2,
			const Register R3,
			const Register R4,
			const Register R5,
			Vs&&... args
		) noexcept -> decltype(auto)
		{
			if constexpr (Index > 0)
			{
				using ElementType = typename ArgumentTypesTuple::template ElementType<Index - 1>;
				return CallMemberFunction<OwnerType, Function, Index - 1, ArgumentTypesTuple>(
					object,
					R0,
					R1,
					R2,
					R3,
					R4,
					R5,
					ExtractArgument<Index, ElementType>(R0, R1, R2, R3, R4, R5),
					Forward<Vs>(args)...
				);
			}
			else
			{
				using FunctionType = decltype(Function);
				using FunctionValueType = TypeTraits::WithoutConstOrVolatile<FunctionType>;
				constexpr FunctionValueType function = Function;
				return (object.*function)(Forward<Vs>(args)...);
			}
		}

		template<auto Function>
		[[nodiscard]] FORCE_INLINE static ReturnValue
		Invoke(Register R0, const Register R1, const Register R2, const Register R3, const Register R4, const Register R5)
		{
			using FunctionType = decltype(Function);
			using FunctionValueType = TypeTraits::WithoutConstOrVolatile<FunctionType>;
			using ArgumentTypes = typename TypeTraits::GetFunctionSignature<FunctionValueType>::ArgumentTypes;
			using ReturnType = typename TypeTraits::GetFunctionSignature<FunctionValueType>::ReturnType;
			static constexpr bool IsReturnTypeReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<ReturnType>, ReturnType>;
			using FilteredReturnType =
				TypeTraits::Select<IsReturnTypeReference, ReferenceWrapper<TypeTraits::WithoutReference<ReturnType>>, ReturnType>;

			if constexpr (TypeTraits::IsMemberFunction<FunctionValueType>)
			{
				using OwnerType = TypeTraits::MemberOwnerType<FunctionValueType>;
				static constexpr bool IsLambda = TypeTraits::HasFunctionCallOperator<OwnerType> && TypeTraits::IsCopyConstructible<OwnerType>;

				OwnerType* pObject;
				if constexpr (IsLambda)
				{
					pObject = reinterpret_cast<OwnerType*>(&R0);
				}
				else
				{
					pObject = Memory::GetAddressOf(ExtractArgument<OwnerType&>(R0));
				}

				constexpr FunctionValueType function = Function;
				if constexpr (TypeTraits::IsSame<ReturnType, void>)
				{
					CallMemberFunction<OwnerType, function, ArgumentTypes::ElementCount, ArgumentTypes>(*pObject, R0, R1, R2, R3, R4, R5);
					return ReturnValue{R0, R1, R2, R3};
				}
				else
				{
					return LoadReturnValue<FilteredReturnType>(
						CallMemberFunction<OwnerType, function, ArgumentTypes::ElementCount, ArgumentTypes>(*pObject, R0, R1, R2, R3, R4, R5)
					);
				}
			}
			else
			{
				if constexpr (TypeTraits::IsSame<ReturnType, void>)
				{
					CallFunction<Function, ArgumentTypes::ElementCount, ArgumentTypes>(R0, R1, R2, R3, R4, R5);
					return ReturnValue{R0, R1, R2, R3};
				}
				else
				{
					return LoadReturnValue<FilteredReturnType>(
						CallFunction<Function, ArgumentTypes::ElementCount, ArgumentTypes>(R0, R1, R2, R3, R4, R5)
					);
				}
			}
		}

		template<auto Variable>
		[[nodiscard]] FORCE_INLINE static ReturnValue
		InvokeGetMemberVariable(Register R0, const Register R1, const Register R2, const Register R3, const Register R4, const Register R5)
		{
			using VariableType = decltype(Variable);
			if constexpr (TypeTraits::IsMemberVariable<VariableType>)
			{
				using MemberValueType = TypeTraits::WithoutConstOrVolatile<TypeTraits::MemberType<VariableType>>;
				static constexpr bool IsValueTypeReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<MemberValueType>, MemberValueType>;
				using FilteredValueType = TypeTraits::
					Select<IsValueTypeReference, ReferenceWrapper<TypeTraits::WithoutReference<MemberValueType>>, ReferenceWrapper<MemberValueType>>;

				using OwnerType = TypeTraits::MemberOwnerType<VariableType>;

				OwnerType* pObject = Memory::GetAddressOf(ExtractArgument<OwnerType&>(R0));

				constexpr VariableType member = Variable;
				return LoadReturnValue<FilteredValueType>(pObject->*member);
			}
			else if constexpr (TypeTraits::IsFunctionPointer<VariableType>)
			{
				return Invoke<Variable>(R0, R1, R2, R3, R4, R5);
			}
			else
			{
				using MemberValueType = TypeTraits::WithoutConstOrVolatile<VariableType>;
				static constexpr bool IsValueTypeReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<MemberValueType>, MemberValueType>;
				using FilteredValueType = TypeTraits::
					Select<IsValueTypeReference, ReferenceWrapper<TypeTraits::WithoutReference<MemberValueType>>, ReferenceWrapper<MemberValueType>>;

				return LoadReturnValue<FilteredValueType>(*Variable);
			}
		}
	}
}
