#pragma once

#include "../Version.h"
#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>
#include <Common/Memory/Containers/FlatString.h>
#include <Common/Memory/Containers/Format/String.h>

namespace ngine
{
	inline FlatString<15> Version::ToString() const
	{
		return FlatString<15>().Format("{0}.{1}.{2}", GetMajor(), GetMinor(), GetPatch());
	}

	inline bool Version::Serialize(const Serialization::Reader serializer)
	{
		const Serialization::Value& __restrict currentElement = serializer.GetValue();
		Assert(currentElement.IsString());
		const ConstStringView versionString = ConstStringView(currentElement.GetString(), currentElement.GetStringLength());
		const ConstStringView::SizeType firstDelimiterIndex = versionString.FindFirstOf('.');

		const ConstStringView majorVersionString = versionString.GetSubstring(0, firstDelimiterIndex);
		const ConstStringView::SizeType secondDelimiterIndex =
			versionString.FindFirstOf('.', firstDelimiterIndex + (ConstStringView::SizeType)1u);

		const ConstStringView minorVersionString = firstDelimiterIndex != ConstStringView::InvalidPosition
		                                             ? versionString.GetSubstring(firstDelimiterIndex + 1, secondDelimiterIndex - 1)
		                                             : "0";
		const ConstStringView patchVersionString =
			secondDelimiterIndex != ConstStringView::InvalidPosition
				? versionString.GetSubstring(secondDelimiterIndex + 1, versionString.GetSize() - secondDelimiterIndex - 1)
				: "0";

		const uint16 majorVersion = majorVersionString.ToIntegral<uint16>();
		const uint16 minorVersion = minorVersionString.ToIntegral<uint16>();
		const uint16 patchVersion = patchVersionString.ToIntegral<uint16>();

		*this = Version(majorVersion, minorVersion, patchVersion);
		return true;
	}

	inline bool Version::Serialize(Serialization::Writer serializer) const
	{
		ngine::FlatString<15> versionString = ToString();
		Serialization::Value& __restrict currentElement = serializer.GetValue();
		currentElement = Serialization::Value(versionString.GetData(), versionString.GetSize(), serializer.GetDocument().GetAllocator());
		return true;
	}
}
