#pragma once

namespace ngine
{
	template<typename SignatureType, typename... ArgumentTypes>
	struct FunctionPointer;

	template<typename ReturnType, typename... ArgumentTypes>
	struct FunctionPointer<ReturnType(ArgumentTypes...)>;
}
