#include <Core/Graphics/Cuda/Runtime/Sync/Profiler.hpp>

#include <algorithm>
#include <numeric>

#include <spdlog/spdlog.h>

namespace Core::Graphics::Cuda::Runtime
{
    std::expected<Profiler, Core::Utils::Error> Profiler::Create(const std::string& name)
    {
        Profiler profiler(name);
        CORE_TRY(timer, Timer::Create());
        profiler.m_Timer = std::move(timer);
        return profiler;
    }

    std::expected<void, Core::Utils::Error> Profiler::StartProfiling(const Stream& stream)
    {
        m_Sections.clear();
        m_SectionOrder.clear();

        return m_Timer.Start(stream);
    }

    std::expected<void, Core::Utils::Error> Profiler::StopProfiling(const Stream& stream)
    {
        CORE_TRY_DISCARD(m_Timer.Stop(stream));

        return {};
    }

    std::expected<void, Core::Utils::Error> Profiler::CreateSection(const std::string& name)
    {
        if (m_Sections.find(name) != m_Sections.end())
            return std::unexpected(Core::Utils::Error("Profiler section already exists: {}", name));

        CORE_TRY_CONTEXT(sectionTimer, Timer::Create(), "Failed to create timer for profiler section: {}", name);

        m_Sections.emplace(name, SectionData{ {}, std::move(sectionTimer) });
        m_SectionOrder.push_back(name);

        return {};
    }

    std::expected<void, Core::Utils::Error> Profiler::StartSection(const std::string& name, const Stream& stream)
    {
        auto iterator = m_Sections.find(name);

        if (iterator == m_Sections.end())
        {
            CORE_TRY_DISCARD_CONTEXT(CreateSection(name), "Failed to create profiler section: {}", name);
            iterator = m_Sections.find(name);
        }

        return iterator->second.timer.Start(stream);
    }

    std::expected<void, Core::Utils::Error> Profiler::StopSection(const std::string& name, const Stream& stream)
    {
        auto iterator = m_Sections.find(name);

        if (iterator == m_Sections.end())
            return std::unexpected(Core::Utils::Error("Profiler section not found: {}", name));

        auto& section = iterator->second;

        CORE_TRY_DISCARD(section.timer.Stop(stream));

        section.timesMilliseconds.push_back(section.timer.GetElapsedMilliseconds());

        return {};
    }

    float Profiler::GetSectionTotalTimeMilliseconds(const std::string& name) const
    {
        const auto& times = GetSectionTimesMilliseconds(name);

        return std::accumulate(times.begin(), times.end(), 0.0f);
    }

    float Profiler::GetSectionAverageTimeMilliseconds(const std::string& name) const
    {
        const auto& times = GetSectionTimesMilliseconds(name);

        if (times.empty())
            return 0.0f;

        const float totalTimeMilliseconds = std::accumulate(times.begin(), times.end(), 0.0f);

        return totalTimeMilliseconds / static_cast<float>(times.size());
    }

    float Profiler::GetSectionMaxTimeMilliseconds(const std::string& name) const
    {
        const auto& times = GetSectionTimesMilliseconds(name);

        if (times.empty())
            return 0.0f;

        return *std::max_element(times.begin(), times.end());
    }

    float Profiler::GetSectionPercentageOfTotalTime(const std::string& name) const
    {
        const float totalTimeMilliseconds = GetProfiledTimeMilliseconds();

        if (totalTimeMilliseconds <= 0.0f)
            return 0.0f;

        const float sectionTimeMilliseconds = GetSectionTotalTimeMilliseconds(name);

        return sectionTimeMilliseconds / totalTimeMilliseconds * 100.0f;
    }

    Profiler::Result Profiler::GetProfileResult() const
    {
        Result result;
        result.totalTimeMilliseconds = GetProfiledTimeMilliseconds();

        for (const auto& name : m_SectionOrder)
        {
            const auto& sectionData = m_Sections.at(name);

            SectionResult sectionResult;
            sectionResult.name = name;
            sectionResult.runCount = sectionData.timesMilliseconds.size();
            sectionResult.totalTimeMilliseconds = GetSectionTotalTimeMilliseconds(name);
            sectionResult.averageTimeMilliseconds = GetSectionAverageTimeMilliseconds(name);
            sectionResult.maxTimeMilliseconds = GetSectionMaxTimeMilliseconds(name);
            sectionResult.percentageOfTotalTime = GetSectionPercentageOfTotalTime(name);

            result.sections.push_back(sectionResult);
        }

        return result;
    }

    void Profiler::LogResults() const
    {
        const auto result = GetProfileResult();

        spdlog::info("Profile: {}, Total Time: {:.3f} ms", m_Name, result.totalTimeMilliseconds);

        for (const auto& section : result.sections)
        {
            spdlog::info(
                "Section: {}, Runs: {}, Total Time: {:.3f} ms, Average Time: {:.3f} ms, Max Time: {:.3f} ms, Percentage of Total Time: {:.2f}%",
                section.name,
                section.runCount,
                section.totalTimeMilliseconds,
                section.averageTimeMilliseconds,
                section.maxTimeMilliseconds,
                section.percentageOfTotalTime
            );
        }
    }
}