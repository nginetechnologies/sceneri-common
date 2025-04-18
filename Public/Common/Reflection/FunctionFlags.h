#pragma once

#include <Common/EnumFlagOperators.h>

namespace ngine::Reflection
{
	enum class FunctionFlags : uint16
	{
		//! Enables remote calling of this function from a client to the host
		//! Note: In bound object contexts this is only allowed if the client has authority, or if AllowClientToHostWithoutAuthority is set on
		//! both peers
		ClientToHost = 1 << 0,
		//! Enables remote calling of this function from the host to one or more clients
		HostToClient = 1 << 1,
		//! Enables remote calling of this function from client to client
		//! Only allowed when the client has authority in the specified context
		//! Note: This may require a call to the server and then broadcast to other clients
		ClientToClient = 1 << 2,
		//! Whether this function can be called from client to host even if the client doesn't have authority
		AllowClientToHostWithoutAuthority = 1 << 3,
		IsMemberFunction = 1 << 4,
		//! Whether the function should be visible and usable from UI (i.e. logic graphs)
		VisibleToUI = 1 << 5,
		//! Whether the function does not modify any state
		//! This does not affect compilation in any way, see PURE_NOSTATICS, PURE_STATICS and more for that.
		IsPure = 1 << 6,
		//! Whether this function is an event getter
		IsEvent = 1 << 7,
		//! Whether  the function was defined from a script (otherwise it is assumed native)
		IsScript = 1 << 8,
		IsNetworkedMask = ClientToHost | HostToClient | ClientToClient
	};
	ENUM_FLAG_OPERATORS(FunctionFlags);
};
