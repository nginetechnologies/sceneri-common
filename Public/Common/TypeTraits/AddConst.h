#pragma once

#include <Common/TypeTraits/Void.h>

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<class _Ty>
		struct AddConst
		{
			using LeftType = const _Ty;
			using RightType = const _Ty;
		};

		template<class _Ty>
		struct AddConst<_Ty*>
		{
			using LeftType = const _Ty*;
			using RightType = _Ty* const;
		};
	}

	template<class _Ty>
	using AddLeftConst = typename Internal::AddConst<_Ty>::LeftType;

	template<class _Ty>
	using AddRightConst = typename Internal::AddConst<_Ty>::RightType;
}
