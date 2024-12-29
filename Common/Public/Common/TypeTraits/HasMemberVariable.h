#pragma once

namespace ngine::TypeTraits
{
#define HasMemberVariable(ObjectType, VariableName) \
	template<typename Type__ = ObjectType> \
	static auto checkHas##VariableName(int) -> decltype(Type__::VariableName, uint8()); \
	template<typename Type__ = ObjectType> \
	static uint16 checkHas##VariableName(...); \
	inline static constexpr bool Has##VariableName = sizeof(checkHas##VariableName<ObjectType>(0)) == sizeof(uint8);
}
