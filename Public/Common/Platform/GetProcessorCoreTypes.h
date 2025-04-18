#pragma once

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#include <Common/Memory/Containers/FlatVector.h>
#elif PLATFORM_APPLE
#include <sys/sysctl.h>
#include <thread>
#elif PLATFORM_POSIX
#include <Common/IO/Path.h>
#include <Common/IO/File.h>
#else
#include <thread>
#endif

#if PLATFORM_EMSCRIPTEN
#include <emscripten/threading.h>
#include <Common/Math/Min.h>
#endif

namespace ngine::Platform
{
	namespace Internal
	{
		struct CoreInfo
		{
			uint16 m_logicalPerformanceCoreCount{0};
			uint16 m_logicalEfficiencyCoreCount{0};
			bool m_isHyperThreaded{false};
		};

		[[nodiscard]] inline PURE_STATICS CoreInfo GetCoreInfoInternal()
		{
			CoreInfo info;

#if PLATFORM_WINDOWS
			// code adapted from https://github.com/alecazam/kram/blob/f80324870546cf612b0703b1b845c53b783e2371/libkram/kram/TaskSystem.cpp
			DWORD returnLength = 0;
			GetLogicalProcessorInformation(nullptr, &returnLength);

			Assert(returnLength % sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) == 0);

			FlatVector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION, 512> procInfo{
				Memory::ConstructWithSize,
				Memory::Uninitialized,
				(uint16)(returnLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION))
			};

			GetLogicalProcessorInformation(&procInfo[0], &returnLength);

			// walk the array a first time to see if we have hyperthreading
			PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = &procInfo[0];
			DWORD byteOffset = 0;

			while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
			{
				if (ptr->Relationship == RelationProcessorCore)
				{
					uint64 logicalCores = Memory::GetNumberOfSetBits(ptr->ProcessorMask);
					if (logicalCores > 1)
					{
						info.m_isHyperThreaded = true;
						break;
					}
				}

				byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
				ptr++;
			}

			// Walk the array a second time, if we have at least an hyperthreaded core, any physical core with 1 logical core is considered an
			// efficiency core

			ptr = &procInfo[0];
			byteOffset = 0;

			while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
			{
				if (ptr->Relationship == RelationProcessorCore)
				{
					uint64 logicalCores = Memory::GetNumberOfSetBits(ptr->ProcessorMask);
					if (logicalCores > 1 || !info.m_isHyperThreaded)
					{
						info.m_logicalPerformanceCoreCount += (uint16)logicalCores;
					}
					else
					{
						info.m_logicalEfficiencyCoreCount++;
					}
				}

				byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
				ptr++;
			}
#elif PLATFORM_APPLE
			int physicalPerformanceCoreCount;
			size_t size = sizeof(physicalPerformanceCoreCount);
			if (::sysctlbyname("hw.perflevel0.physicalcpu", &physicalPerformanceCoreCount, &size, nullptr, 0) == 0)
			{
				info.m_logicalPerformanceCoreCount = (uint16)physicalPerformanceCoreCount;
			}
			else
			{
				info.m_logicalPerformanceCoreCount = (uint16)std::thread::hardware_concurrency();
			}

