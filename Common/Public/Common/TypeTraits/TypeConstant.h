#pragma once

namespace ngine::TypeTraits
{
	template<typename _Type, _Type _Value>
	struct TypeConstant
	{
		using Type = _Type;
		inline static constexpr Type Value = _Value;
		[[nodiscard]] FORCE_INLINE constexpr operator Type() const noexcept
		{
			return Value;
		}
	};

	using TrueType = TypeConstant<bool, true>;
	using FalseType = TypeConstant<bool, false>;
}
