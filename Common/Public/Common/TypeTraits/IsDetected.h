#pragma once

#include "Common/TypeTraits/Void.h"
#include "Common/TypeTraits/IsSame.h"
#include "Common/TypeTraits/Nonesuch.h"
#include "Common/TypeTraits/TypeConstant.h"

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<class Default, class AlwaysVoid, template<class...> class Op, class... Args>
		struct Detector
		{
			using ValueT = FalseType;
			using Type = Default;
		};

		template<class Default, template<class...> class Op, class... Args>
		struct Detector<Default, Void<Op<Args...>>, Op, Args...>
		{
			using ValueT = TrueType;
			using Type = Op<Args...>;
		};
	}

	template<template<class...> class Op, class... Args>
	using Detected = typename Internal::Detector<Nonesuch, void, Op, Args...>::Type;

	template<template<class...> class Op, class... Args>
	static constexpr bool IsDetected = Internal::Detector<Nonesuch, void, Op, Args...>::ValueT::Value;

	template<class Expected, template<class...> class Op, class... Args>
	static constexpr bool IsDetectedExact = IsSame<Expected, Detected<Op, Args...>>;
}
