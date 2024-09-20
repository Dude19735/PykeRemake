#define BOOST_TEST_MODULE RunTestVK5
#include "boost/test/included/unit_test.hpp"

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
   #define VK_USE_PLATFORM_WIN32_KHR
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
#elif __ANDROID__
    // Below __linux__ check should be enough to handle Android,
    // but something may be unique to Android.
#elif __linux__
    // linux
	#define PLATFORM_LINUX
	// #define VK_USE_PLATFORM_XCB_KHR
	#define VK_USE_PLATFORM_XLIB_KHR
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif

// #include "vk5_test_terminal_colors.cpp"
// #include "vk5_test_viewer.cpp"
#include "vk5_test_device.cpp"