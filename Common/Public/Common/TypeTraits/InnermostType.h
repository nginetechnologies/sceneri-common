#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename T>
		struct TGetInnermostType
		{
			using Type = T;
		};
		template<template<typename> typename P, typename T>
		struct TGetInnermostType<P<T>>
		{
			using Type = typename TGetInnermostType<T>::Type;
		};
	}
	template<typename T>
	using InnermostType = typename Internal::TGetInnermostType<T>::Type;
}
