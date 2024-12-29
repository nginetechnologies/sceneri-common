#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename FunctionType>
		struct GetMemberType
		{
			using Type = void;
		};

		template<typename ReturnType, typename OwnerType>
		struct GetMemberType<ReturnType OwnerType::*>
		{
			using Type = ReturnType;
		};

		template<typename ReturnType, typename OwnerType>
		struct GetMemberType<ReturnType OwnerType::*const>
		{
			using Type = ReturnType;
		};
	}

	template<typename Type>
	using MemberType = typename Internal::GetMemberType<Type>::Type;
}
