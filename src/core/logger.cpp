#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks-inl.h>
#include "logger.hpp"

namespace Core {
  void Logger::initialize() {
    spdlog::set_pattern("%^[%Y-%m-%d %H:%M:%S] [%l] [%n] %v%$");
    coreLogger = spdlog::stdout_color_mt("CORE");
    coreLogger->set_level(spdlog::level::trace);

    gameLogger = spdlog::stdout_color_mt("GAME");
    gameLogger->set_level(spdlog::level::trace);
  }
}
