#include "Guid.h"

#include <Common/Memory/Containers/FlatString.h>
#include <Common/Memory/Containers/Format/String.h>
#include <Common/Math/Hash.h>
#include <Common/Math/Random.h>
#include <Common/Time/Timestamp.h>
#include <Common/Reflection/TypeDefinition.h>

#include <Common/Serialization/Guid.h>

namespace ngine
{
	size Guid::Hash::operator()(const Guid& guid) const
	{
		const uint64 hipart = *reinterpret_cast<const uint64*>(&guid);
		const uint64 lopart = *(reinterpret_cast<const uint64*>(&guid) + 1);
		return Math::Hash(hipart, lopart);
	}

	FlatString<37> Guid::ToString() const
	{
		FlatString<37> result;
		result.Format(
			"{:08x}-{:04x}-{:04x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
			// Data0 (first 32 bits)
			uint32((m_data >> 96) & 0xFFFFFFFF),

			// Data1 (next 16 bits)
			uint16((m_data >> 80) & 0xFFFF),

			// Data2 (next 16 bits)
			uint16((m_data >> 64) & 0xFFFF),

			// Data3 (next 16 bits)
			uint8((m_data >> 56) & 0xFF),
			uint8((m_data >> 48) & 0xFF),

			// Data4 (remaining 48 bits)
			uint8((m_data >> 40) & 0xFF),
			uint8((m_data >> 32) & 0xFF),
			uint8((m_data >> 24) & 0xFF),
			uint8((m_data >> 16) & 0xFF),
			uint8((m_data >> 8) & 0xFF),
			uint8(m_data & 0xFF)
		);
		return result;
	}

	/* static */ Guid Guid::Generate()
	{
		// Generate a UUID v7 compatible GUID
		Guid guid;

		const uint64 timestampInMilliseconds = Time::Timestamp::GetCurrent().GetMilliseconds();

		// Fill in the timestamp in the first 6 bytes (48 bits)
		guid.m_data = (uint128(timestampInMilliseconds & 0xFFFFFFFFFFFF) << 80);

		// Set next 8 bytes
		guid.m_data |= uint128(Math::Random<uint64>()) << 16;
		// Set the last 2 bytes
		guid.m_data |= uint128(Math::Random<uint16>());

		// Indicate UUID version v7 by setting bits 4-7 in the 7th byte to 0111
		guid.m_data &= ~(uint128(0xF) << 76); // Clear bits 4-7 in the 7th byte
		guid.m_data |= (uint128(0x7) << 76);  // Set bits 4-7 to 0111

		// Set the variant to 10xx (RFC 4122) in the 9th byte
		guid.m_data &= ~(uint128(0xC0) << 64); // Clear the high 2 bits in the 9th byte
		guid.m_data |= (uint128(0x80) << 64);  // Set the high 2 bits to 10

		return guid;
	}

	namespace Reflection
	{
		size TypeDefinition::Hash::operator()(const TypeDefinition& typeDefinition) const
		{
			absl::Hash<ManagerFunction> hasher;
			return hasher(typeDefinition.m_manager);
		}
	}
}
