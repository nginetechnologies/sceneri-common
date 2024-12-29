#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename Type, typename AssignedType>
		struct IsTriviallyAssignable
		{
			inline static constexpr bool Value = __is_trivially_assignable(Type, AssignedType);
		};
	}

	template<typename Type, typename AssignedType>
	inline static constexpr bool IsTriviallyAssignable = Internal::IsTriviallyAssignable<Type, AssignedType>::Value;
	template<typename Type>
	inline static constexpr bool IsTriviallyCopyAssignable = Internal::IsTriviallyAssignable<Type&, const Type&>::Value;
	template<typename Type>
	inline static constexpr bool IsTriviallyCopyAssignable = Internal::IsTriviallyAssignable<Type&, Type&&>::Value;
}
