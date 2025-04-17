#pragma once

#include <Common/Threading/Jobs/Job.h>
#include <Common/Threading/Jobs/IntermediateStage.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/TypeTraits/IsBaseOf.h>
#include <Common/TypeTraits/IsSame.h>

namespace ngine::Threading
{
	struct Job;

	struct TRIVIAL_ABI JobBatch
	{
	private:
		template<typename Stage>
		[[nodiscard]] static Optional<Threading::StageBase*> GetStage(Stage& stage)
		{
			if constexpr (TypeTraits::IsSame<TypeTraits::WithoutConst<Stage>, IntermediateStageType>)
			{
				return Threading::CreateIntermediateStage();
			}
			else
			{
				return stage;
			}
		}
	public:
		JobBatch() = default;
		JobBatch(JobBatch&) = default;
		JobBatch(const JobBatch&) = default;
		JobBatch(JobBatch&&) = default;
		JobBatch& operator=(JobBatch&) = default;
		JobBatch& operator=(const JobBatch&) = default;
		JobBatch& operator=(JobBatch&&) = default;

		JobBatch(const Optional<Threading::Job*> pJob)
			: m_startStageType(StageType::Job)
			, m_finishStageType(StageType::Job)
			, m_pStartStage(pJob)
			, m_pFinishedStage(pJob)
		{
		}
		enum class IntermediateStageType : uint8
		{
			IntermediateStage
		};
		inline static constexpr IntermediateStageType IntermediateStage = IntermediateStageType::IntermediateStage;
		template<typename Stage>
		JobBatch(Stage& job)
			: m_startStageType(TypeTraits::IsBaseOf<Threading::Job, Stage> ? StageType::Job : StageType::Intermediate)
			, m_finishStageType(m_startStageType)
			, m_pStartStage(GetStage(job))
			, m_pFinishedStage(GetStage(job))
		{
			if constexpr (TypeTraits::IsSame<TypeTraits::WithoutConst<Stage>, IntermediateStageType>)
			{
				m_pStartStage->AddSubsequentStage(*m_pFinishedStage);
			}
		}
		template<typename StageTypeLeft, typename StageTypeRight>
		JobBatch(StageTypeLeft& startJob, StageTypeRight& finishedStage)
			: m_startStageType(TypeTraits::IsBaseOf<Threading::Job, StageTypeLeft> ? StageType::Job : StageType::Intermediate)
			, m_finishStageType(TypeTraits::IsBaseOf<Threading::Job, StageTypeRight> ? StageType::Job : StageType::Intermediate)
			, m_pStartStage(GetStage(startJob))
			, m_pFinishedStage(GetStage(finishedStage))
		{
			m_pStartStage->AddSubsequentStage(*m_pFinishedStage);
		}
		enum class ManualDependenciesType : uint8
		{
			ManualDependencies
		};
		inline static constexpr ManualDependenciesType ManualDependencies = ManualDependenciesType::ManualDependencies;
		template<typename StageTypeLeft, typename StageTypeRight>
		JobBatch(ManualDependenciesType, StageTypeLeft& startStage, StageTypeRight& finishedStage)
			: m_startStageType(TypeTraits::IsBaseOf<Threading::Job, StageTypeLeft> ? StageType::Job : StageType::Intermediate)
			, m_finishStageType(TypeTraits::IsBaseOf<Threading::Job, StageTypeRight> ? StageType::Job : StageType::Intermediate)
			, m_pStartStage(&startStage)
			, m_pFinishedStage(&finishedStage)
		{
		}

		[[nodiscard]] bool IsValid() const
		{
			Assert(m_pStartStage != nullptr || m_pFinishedStage == nullptr);
			return m_pStartStage != nullptr;
		}
		[[nodiscard]] bool IsInvalid() const
		{
			return !IsValid();
		}
		[[nodiscard]] operator bool() const
		{
			return IsValid();
		}

		void QueueAfterStartStage(Threading::StageBase& stage)
		{
			if (m_pStartStage == nullptr)
			{
				*this = IntermediateStage;
			}
			else if (m_pStartStage == m_pFinishedStage)
			{
				m_finishStageType = StageType::Intermediate;
				m_pFinishedStage = &Threading::CreateIntermediateStage();
				m_pStartStage->AddSubsequentStage(*m_pFinishedStage);
			}

			m_pStartStage->AddSubsequentStage(stage);
			stage.AddSubsequentStage(*m_pFinishedStage);
		}

		void QueueAfterStartStage(JobBatch& otherBatch)
		{
			if (!otherBatch.IsValid())
			{
				return;
			}

			if (!IsValid())
			{
				*this = JobBatch(IntermediateStage);
			}

			m_pStartStage->AddSubsequentStage(otherBatch.GetStartStage());
			otherBatch.GetFinishedStage().AddSubsequentStage(*m_pFinishedStage);
			otherBatch.m_startStageType = StageType::Intermediate;
		}

		void QueueAfterStartStage(JobBatch&& otherBatch)
		{
			QueueAfterStartStage(static_cast<JobBatch&>(otherBatch));
		}

		void QueueAsNewFinishedStage(Threading::StageBase& stage)
		{
			if (IsValid())
			{
				m_pFinishedStage->AddSubsequentStage(stage);
				m_pFinishedStage = &stage;
				m_finishStageType = StageType::Intermediate;
			}
			else
			{
				m_pStartStage = &Threading::CreateIntermediateStage();
				m_startStageType = StageType::Intermediate;
				m_pFinishedStage = &stage;
				m_pStartStage->AddSubsequentStage(stage);
			}
		}

		void QueueAsNewFinishedStage(JobBatch& otherBatch)
		{
			if (!otherBatch.IsValid())
			{
				return;
			}

			if (IsValid())
			{
				m_pFinishedStage->AddSubsequentStage(otherBatch.GetStartStage());
				m_pFinishedStage = &otherBatch.GetFinishedStage();
				m_finishStageType = otherBatch.m_finishStageType;
				otherBatch.m_startStageType = StageType::Intermediate;
			}
			else
			{
				*this = otherBatch;
			}
		}

		[[nodiscard]] Threading::StageBase& GetStartStage() const
		{
			Expect(m_pStartStage != nullptr);
			return *m_pStartStage;
		}

		[[nodiscard]] PURE_STATICS bool HasStartStage() const noexcept
		{
			return m_pStartStage.IsValid();
		}

		[[nodiscard]] Threading::StageBase& GetFinishedStage() const
		{
			Expect(m_pFinishedStage != nullptr);
			return *m_pFinishedStage;
		}

		[[nodiscard]] PURE_STATICS bool HasFinishedStage() const noexcept
		{
			return m_pFinishedStage.IsValid();
		}

		[[nodiscard]] Optional<Threading::Job*> GetStartJob() const
		{
			return Optional<Threading::Job*>(static_cast<Threading::Job*>(m_pStartStage.Get()), m_startStageType == StageType::Job);
		}

		[[nodiscard]] Optional<Threading::Job*> GetFinishJob() const
		{
			return Optional<Threading::Job*>(static_cast<Threading::Job*>(m_pFinishedStage.Get()), m_finishStageType == StageType::Job);
		}
	private:
		enum class StageType : uint8
		{
			None,
			Job,
			Intermediate
		};
		StageType m_startStageType = StageType::None;
		StageType m_finishStageType = StageType::None;

		Optional<Threading::StageBase*> m_pStartStage;
		Optional<Threading::StageBase*> m_pFinishedStage;
	};
}
