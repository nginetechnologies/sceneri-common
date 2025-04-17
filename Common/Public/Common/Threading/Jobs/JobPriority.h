#pragma once

#include <Common/Platform/UnderlyingType.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Math/Round.h>
#include <Common/Math/Range.h>

namespace ngine::Threading
{
	enum class JobPriority : uint8
	{
		FirstUserInteractive,
		EndNetworkSession = FirstUserInteractive,
		SaveOnClose,
		EndFrame,
		StartFrame,
		WindowResizing,
		Present,
		SubmitCallback,
		Submit,
		FinishSubmitCallback,
		Draw,
		OctreeCulling,
		WidgetDrawing,
		UserInputPolling,
		RealtimeNetworking,
		Physics,
		Animation,
		ComponentUpdates,
		LateStageRenderItemChanges,
		LastUserInteractive = LateStageRenderItemChanges,

		FirstUserInitiated,
		UserInterfaceAction = FirstUserInitiated,
		QueuedComponentDestructions,

		AwaitFrameFinish,
		DestroyEmptyOctreeNodes,
		AsyncTimers,
		LastUserInitiated = AsyncTimers,

		FirstUserVisibleBackground,
		CoreRenderStageResources = FirstUserVisibleBackground,

		LoadFont,
		UserInterfaceLoading,
		LoadGraphicsPipeline,
		LoadShader,
		WidgetHierarchyRecalculation,
		InteractivityLogic,
		LoadLogic,
		LoadProject,
		LoadPlugin,
		LoadSceneTemplate,
		LoadScene,
		DeserializeComponent,
		LoadScriptObject,
		LoadIndicator,

		LoadMaterialStageResources,
		CreateRenderTargetSubmission,
		CreateRenderTarget,

		LoadTextureSubmission,
		LoadTextureFirstMip,

		CreateRenderMesh,
		LoadMaterialInstance,

		CreateProceduralMesh,
		LoadMeshData,
		CloneMeshData,

		LoadTextureLastMip = CloneMeshData + 12,

		LoadAudio,
		LoadSkeleton,
		LoadSkeletonJoint,
		LoadAnimation,
		LoadMeshSkin,
		LoadMeshCollider,
		CreateMeshCollider,
		LoadPhysicsMaterial,

		HighPriorityAsyncNetworkingRequests,

		HighPriorityBackendNetworking,
		LowPriorityBackendNetworking,
		LowPriorityAsyncNetworkingRequests,
		AssetCompilation,
		ReloadChangedAsset,
		LastUserVisibleBackground = ReloadChangedAsset,

		FirstBackground,
		SceneChangeDetection = FirstBackground,
		FileChangeDetection,
		Tutorial,

		DeallocateResourcesMin,
		DeallocateResourcesMax = DeallocateResourcesMin + 32,

		LastBackground = DeallocateResourcesMax
	};

	[[nodiscard]] inline JobPriority GetJobPriorityRange(const float ratio, const JobPriority min, const JobPriority max)
	{
		using UnderlyingType = UNDERLYING_TYPE(JobPriority);
		return (JobPriority)Math::Range<UnderlyingType>::MakeStartToEnd((UnderlyingType)min, (UnderlyingType)max).GetValueFromRatio(ratio);
	}

	[[nodiscard]] inline JobPriority GetDeallocationJobPriority(const size deallocationSize)
	{
		constexpr size HighestPriorityMinimumSize = 1024 * 1024;
		constexpr size LowestPriorityMaximumSize = 8;
		constexpr Math::Range<size> referenceRange = Math::Range<size>::MakeStartToEnd(LowestPriorityMaximumSize, HighestPriorityMinimumSize);
		using UnderlyingType = UNDERLYING_TYPE(JobPriority);
		constexpr Math::Range<UnderlyingType> deallocationPriorityRange = Math::Range<UnderlyingType>::MakeStartToEnd(
			(UnderlyingType)JobPriority::DeallocateResourcesMin,
			(UnderlyingType)JobPriority::DeallocateResourcesMax
		);
		return (JobPriority)deallocationPriorityRange.GetValueFromRatio(1.f - referenceRange.GetClampedRatio(deallocationSize));
	}

	[[nodiscard]] inline constexpr JobPriority operator-(const JobPriority left, const JobPriority right)
	{
		return JobPriority((UNDERLYING_TYPE(JobPriority))left - (UNDERLYING_TYPE(JobPriority))right);
	}

	[[nodiscard]] inline constexpr JobPriority operator+(const JobPriority left, const JobPriority right)
	{
		return JobPriority((UNDERLYING_TYPE(JobPriority))left + (UNDERLYING_TYPE(JobPriority))right);
	}

	[[nodiscard]] inline JobPriority operator*(const JobPriority priority, const float scalar)
	{
		return JobPriority((UNDERLYING_TYPE(JobPriority))Math::Round(float(priority) * scalar));
	}
}
