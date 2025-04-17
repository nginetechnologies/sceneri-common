#pragma once

#include "DynamicDelegate.h"

#include <Common/TypeTraits/IsInvocable.h>

namespace ngine::Scripting::VM
{
	template<typename SignatureType>
	struct NativeDelegate;

	template<typename ReturnType, typename... ArgumentTypes>
	struct NativeDelegate<ReturnType(ArgumentTypes...)> : private DynamicDelegate
	{
		using BaseType = DynamicDelegate;
		using BaseType::BaseType;
		NativeDelegate(DynamicDelegate&& base)
			: DynamicDelegate(Forward<DynamicDelegate>(base))
		{
		}

		template<auto Callback, typename ObjectType, typename = EnableIf<TypeTraits::IsInvocable<decltype(Callback), ArgumentTypes...>>>
		[[nodiscard]] static NativeDelegate Make(ObjectType& object)
		{
			return DynamicDelegate::Make<Callback, ObjectType>(object);
		}

		template<
			typename Identifier,
			typename CallbackObject,
			typename = EnableIf<
				TypeTraits::HasFunctionCallOperator<CallbackObject> &&
				TypeTraits::IsInvocable<decltype(CallbackObject::operator()), ArgumentTypes...>>>
		[[nodiscard]] static NativeDelegate Make(const Identifier identifier, CallbackObject&& callback)
		{
			return DynamicDelegate::Make<Identifier, CallbackObject>(identifier, Forward<CallbackObject>(callback));
		}

		ReturnType operator()(ArgumentTypes&&... args) const
		{
			if constexpr (TypeTraits::IsSame<ReturnType, void>)
			{
				BaseType::operator()(Forward<ArgumentTypes>(args)...);
			}
			else
			{
				const ReturnValue registerReturnValue = BaseType::operator()(Forward<ArgumentTypes>(args)...);
				Registers
					registers{registerReturnValue.x, registerReturnValue.y, registerReturnValue.z, registerReturnValue.w, Register{}, Register{}};
				ReturnType returnValue = Move(registers.ExtractReturnType<ReturnType>());
				registers.PopReturnType<ReturnType>();
				return Move(returnValue);
			}
		}

		using BaseType::operator==;
		using BaseType::operator!=;
		using BaseType::IsBoundToObject;

		[[nodiscard]] DynamicDelegate GetDynamic() const
		{
			return *this;
		}
	};
}
