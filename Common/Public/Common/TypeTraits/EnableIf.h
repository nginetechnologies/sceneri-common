#pragma once

namespace ngine
{
	namespace Internal
	{
		template<bool B, class T = void>
		struct EnableIf
		{
		};

		template<class T>
		struct EnableIf<true, T>
		{
			using Type = T;
		};
	}

	template<bool B, class T = void>
	using EnableIf = typename Internal::EnableIf<B, T>::Type;
}
