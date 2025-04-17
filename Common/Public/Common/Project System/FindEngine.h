#pragma once

#include "EngineInfo.h"
#include "EngineAssetFormat.h"
#include "ProjectInfo.h"
#include "ProjectAssetFormat.h"
#include "PluginInfo.h"
#include "PluginAssetFormat.h"
#include "EngineDatabase.h"
#include <Common/CommandLine/CommandLineArguments.h>
#include <Common/Asset/Context.h>
#include <Common/Asset/AssetOwners.h>
#include <Common/IO/FileIterator.h>

namespace ngine::ProjectSystem
{
	[[nodiscard]] EngineInfo FindEngineFromProject(const ProjectInfo& projectInfo, const CommandLine::Arguments& commandLineArguments);
	[[nodiscard]] EngineInfo FindEngineFromProject(const ProjectInfo& projectInfo);
	[[nodiscard]] EngineInfo
	FindEngineFromPlugin([[maybe_unused]] const PluginInfo& plugin, const CommandLine::Arguments& commandLineArguments);
	[[nodiscard]] EngineInfo FindEngineFromPlugin([[maybe_unused]] const PluginInfo& plugin);

	IO::Path FindEngineConfigPathFromExecutableFolder(const IO::PathView executableFolderPath);
	IO::Path FindEngineDirectoryFromExecutableFolder(const IO::PathView executableFolderPath);

	//! Attempts to find the engine connected to the folder we are running from
	[[nodiscard]] inline EngineInfo FindLocalEngine()
	{
		const IO::Path executableDirectory = IO::Path::GetExecutableDirectory();
		const IO::Path buildFolder = IO::Path::Combine(executableDirectory, MAKE_PATH(".."), MAKE_PATH(".."));

		auto findEngine = [](const IO::Path buildFolder) -> EngineInfo
		{
			for (IO::FileIterator fileIterator(buildFolder); !fileIterator.ReachedEnd(); fileIterator.Next())
			{
				if (fileIterator.GetCurrentFileType() == IO::FileType::File)
				{
					IO::Path filePath = fileIterator.GetCurrentFilePath();
					const IO::PathView fileExtension = filePath.GetRightMostExtension();
					if (fileExtension == EngineAssetFormat.metadataFileExtension)
					{
						return Move(filePath);
					}
					else if (fileExtension == ProjectAssetFormat.metadataFileExtension)
					{
						ProjectInfo projectInfo(Move(filePath));
						return FindEngineFromProject(projectInfo);
					}
					else if (fileExtension == PluginAssetFormat.metadataFileExtension)
					{
						PluginInfo pluginInfo(Move(filePath));
						return FindEngineFromPlugin(pluginInfo);
					}
				}
			}

			return IO::Path();
		};

		EngineInfo engineInfo = findEngine(IO::Path(buildFolder));
		if (engineInfo.IsValid())
		{
			return engineInfo;
		}

		IO::Path engineConfigFilePath = FindEngineConfigPathFromExecutableFolder(executableDirectory);
		if (engineConfigFilePath.Exists())
		{
			engineInfo = EngineInfo(Move(engineConfigFilePath));
			if (engineInfo.IsValid())
			{
				return engineInfo;
			}
		}

		return {};
	}

	//! Attempts to find the engine connected to the folder we are running from
	[[nodiscard]] inline EngineInfo FindLocalEngine(const CommandLine::Arguments& commandLineArguments)
	{
		const IO::Path executableDirectory = IO::Path::GetExecutableDirectory();
		const IO::Path buildFolder = IO::Path::Combine(executableDirectory, MAKE_PATH(".."), MAKE_PATH(".."));

		auto findEngine = [&commandLineArguments](const IO::Path buildFolder) -> EngineInfo
		{
			for (IO::FileIterator fileIterator(buildFolder); !fileIterator.ReachedEnd(); fileIterator.Next())
			{
				if (fileIterator.GetCurrentFileType() == IO::FileType::File)
				{
					IO::Path filePath = fileIterator.GetCurrentFilePath();
					const IO::PathView fileExtension = filePath.GetRightMostExtension();
					if (fileExtension == EngineAssetFormat.metadataFileExtension)
					{
						return Move(filePath);
					}
					else if (fileExtension == ProjectAssetFormat.metadataFileExtension)
					{
						ProjectInfo projectInfo(Move(filePath));
						return FindEngineFromProject(projectInfo, commandLineArguments);
					}
					else if (fileExtension == PluginAssetFormat.metadataFileExtension)
					{
						PluginInfo pluginInfo(Move(filePath));
						return FindEngineFromPlugin(pluginInfo, commandLineArguments);
					}
				}
			}

			return IO::Path();
		};

		EngineInfo engineInfo = findEngine(IO::Path(buildFolder));
		if (engineInfo.IsValid())
		{
			return engineInfo;
		}

		IO::Path engineConfigFilePath = FindEngineConfigPathFromExecutableFolder(executableDirectory);
		if (engineConfigFilePath.Exists())
		{
			engineInfo = EngineInfo(Move(engineConfigFilePath));
			if (engineInfo.IsValid())
			{
				return engineInfo;
			}
		}

		return {};
	}

	[[nodiscard]] inline EngineInfo FindEngineFromProject(const ProjectInfo& projectInfo)
	{
		if (Optional<EngineInfo> engine = EngineDatabase().FindEngineConfig(projectInfo.GetEngineGuid()))
		{
			return Move(engine.Get());
		}

		return FindLocalEngine();
	}

	[[nodiscard]] inline EngineInfo FindEngineFromProject(const ProjectInfo& projectInfo, const CommandLine::Arguments& commandLineArguments)
	{
		if (OptionalIterator<const CommandLine::Argument> engineConfigPath = commandLineArguments.FindArgument(MAKE_NATIVE_LITERAL("engine"), CommandLine::Prefix::Minus))
		{
			return EngineInfo(IO::Path(engineConfigPath->value.GetView()));
		}

		if (Optional<EngineInfo> engine = EngineDatabase().FindEngineConfig(projectInfo.GetEngineGuid()))
		{
			return Move(engine.Get());
		}

		return FindLocalEngine(commandLineArguments);
	}

	[[nodiscard]] inline EngineInfo FindEngineFromPlugin([[maybe_unused]] const PluginInfo& plugin)
	{
		return FindLocalEngine();
	}

	[[nodiscard]] inline EngineInfo
	FindEngineFromPlugin([[maybe_unused]] const PluginInfo& plugin, const CommandLine::Arguments& commandLineArguments)
	{
		if (OptionalIterator<const CommandLine::Argument> engineConfigPath = commandLineArguments.FindArgument(MAKE_NATIVE_LITERAL("engine"), CommandLine::Prefix::Minus))
		{
			return EngineInfo(IO::Path(engineConfigPath->value.GetView()));
		}
		return FindLocalEngine(commandLineArguments);
	}

	[[nodiscard]] inline EngineInfo
	FindEngineFromAsset([[maybe_unused]] const PluginInfo& plugin, const CommandLine::Arguments& commandLineArguments)
	{
		if (OptionalIterator<const CommandLine::Argument> engineConfigPath = commandLineArguments.FindArgument(MAKE_NATIVE_LITERAL("engine"), CommandLine::Prefix::Minus))
		{
			return EngineInfo(IO::Path(engineConfigPath->value.GetView()));
		}
		return FindLocalEngine(commandLineArguments);
	}

	[[nodiscard]] Asset::Owners inline GetAssetOwners(
		const IO::PathView assetPath, Asset::Context&& context, const CommandLine::Arguments& commandLineArguments
	)
	{
		Asset::Owners assetOwners(assetPath, Forward<Asset::Context>(context));
		if (!assetOwners.m_context.HasEngine())
		{
			assetOwners.m_context.SetEngine(FindLocalEngine(commandLineArguments));
		}
		return assetOwners;
	}

	[[nodiscard]] inline Asset::Context GetInitialAssetContext(const CommandLine::Arguments& commandLineArguments)
	{
		Asset::Context context;

		if (OptionalIterator<const CommandLine::Argument> engineConfigPath = commandLineArguments.FindArgument(MAKE_NATIVE_LITERAL("engine"), CommandLine::Prefix::Minus))
		{
			context.SetEngine(EngineInfo(IO::Path(engineConfigPath->value.GetView())));
		}
		if (OptionalIterator<const CommandLine::Argument> projectConfigPath = commandLineArguments.FindArgument(MAKE_NATIVE_LITERAL("project"), CommandLine::Prefix::Minus))
		{
			context.SetProject(ProjectInfo(IO::Path(projectConfigPath->value.GetView())));
		}
		if (OptionalIterator<const CommandLine::Argument> pluginConfigPath = commandLineArguments.FindArgument(MAKE_NATIVE_LITERAL("plugin"), CommandLine::Prefix::Minus))
		{
			context.SetPlugin(PluginInfo(IO::Path(pluginConfigPath->value.GetView())));
		}

		if (!context.HasEngine())
		{
			context.SetEngine(FindLocalEngine(commandLineArguments));
		}

		return context;
	}
}
