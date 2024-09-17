#pragma once

// some generic way to distinguish operating systems
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   //define something for Windows (32-bit and 64-bit, this part is common)
   #ifdef _WIN64
      //define something for Windows (64-bit only)
	  #define PLATFORM_WINDOWS_x64
   #else
      //define something for Windows (32-bit only)
	  #define PLATFORM_WINDOWS_x32
   #endif
   #define PLATFORM_WINDOWS
   #include "include/lwws_window_win.hpp"
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
         // iOS, tvOS, or watchOS Simulator
    #elif TARGET_OS_MACCATALYST
         // Mac's Catalyst (ports iOS API into Mac, like UIKit).
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
    #elif TARGET_OS_MAC
        // Other kinds of Apple platforms
		#define PLATFORM_MAC
    #else
    #   error "Unknown Apple platform"
    #endif
    #error "Not Supported"
#elif __ANDROID__
    // Below __linux__ check should be enough to handle Android,
    // but something may be unique to Android.
    #error "Not Supported"
#elif __linux__
    // linux
	#define PLATFORM_LINUX
    #define LWWS_X11
    #ifdef LWWS_X11
        #include "include/lwws_window_x11.hpp"
    #endif
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif

