#pragma once

#include "Context.h"

namespace ngine::Asset
{
	struct Database;

	struct Owners
	{
		Owners() = default;
		Owners(const IO::PathView metaAssetFile, Context&& context);

		[[nodiscard]] bool HasOwner() const
		{
			return m_context.IsValid();
		}

		Context m_context;

		[[nodiscard]] IO::Path GetProjectLauncherPath() const;
		[[nodiscard]] IO::Path GetDatabasePath() const;
		[[nodiscard]] IO::Path GetDatabaseRootDirectory() const;
		[[nodiscard]] IO::Path GetAssetDirectoryPath() const;
		[[nodiscard]] bool IsAssetInDatabase(const Guid assetGuid) const;
	};
}
