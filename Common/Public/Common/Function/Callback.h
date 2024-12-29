#pragma once

#include "ForwardDeclarations/Callback.h"
#include <Common/TypeTraits/GetParameterTypes.h>
#include <Common/TypeTraits/ReturnType.h>
#include <Common/TypeTraits/IsSame.h>

namespace ngine
{
	template<typename Function, typename ReturnType, typename... ArgumentTypes>
	struct Callback<Function, ReturnType(ArgumentTypes...)>
	{
		static_assert(TypeTraits::IsSame<TypeTraits::ReturnType<Function>, ReturnType>);
		static_assert(TypeTraits::IsSame<TypeTraits::GetParameterTypes<Function>, Tuple<ArgumentTypes...>>);

		constexpr Callback(const Function& function)
			: m_function(function)
		{
		}
		constexpr Callback(Function&& function)
			: m_function(Forward<Function>(function))
		{
		}

		constexpr ReturnType operator()(ArgumentTypes... args) const
		{
			return m_function(Forward<ArgumentTypes>(args)...);
		}
	private:
		Function m_function;
	};

	namespace Internal
	{
		template<typename ReturnType, typename... ArgumentTypes>
		struct ExtractFunctionSignature;

		template<typename ReturnType, typename... ArgumentTypes>
		struct ExtractFunctionSignature<ReturnType, Tuple<ArgumentTypes...>>
		{
			using Signature = ReturnType(ArgumentTypes...);
		};
	}

	template<class Function>
	Callback(Function) -> Callback<
		Function,
		typename Internal::ExtractFunctionSignature<TypeTraits::ReturnType<Function>, TypeTraits::GetParameterTypes<Function>>::Signature>;
}
