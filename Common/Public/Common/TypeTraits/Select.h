#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<bool B, typename TrueType, typename FalseType>
		struct Select
		{
			using Type = TrueType;
		};

		template<typename TrueType, typename FalseType>
		struct Select<false, TrueType, FalseType>
		{
			using Type = FalseType;
		};
	}

	template<bool Check, typename TrueType, typename FalseType>
	using Select = typename Internal::Select<Check, TrueType, FalseType>::Type;
}
