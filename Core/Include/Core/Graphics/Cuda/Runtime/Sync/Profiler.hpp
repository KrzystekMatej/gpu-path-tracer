#pragma once
#include <expected>
#include <string>
#include <unordered_map>
#include <vector>
#include <Core/Graphics/Cuda/Runtime/Sync/Timer.hpp>

namespace Core::Graphics::Cuda::Runtime
{

    class Profiler
    {
    public:
        struct SectionData
        {
            std::vector<float> timesMilliseconds;
            Timer timer;
        };

        struct SectionResult
        {
            std::string name;
            size_t runCount;
            float totalTimeMilliseconds;
            float averageTimeMilliseconds;
            float maxTimeMilliseconds;
            float percentageOfTotalTime;
        };

        struct Result
        {
            float totalTimeMilliseconds;
            std::vector<SectionResult> sections;
        };

        static std::expected<Profiler, Core::Utils::Error> Create(const std::string& name);

        std::expected<void, Core::Utils::Error> StartProfiling(const Stream& stream = Stream::Default());
        std::expected<void, Core::Utils::Error> StopProfiling(const Stream& stream = Stream::Default());

        std::expected<void, Core::Utils::Error> CreateSection(const std::string& name);
        std::expected<void, Core::Utils::Error> StartSection(const std::string& name, const Stream& stream = Stream::Default());
        std::expected<void, Core::Utils::Error> StopSection(const std::string& name, const Stream& stream = Stream::Default());

        float GetProfiledTimeMilliseconds() const { return m_Timer.GetElapsedMilliseconds(); }
        float GetProfiledTimeSeconds() const { return m_Timer.GetElapsedSeconds(); }
        
        const std::vector<float>& GetSectionTimesMilliseconds(const std::string& name) const { return m_Sections.at(name).timesMilliseconds; }
        size_t GetSectionRunCount(const std::string& name) const { return m_Sections.at(name).timesMilliseconds.size(); }
        
        float GetSectionTotalTimeMilliseconds(const std::string& name) const;
        float GetSectionTotalTimeSeconds(const std::string& name) const { return GetSectionTotalTimeMilliseconds(name) * 0.001f; }

        float GetSectionAverageTimeMilliseconds(const std::string& name) const;
        float GetSectionAverageTimeSeconds(const std::string& name) const { return GetSectionAverageTimeMilliseconds(name) * 0.001f; }

        float GetSectionMaxTimeMilliseconds(const std::string& name) const;
        float GetSectionMaxTimeSeconds(const std::string& name) const { return GetSectionMaxTimeMilliseconds(name) * 0.001f; }

        float GetSectionPercentageOfTotalTime(const std::string& name) const;

        Result GetProfileResult() const;
        void LogResults() const;
    private:
        Profiler(const std::string& name) : m_Name(name) {}

        std::unordered_map<std::string, SectionData> m_Sections;
        Timer m_Timer;
        std::string m_Name;
    };
}

#pragma once

#ifdef CORE_ENABLE_TIMING

#define CUDA_PROFILE_CREATE(name) \
    CORE_TRY(name, Core::Graphics::Cuda::Runtime::Profiler::Create())

#define CUDA_PROFILE_START(profiler, stream) \
    CORE_TRY_DISCARD((profiler).StartProfiling(stream))

#define CUDA_PROFILE_STOP(profiler, stream) \
    CORE_TRY_DISCARD((profiler).StopProfiling(stream))

#define CUDA_PROFILE_LOG(profiler) \
    (profiler).LogResults()

#define CUDA_PROFILE_SECTION(profiler, stream, sectionName, body) \
    do \
    { \
        CORE_TRY_DISCARD((profiler).StartSection((sectionName), (stream))); \
        body \
        CORE_TRY_DISCARD((profiler).StopSection((sectionName), (stream))); \
    } while (false)

#else

#define CUDA_PROFILE_CREATE(name)
#define CUDA_PROFILE_START(profiler, stream)
#define CUDA_PROFILE_STOP(profiler, stream)
#define CUDA_PROFILE_LOG(profiler)

#define CUDA_PROFILE_SECTION(profiler, stream, sectionName, body) \
    do \
    { \
        body \
    } while (false)

#endif