#pragma once

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename FunctionType>
		struct GetConstMemberVariablePointer;

		template<typename ReturnType, typename OwnerType>
		struct GetConstMemberVariablePointer<ReturnType OwnerType::*>
		{
			using Type = ReturnType OwnerType::*const;
		};

		template<typename ReturnType, typename OwnerType>
		struct GetConstMemberVariablePointer<ReturnType OwnerType::*const>
		{
			using Type = ReturnType OwnerType::*const;
		};
	}

	template<typename Type>
	using ConstMemberVariablePointer = typename Internal::GetConstMemberVariablePointer<Type>::Type;
}
