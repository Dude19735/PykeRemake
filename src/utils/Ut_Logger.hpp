#pragma once

#include <iostream>
#include <string>
#include <format>
#include <stacktrace>
#include <assert.h>

#include "Ut_Colors.hpp"

#ifdef _DEBUG
// #define DEBUG_TRACE
#define DEBUG_LOG
#endif
#define MESSAGE
#define DEBUG_WARN
#define DEBUG_ERROR 
#define DEBUG_FATAL

class NoneObj {};

// define a simple logger that allowes not to define iostream all over the place
namespace UT {
	class Ut_Logger {

	public:
		// define a trace/log/warn... that can take a message 
		// and a bunch of other parameters
		//static void Trace(std::string message ...);
		template<class ..._Types>
		static void Trace(const std::type_info& info, const std::string& message, const _Types&... _args) {
#ifdef DEBUG_TRACE
			std::string msg = GlobalCasters::castTraceTitle("[TRACE | " + std::string(info.name()) + "]: ");
			std::cout << msg << std::vformat(message, std::make_format_args(_args...)) << std::endl; // std::format(message, _args...) << std::endl;
#endif
		}

		template<class ..._Types>
		static void Log(const std::type_info& info, const std::string& message, const _Types&... _args) {
#ifdef DEBUG_LOG
			std::string msg = GlobalCasters::castLogTitle("[LOG | " + std::string(info.name()) + "]: ");
			std::cout << msg << std::vformat(message, std::make_format_args(_args...)) << std::endl; // std::format(message, _args...) << std::endl;
#endif
		}

		template<class ..._Types>
		static void Message(const std::type_info& info, const std::string& message, const _Types&... _args) {
#ifdef MESSAGE
			std::string msg = GlobalCasters::castMessageTitle("[MESSAGE | " + std::string(info.name()) + "]: ");
			std::cout << msg << std::vformat(message, std::make_format_args(_args...)) << std::endl; // std::format(message, _args...) << std::endl;
#endif
		}

		template<class ..._Types>
		static void Warn(const std::type_info& info, const std::string& message, const _Types&... _args) {
#ifdef DEBUG_WARN
			std::string msg = GlobalCasters::castWarnTitle("[WARN | " + std::string(info.name()) + "]: ");
			std::cout << msg << std::vformat(message, std::make_format_args(_args...)) << std::endl; // std::format(message, _args...) << std::endl;
#endif
		}

		template<class ..._Types>
		static void Error(const std::type_info& info, const std::string& message, const _Types&... _args) {
#ifdef DEBUG_ERROR
			std::string msg = GlobalCasters::castErrorTitle("[ERROR | " + std::string(info.name()) + "]: ");
			std::cout << msg << std::vformat(message, std::make_format_args(_args...)) << std::endl; // std::format(message, _args...) << std::endl;
			//std::cout << "==============================" << std::endl;
			//std::cout << std::stacktrace::current() << std::endl;
			//std::cout << "==============================" << std::endl;
#endif
		}

		template<class ..._Types>
		static void RuntimeError(const std::type_info& info, const std::string& message, const _Types&... _args) {
#ifdef DEBUG_FATAL
			std::string msg = GlobalCasters::castFatalTitle("[FATAL | " + std::string(info.name()) + "]: ");
			std::stringstream s;
			s << msg;
			s << std::vformat(message, std::make_format_args(_args...));
			std::cout << s.str() << std::endl; // std::format(message, _args...) << std::endl;
			//std::cout << "==============================" << std::endl;
			//std::cout << std::stacktrace::current() << std::endl;
			//std::cout << "==============================" << std::endl;
			throw std::runtime_error(s.str());
#endif
		}
	};
}
