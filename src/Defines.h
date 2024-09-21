#pragma once
// #define PYVK
// #define _DEBUG

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

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define NOMINMAX

#include <vulkan/vulkan.h>
#include <mutex>
#include <iostream>
#include <functional> //for std::hash
#include <string>
#include <memory>
#include <streambuf>
#include <array>
#include <thread>
#include <unordered_map>
#include <limits>

#pragma warning(push)
#pragma warning(disable : 4196)
#include "glm/glm.hpp" // vectors and matrices
#include "glm/gtc/matrix_transform.hpp"
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#pragma warning(pop)

#undef max
#undef min

#ifdef PYVK
	#include <pybind11/pybind11.h>
	#include <pybind11/numpy.h>
	#include <pybind11/stl.h>
	#include <pybind11/functional.h>
	#include <pybind11/embed.h>

	namespace py = pybind11;
#endif

#include <cinttypes>

#define PRECISION 1e-6
#define TYPE_VULKAN 'V'
#define TYPE_SOFTWARE 'S'
#define TYPE_DUMMY 'D'

#include "./lwws_win/lwws_win.hpp"
#include "./utils/Ut_Logger.hpp"
#include "./utils/Ut_Std.hpp"
#include "Vk_Lib.hpp"

namespace VK5 {
	//static VkSampleCountFlagBits offscreen_samples = VK_SAMPLE_COUNT_1_BIT;

	typedef float point_type;
	typedef std::uint32_t index_type;
    typedef int TSteeringGroup;

    struct LayoutPos {
        int W,H;
        float R,Phi;
    };

    struct WH : public LayoutPos {
        WH(int w, int h) : LayoutPos {.W=w,.H=h,.R=0,.Phi=0} {}

        bool const operator==(const WH &other) const {
            return W == other.W && H == other.H;
        }

        bool const operator<(const WH &other) const {
            return W < other.W || (W == other.W && H < other.H);
        }
    };

    struct RPhi : public LayoutPos {
        RPhi(float r, float phi) : LayoutPos {.W=0,.H=0,.R=r,.Phi=phi} {}

        bool const operator==(const RPhi &other) const {
            return R == other.R && Phi == other.Phi;
        }

        bool const operator<(const RPhi &other) const {
            return R < other.R || (R == other.R && Phi < other.Phi);
        }
    };

    enum class Vk_RenderType {
		Rasterizer_IM
	};

    enum class Vk_SteeringType {
		CameraCentric,
		ObjectCentric
	};

    enum class Vk_GpuOp {
        Graphics,
        Compute,
        Transfer
        /**
         * Exsist but unsupported
         */
        // SparseBinding,
        // Protected,
        // VideoDecode,
        // VideoEncode,
        // OpticalFlow
    };

    static std::string Vk_GpuOp2String(Vk_GpuOp type){
            switch(type){
                case Vk_GpuOp::Compute: return "Compute";
                case Vk_GpuOp::Graphics: return "Graphics";
                case Vk_GpuOp::Transfer: return "Transfer";
                default: return "Unknown";
            }
        }

    struct Vk_PinholeState {
        glm::tvec3<point_type> wPos;
        glm::tvec3<point_type> wLook;
        glm::tvec3<point_type> wUp;
        point_type fow;
        point_type wNear;
        point_type wFar;
        glm::mat4 view;
        glm::mat4 perspective;
        glm::tvec3<point_type> xAxis;
        glm::tvec3<point_type> yAxis;
        glm::tvec3<point_type> zAxis;
        float aspect;
    };

    struct Vk_CameraState {
        LWWS::LWWS_Viewport viewport;
        Vk_PinholeState pinhole;
    };

    struct Vk_CameraPinhole {
        glm::tvec3<point_type> wPos;
        glm::tvec3<point_type> wLook;
        glm::tvec3<point_type> wUp;
        point_type fow;
        point_type wNear;
        point_type wFar;
#ifdef PYVK
        Vk_CameraPinhole(
            const py::array_t<VK5::point_type, py::array::c_style>& p_wPos,
            const py::array_t<VK5::point_type, py::array::c_style>& p_wLook,
            const py::array_t<VK5::point_type, py::array::c_style>& p_wUp,
            point_type p_fow,
            point_type p_wNear,
            point_type p_wFar
        )
        : 
        wPos(Vk_NumpyTransformers::arrayToGLMv3(p_wPos)), 
        wLook(Vk_NumpyTransformers::arrayToGLMv3(p_wLook)),
        wUp(Vk_NumpyTransformers::arrayToGLMv3(p_wUp)),
        fow(p_fow), wNear(p_wNear), wFar(p_wFar)
        {}
#endif
    };

    static void Vk_CheckVkResult(const std::type_info& info, VkResult res, const std::string& msg){
        if(res != VK_SUCCESS){
            UT::Ut_Logger::RuntimeError(info, "Failed with error code {0}: " + msg, Vk_Lib::Vk_VkResult2String(res));
        }
    }

    static void Vk_CheckVkResult(const std::type_info& info, bool res, const std::string& msg){
        if(!res){
            UT::Ut_Logger::RuntimeError(info, msg);
        }
    }

    std::mutex queue_submit_mutex;
	std::mutex queue_wait_idle_mutex;
	std::mutex device_wait_idle_mutex;

    class Vk_ThreadSafe {
	public:
		static VkResult Vk_ThreadSafe_QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence){
			VkResult res;
			{
				/*
				TODO: this one really needs a VkFence: vkQueueSubmit must finish before a new one can
				be called 
					=> only possible with a fence.
					=> use mutex to do CPU sided mutual exclusion and a fence for GPU sided mutual exclusion
					=> maybe there is a more secure way... => find some book?
				*/
				std::cout << "wait for queue submit: " << std::this_thread::get_id() << std::endl;
				auto lock = std::lock_guard<std::mutex>(queue_submit_mutex);
				std::cout << "got queue submit: " << std::this_thread::get_id() << std::endl;
				res = vkQueueSubmit(queue, submitCount, pSubmits, fence);
			}
			std::cout << "left queue submit: " << std::this_thread::get_id() << std::endl;
			return res;
		}

		static VkResult Vk_ThreadSafe_QueueWaitIdle(VkQueue queue){
			VkResult res;
			{
				auto lock = std::lock_guard<std::mutex>(queue_wait_idle_mutex);
				res = vkQueueWaitIdle(queue);
			}
			return res;
		}

		static VkResult Vk_ThreadSafe_DeviceWaitIdle(VkDevice device){
			VkResult res;
			{
				auto lock = std::lock_guard<std::mutex>(device_wait_idle_mutex);
				res = vkDeviceWaitIdle(device);
			}
			return res;
		}
	};
}

// std specializations
template <>
struct std::hash<VK5::WH>
{
    std::size_t operator()(const VK5::WH& wh) const
    {
        return ((std::hash<int>()(wh.W) ^ (std::hash<int>()(wh.H) << 1)) >> 1);
    }
};

// std specializations
template <>
struct std::hash<VK5::RPhi>
{
    std::size_t operator()(const VK5::RPhi& rphi) const
    {
        return ((std::hash<float>()(rphi.R) ^ (std::hash<float>()(rphi.Phi) << 1)) >> 1);
    }
};