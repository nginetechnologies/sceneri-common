#pragma once

#include <Common/EnumFlagOperators.h>

namespace ngine::Reflection
{
	enum class PropertyFlags : uint8
	{
		NotSavedToDisk = 1 << 0,
		NotReadFromDisk = 1 << 1,
		Transient = NotSavedToDisk | NotReadFromDisk,
		HideFromUI = 1 << 2,
		//! Whether this property should be sent over the network when the owner is sent as an arugment in a networked function call
		SentWithNetworkedFunctions = 1 << 3,
		//! Enables propagation of this property over the network from client to host
		PropagateClientToHost = 1 << 4,
		//! Enables propagation of this property over the network from host to one or more clients
		PropagateHostToClient = 1 << 5,
		//! Enables propagation of this property over the network from client to client
		//! Only allowed when the client has authority in the specified context
		//! Note: This may require first propagating to the server and then broadcast to other clients
		PropagateClientToClient = 1 << 6,
		//! Whether parents of our owner should be aware of this property's existence
		VisibleToParentScope = 1 << 7,

		IsNetworkPropagatedMask = PropagateClientToHost | PropagateHostToClient | PropagateClientToClient
	};
	ENUM_FLAG_OPERATORS(PropertyFlags);
};