			size = sizeof(int);
			int performanceLevelCount = 0;
			if (::sysctlbyname("hw.nperflevels", &performanceLevelCount, &size, nullptr, 0) == 0 && performanceLevelCount > 1)
			{
				int physicalEfficiencyCoreCount;
				if (::sysctlbyname("hw.perflevel1.physicalcpu", &physicalEfficiencyCoreCount, &size, nullptr, 0) == 0)
				{
					info.m_logicalEfficiencyCoreCount = (uint16)physicalEfficiencyCoreCount;
				}
			}
#elif PLATFORM_EMSCRIPTEN
			info.m_logicalPerformanceCoreCount = emscripten_has_threading_support() ? (uint16)Math::Min(emscripten_num_logical_cores(), 6) : 1;
			// Note: No information about efficiency cores
#elif PLATFORM_POSIX
			{
				IO::File cpuInfoFile(IO::Path(MAKE_PATH("/proc/cpuinfo")), IO::AccessModeFlags::Read);
				Assert(cpuInfoFile.IsValid());
				if (LIKELY(cpuInfoFile.IsValid()))
				{
					Array<char, 10240, uint32, uint32> cpuInfoBuffer;
					Array<char, 10240, uint32, uint32> coreInfoBuffer;
					IO::Path::StringType cpuInfoMaxFrequencyFilePath;
					uint16 coreIndex = 0;

					while (cpuInfoFile.ReadLineIntoView(cpuInfoBuffer.GetDynamicView()))
					{
						const ConstStringView coreInfoStringView{cpuInfoBuffer.GetData(), cpuInfoBuffer.GetSize()};
						if (coreInfoStringView.StartsWith("processor"))
						{
							cpuInfoMaxFrequencyFilePath.Format("/sys/devices/system/cpu/cpu{}/cpufreq/cpuinfo_max_freq", coreIndex);
							IO::File coreMaxFrequencyFile(cpuInfoMaxFrequencyFilePath, IO::AccessModeFlags::Read);
							if (coreMaxFrequencyFile.IsValid())
							{
								if (coreMaxFrequencyFile.ReadLineIntoView(coreInfoBuffer.GetDynamicView()))
								{
									const ConstStringView coreMaxFrequencyStringView{
										coreInfoBuffer.GetData(),
										(ConstStringView::SizeType)strlen(coreInfoBuffer.GetData())
									};
									const uint32 frequency = coreMaxFrequencyStringView.ToIntegral<uint32>();
									constexpr uint32 maximumEfficiencyCoreFrequency = 2200000;
									if (frequency > maximumEfficiencyCoreFrequency)
									{
										info.m_logicalPerformanceCoreCount++;
									}
									else
									{
										info.m_logicalEfficiencyCoreCount++;
									}
								}
								else
								{
									info.m_logicalPerformanceCoreCount++;
								}
							}
							else
							{
								info.m_logicalPerformanceCoreCount++;
							}
							coreIndex++;
						}
					}
				}
			}

#if !PLATFORM_LINUX
			{
				IO::File smtInfoFile(IO::Path(MAKE_PATH("/sys/devices/system/cpu/smt/active")), IO::AccessModeFlags::Read);
				if (smtInfoFile.IsValid())
				{
					Array<char, 2, uint32, uint32> buffer;
					if (smtInfoFile.ReadLineIntoView(buffer.GetDynamicView()))
					{
						const ConstStringView smtInfoStringView{buffer.GetData(), (ConstStringView::SizeType)strlen(buffer.GetData())};
						info.m_isHyperThreaded = smtInfoStringView.ToIntegral<uint8>() != 0;

						info.m_logicalPerformanceCoreCount *= static_cast<uint8>(info.m_isHyperThreaded) + 1;
					}
				}
			}
#endif

#else
			info.m_logicalPerformanceCoreCount = (uint16)std::thread::hardware_concurrency();
			// Note: No information about efficiency cores
#endif

			return info;
		}

		[[nodiscard]] inline PURE_STATICS CoreInfo& GetCoreInfo()
		{
			static CoreInfo coreInfo = GetCoreInfoInternal();
			return coreInfo;
		}
	}

	[[maybe_unused]] [[nodiscard]] static uint16 GetPhysicalPerformanceCoreCount()
	{
		const Internal::CoreInfo cpuInfo = Internal::GetCoreInfo();
		return cpuInfo.m_logicalPerformanceCoreCount;
	}

	[[nodiscard]] inline static uint16 GetLogicalPerformanceCoreCount()
	{
		const Internal::CoreInfo cpuInfo = Internal::GetCoreInfo();
		return cpuInfo.m_logicalPerformanceCoreCount;
	}
	[[nodiscard]] inline static uint16 GetLogicalEfficiencyCoreCount()
	{
		const Internal::CoreInfo cpuInfo = Internal::GetCoreInfo();
		return cpuInfo.m_logicalEfficiencyCoreCount;
	}
}
