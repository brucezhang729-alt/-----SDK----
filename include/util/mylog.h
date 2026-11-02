#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <spdlog/spdlog.h>

namespace Bite
{
    class Logger
    {
        public:
            static void InitLogger(const std::string& LoggerName, const std::string& logFilePath, spdlog::level::level_enum logLevel = spdlog::level::info);
            // 必须先调用 InitLogger；未初始化时 GetLogger 会抛出异常。
            static std::shared_ptr<spdlog::logger> GetLogger();
        private:
            Logger();
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;
        private:
            static std::shared_ptr<spdlog::logger> _logger;
            static std::mutex _mutex;
    };
} // end Bite

// 文件名和行号由 spdlog 的 source_loc 传递，并通过日志格式输出。
#define TRC(...) SPDLOG_LOGGER_CALL(::Bite::Logger::GetLogger(), ::spdlog::level::trace, __VA_ARGS__)
#define DBG(...) SPDLOG_LOGGER_CALL(::Bite::Logger::GetLogger(), ::spdlog::level::debug, __VA_ARGS__)
#define INFO(...) SPDLOG_LOGGER_CALL(::Bite::Logger::GetLogger(), ::spdlog::level::info, __VA_ARGS__)
#define WRN(...) SPDLOG_LOGGER_CALL(::Bite::Logger::GetLogger(), ::spdlog::level::warn, __VA_ARGS__)
#define ERR(...) SPDLOG_LOGGER_CALL(::Bite::Logger::GetLogger(), ::spdlog::level::err, __VA_ARGS__)
#define CRI(...) SPDLOG_LOGGER_CALL(::Bite::Logger::GetLogger(), ::spdlog::level::critical, __VA_ARGS__)
