#pragma once

#include <Common/Memory/ForwardDeclarations/Tuple.h>

namespace ngine::TypeTraits
{
	template<typename FunctionType>
	struct GetFunctionSignature
	{
		using CallableOperatorFunctionSignature = GetFunctionSignature<decltype(&FunctionType::operator())>;
		using ArgumentTypes = typename CallableOperatorFunctionSignature::ArgumentTypes;
		using ReturnType = typename CallableOperatorFunctionSignature::ReturnType;
		using SignatureType = typename CallableOperatorFunctionSignature::SignatureType;
	};

	template<typename ReturnType_, typename... Arguments>
	struct GetFunctionSignature<ReturnType_(Arguments...)>
	{
		using ArgumentTypes = Tuple<Arguments...>;
		using ReturnType = ReturnType_;
		using SignatureType = ReturnType_(Arguments...);
	};

	template<typename ReturnType_, typename... Arguments>
	struct GetFunctionSignature<ReturnType_ (*)(Arguments...)>
	{
		using ArgumentTypes = Tuple<Arguments...>;
		using ReturnType = ReturnType_;
		using SignatureType = ReturnType_(Arguments...);
	};

	template<typename ReturnType_, typename... Arguments>
	struct GetFunctionSignature<ReturnType_ (&)(Arguments...)>
	{
		using ArgumentTypes = Tuple<Arguments...>;
		using ReturnType = ReturnType_;
		using SignatureType = ReturnType_(Arguments...);
	};

	template<typename ClassType, typename ReturnType_, typename... Arguments>
	struct GetFunctionSignature<ReturnType_ (ClassType::*)(Arguments...)>
	{
		using ArgumentTypes = Tuple<Arguments...>;
		using ReturnType = ReturnType_;
		using SignatureType = ReturnType_(Arguments...);
	};

	template<typename ClassType, typename ReturnType_, typename... Arguments>
	struct GetFunctionSignature<ReturnType_ (ClassType::*)(Arguments...) const>
	{
		using ArgumentTypes = Tuple<Arguments...>;
		using ReturnType = ReturnType_;
		using SignatureType = ReturnType_(Arguments...);
	};
}
