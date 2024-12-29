#pragma once

namespace ngine::TypeTraits
{
	struct Nonesuch
	{
		Nonesuch() = delete;
		~Nonesuch() = delete;
		Nonesuch(const Nonesuch&) = delete;
		void operator=(const Nonesuch&) = delete;
	};
}
