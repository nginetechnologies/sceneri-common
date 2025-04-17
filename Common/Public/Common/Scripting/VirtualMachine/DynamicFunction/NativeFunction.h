#pragma once

#include "DynamicFunction.h"

namespace ngine::Scripting::VM
{
	namespace Internal
	{
		template<typename ProvidedArgumentsTuple, typename ArgumentsTuple, size Index>
		void ValidateFunctionArguments()
		{
			static_assert(
				TypeTraits::
					IsSame<typename ProvidedArgumentsTuple::template ElementType<Index>, typename ArgumentsTuple::template ElementType<Index>>,
				"Argument type mismatch!"
			);
			if constexpr (Index + 1 < ProvidedArgumentsTuple::ElementCount)
			{
				ValidateFunctionArguments<ProvidedArgumentsTuple, ArgumentsTuple, Index + 1>();
			}
		}
	}

	template<typename SignatureType>
	struct NativeFunction;

	template<typename ReturnType, typename... ArgumentTypes>
	struct NativeFunction<ReturnType(ArgumentTypes...)> : private DynamicFunction
	{
		using BaseType = DynamicFunction;
		using BaseType::BaseType;
		NativeFunction(const DynamicFunction function)
			: DynamicFunction(function)
		{
		}

		template<auto Function>
		FORCE_INLINE static constexpr NativeFunction Make()
		{
			static_assert(TypeTraits::IsSame<TypeTraits::ReturnType<decltype(Function)>, ReturnType>, "Return type did not match signature!");
			using ProvidedArgumentTypesTuple = TypeTraits::GetParameterTypes<decltype(Function)>;
			static_assert(ProvidedArgumentTypesTuple::ElementCount == sizeof...(ArgumentTypes), "Argument count mismatched!");
			Internal::ValidateFunctionArguments<ProvidedArgumentTypesTuple, Tuple<ArgumentTypes...>, 0>();
			return DynamicInvoke::Invoke<Function>;
		}

		ReturnType operator()(ArgumentTypes&&... args) const
		{
			return BaseType::Invoke<ReturnType, ArgumentTypes...>(Forward<ArgumentTypes>(args)...);
		}

		using BaseType::IsValid;
		using BaseType::operator!=;
		using BaseType::operator==;

		[[nodiscard]] DynamicFunction GetDynamic() const
		{
			return *this;
		}
	};
}
