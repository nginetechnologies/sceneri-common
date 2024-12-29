#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Memory/Containers/ForwardDeclarations/FlatString.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine
{
	struct Version
	{
		constexpr Version(uint16 major, uint16 minor, uint16 patch)
			: m_version((static_cast<uint32>(major) << 22) | (static_cast<uint32>(minor) << 12) | static_cast<uint32>(patch))
		{
		}

		[[nodiscard]] constexpr uint16 GetMajor() const
		{
			return static_cast<uint16>(m_version >> 22);
		}
		[[nodiscard]] constexpr uint16 GetMinor() const
		{
			return static_cast<uint16>((m_version >> 12) & 0x3ff);
		}
		[[nodiscard]] constexpr uint16 GetPatch() const
		{
			return static_cast<uint16>((m_version) & 0xfff);
		}

		[[nodiscard]] FlatString<15> ToString() const;
		bool Serialize(const Serialization::Reader serializer);
		bool Serialize(Serialization::Writer serializer) const;
	protected:
		uint32 m_version;
	};
}
