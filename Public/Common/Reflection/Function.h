#pragma once

#include "TypeDefinition.h"
#include "FunctionInfo.h"

#include <Common/Memory/Tuple.h>
#include <Common/TypeTraits/GetFunctionSignature.h>
#include <Common/TypeTraits/Select.h>
#include <Common/TypeTraits/MemberOwnerType.h>
#include <Common/TypeTraits/IsMemberFunction.h>
#include <Common/Memory/CallMemberFunctionWithTuple.h>
#include <Common/Memory/ReferenceWrapper.h>

namespace ngine::Reflection
{
	template<auto Function>
	struct ReflectedFunction;

	template<typename FunctionType_, typename ReturnTypeInfo, typename... Arguments>
	struct Function : public FunctionInfo
	{
		using FunctionType = FunctionType_;
	protected:
		template<typename Type>
		[[nodiscard]] inline static constexpr Reflection::Argument PopulateArgumentInfo(Reflection::Argument&& argumentInfo)
		{
			constexpr bool IsReference = !TypeTraits::IsSame<TypeTraits::WithoutReference<Type>, Type>;
			using FilteredType = typename TypeTraits::Select<IsReference, ReferenceWrapper<TypeTraits::WithoutReference<Type>>, Type>;
			argumentInfo.m_type = Reflection::TypeDefinition::Get<FilteredType>();

#if COMPILER_CLANG
			// Workaround for Clang 11 bug, remove when Steam runtime is upgraded
			return Move(argumentInfo);
#else
			return argumentInfo;
#endif
		}

		template<size Index, typename ArgumentTypeTuple, typename ArgumentInfoTuple>
		inline static constexpr void EmplaceArgumentInfo(Array<Argument, sizeof...(Arguments)>& arguments, ArgumentInfoTuple&& remainingInfo)
		{
			arguments[Index] =
				PopulateArgumentInfo<typename ArgumentTypeTuple::template ElementType<Index>>(Move(remainingInfo.template Get<Index>()));

			if constexpr ((Index + 1) < ArgumentTypeTuple::ElementCount)
			{
				EmplaceArgumentInfo<Index + 1, ArgumentTypeTuple>(arguments, Forward<ArgumentInfoTuple>(remainingInfo));
			}
		}

		template<typename ArgumentTypeTuple, typename ArgumentInfoTuple>
		[[nodiscard]] inline static constexpr Array<Argument, sizeof...(Arguments)>
		PopulateArgumentInfos([[maybe_unused]] ArgumentInfoTuple&& argumentInfo)
		{
			static_assert(ArgumentTypeTuple::ElementCount == ArgumentInfoTuple::ElementCount);
			Array<Argument, sizeof...(Arguments)> arguments;
			if constexpr (sizeof...(Arguments) > 0)
			{
				EmplaceArgumentInfo<0, ArgumentTypeTuple>(arguments, Forward<ArgumentInfoTuple>(argumentInfo));
			}
			return arguments;
		}

		using GetFunctionSignatureType = TypeTraits::GetFunctionSignature<FunctionType>;
	public:
		constexpr Function(
			const Guid guid,
			const ConstUnicodeStringView displayName,
			FunctionType&& function,
			const EnumFlags<FunctionFlags> flags,
			ReturnTypeInfo&& returnType,
			Arguments&&... arguments
		)
			: FunctionInfo(
					guid,
					displayName,
					flags | (FunctionFlags::IsMemberFunction * TypeTraits::IsMemberFunction<FunctionType>),
					PopulateArgumentInfo<ReturnType>(Forward<ReturnTypeInfo>(returnType)),
					[](const FunctionInfo& functionInfo) -> ArrayView<const Argument, uint8>
					{
						const Function& function = static_cast<const Function&>(functionInfo);
						return function.m_arguments.GetDynamicView();
					}
				)
			, m_function(Forward<FunctionType>(function))
			, m_arguments(PopulateArgumentInfos<ArgumentTypes>(Tuple<Arguments...>{Forward<Arguments>(arguments)...}))
		{
		}

		[[nodiscard]] constexpr Guid GetGuid() const
		{
			return m_guid;
		}

		using SignatureType = typename GetFunctionSignatureType::SignatureType;
		using ArgumentTypes = typename GetFunctionSignatureType::ArgumentTypes;
		using ReturnType = typename GetFunctionSignatureType::ReturnType;
		using MemberOwnerType = TypeTraits::MemberOwnerType<FunctionType>;

		template<typename Function = FunctionType, typename = EnableIf<!TypeTraits::IsMemberFunction<Function>>>
		constexpr ReturnType operator()(ArgumentTypes&& arguments) const
		{
			return CallFunctionWithTuple(m_function, Forward<ArgumentTypes>(arguments));
		}

		template<typename Function = FunctionType, typename = EnableIf<TypeTraits::IsMemberFunction<Function>>>
		constexpr ReturnType operator()(MemberOwnerType* pObject, ArgumentTypes&& arguments) const
		{
			return CallMemberFunctionWithTuple(*pObject, m_function, Forward<ArgumentTypes>(arguments));
		}

		[[nodiscard]] constexpr FunctionType GetFunction() const
		{
			return m_function;
		}
		[[nodiscard]] constexpr EnumFlags<FunctionFlags> GetFlags() const
		{
			return m_flags;
		}

		[[nodiscard]] constexpr ArrayView<const Argument> GetArguments() const
		{
			return m_arguments;
		}
	private:
		const FunctionType m_function;
		Array<Argument, sizeof...(Arguments)> m_arguments;
	};

	template<typename... FunctionTypes>
	struct Functions : public Tuple<FunctionTypes...>
	{
		using BaseType = Tuple<FunctionTypes...>;
		using BaseType::BaseType;
		using BaseType::operator=;
	};
	template<typename... Ts>
	Functions(const Ts&...) -> Functions<Ts...>;
	template<typename... Ts>
	Functions(Ts&&...) -> Functions<Ts...>;
};
