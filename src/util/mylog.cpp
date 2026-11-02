#include"../../include/util/mylog.h"
#include <stdexcept>
#include<spdlog/sinks/basic_file_sink.h>
#include<spdlog/sinks/stdout_color_sinks.h>
#include<spdlog/async.h>
namespace Bite
{
    std::shared_ptr<spdlog::logger> Logger::_logger = nullptr;
    std::mutex Logger::_mutex;

    Logger::Logger() {}

    void Logger::InitLogger(const std::string& LoggerName, const std::string& logFilePath, spdlog::level::level_enum logLevel)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_logger)
        {
            return;
        }

        // 复用已有线程池，避免替换其他异步日志器正在使用的线程池。
        if (!spdlog::thread_pool())
        {
            spdlog::init_thread_pool(32768, 1);
        }

        std::shared_ptr<spdlog::logger> logger;
        if (logFilePath == "stdout")
        {
            logger = spdlog::stdout_color_mt<spdlog::async_factory>(LoggerName);
        }
        else
        {
            logger = spdlog::basic_logger_mt<spdlog::async_factory>(LoggerName, logFilePath);
        }

        logger->flush_on(spdlog::level::info);
        // [时间][日志器名称][日志级别][文件名:行号]日志内容
        logger->set_pattern("[%H:%M:%S][%n][%-7l][%10g:%-4#]%v");
        logger->set_level(logLevel);
        _logger = logger;
    }
    std::shared_ptr<spdlog::logger> Logger::GetLogger()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_logger)
        {
            throw std::logic_error("Logger is not initialized; call Logger::InitLogger() first.");
        }
        return _logger;
    }
} // end Bite

