#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename FunctionType>
		struct GetMemberOwnerType
		{
			using Type = void;
		};

		template<typename ReturnType, typename OwnerType>
		struct GetMemberOwnerType<ReturnType OwnerType::*>
		{
			using Type = OwnerType;
		};

		template<typename ReturnType, typename OwnerType>
		struct GetMemberOwnerType<ReturnType OwnerType::*const>
		{
			using Type = OwnerType;
		};

		template<typename OwnerType, typename ReturnType, typename... Arguments>
		struct GetMemberOwnerType<ReturnType (OwnerType::*)(Arguments...)>
		{
			using Type = OwnerType;
		};

		template<typename OwnerType, typename ReturnType, typename... Arguments>
		struct GetMemberOwnerType<ReturnType (OwnerType::*)(Arguments...) const>
		{
			using Type = OwnerType;
		};
	}

	template<typename Type>
	using MemberOwnerType = typename Internal::GetMemberOwnerType<Type>::Type;
}
