#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<bool Condition, typename TrueType, typename FalseType>
		struct ConditionalType
		{
			using Type = TrueType;
		};

		template<typename TrueType, typename FalseType>
		struct ConditionalType<false, TrueType, FalseType>
		{
			using Type = FalseType;
		};
	}

	template<bool Condition, typename TrueType, typename FalseType>
	using ConditionalType = typename Internal::ConditionalType<Condition, TrueType, FalseType>::Type;
}
