#include "cepch.h"
#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"


namespace CatEngine {

	Ref<spdlog::logger> Log::s_APILogger;
	Ref<spdlog::logger> Log::s_CLILogger;

	void Log::Init() {

        if (s_APILogger != nullptr || s_CLILogger != nullptr)
        {
            CE_API_ASSERT(false, "Logger already Initialized!");
        }

		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_APILogger = spdlog::stdout_color_mt("CEAPI");
		s_APILogger->set_level(spdlog::level::trace);

		s_CLILogger = spdlog::stdout_color_mt("CLI");
		s_CLILogger->set_level(spdlog::level::trace);
	}



}
