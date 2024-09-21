#pragma once

#include <vector>
#include "Defines.h"

namespace VK5 {
    class Vk_Lib {
    public:
		static std::uint64_t bestPowerOfTwo(std::uint64_t size) {
			std::uint64_t temp = size;
			int counter = 0;
			while (temp > 0) {
				temp = temp >> 1;
				counter++;
			}
			return static_cast<std::uint64_t>(std::pow(2, counter));
		}

		static std::string formatWithObjName(std::string name, std::string message) {
			return std::string("[") + name + std::string("] ") + message;
		}

        static bool replace(std::string& str, const std::string& from, const std::string& to) {
			size_t start_pos = str.find(from);
			if (start_pos == std::string::npos)
				return false;
			str.replace(start_pos, from.length(), to);
			return true;
		}

		static double round(double value, int decimals) {
			double f = decimals * 10.0;
			return std::round(value * f) / f;
		}

		static std::string rightCrop(double value) {
			std::string rVal = std::to_string(value);

			// Remove trailing zeroes
			std::string cVal = rVal.substr(0, rVal.find_last_not_of('0') + 1);
			// If the decimal point is now the last character, remove that as well
			if (cVal.find('.') == cVal.size() - 1)
			{
				cVal = cVal.substr(0, cVal.size() - 1);
			}
			return cVal;
		}

		static std::string Vk_VkResult2String(VkResult res){
			switch(res) {
				case VK_SUCCESS: return "VK_SUCCESS";
				case VK_NOT_READY: return "VK_NOT_READY";
				case VK_TIMEOUT: return "VK_TIMEOUT";
				case VK_EVENT_SET: return "VK_EVENT_SET";
				case VK_EVENT_RESET: return "VK_EVENT_RESET";
				case VK_INCOMPLETE: return "VK_INCOMPLETE";
				case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
				case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
				case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
				case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
				case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
				case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
				case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
				case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
				case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
				case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
				case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
				case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
				case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
				case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
				case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
				case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
				case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
				case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED";
				case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
				case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
				case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
				case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
				case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
				case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
				case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
				case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
				case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
				case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
				case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
				case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
				case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
				case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
				case VK_ERROR_NOT_PERMITTED_KHR: return "VK_ERROR_NOT_PERMITTED_KHR";
				case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
				case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
				case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
				case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
				case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
			#ifdef VK_ENABLE_BETA_EXTENSIONS
				case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
			#endif
				case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
				case VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT: return "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT";
				case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
				default: return "VK_NON_REGISTERED_ERROR_CODE";
			}
		}

		static std::string Vk_VkQueueFlagBits2String(VkQueueFlagBits bits) {
			switch(bits) {
				case VK_QUEUE_GRAPHICS_BIT: return "VK_QUEUE_GRAPHICS_BIT";
				case VK_QUEUE_COMPUTE_BIT: return "VK_QUEUE_COMPUTE_BIT";
				case VK_QUEUE_TRANSFER_BIT: return "VK_QUEUE_TRANSFER_BIT";
				case VK_QUEUE_SPARSE_BINDING_BIT: return "VK_QUEUE_SPARSE_BINDING_BIT";
				case VK_QUEUE_PROTECTED_BIT: return "VK_QUEUE_PROTECTED_BIT";
				case VK_QUEUE_VIDEO_DECODE_BIT_KHR: return "VK_QUEUE_VIDEO_DECODE_BIT_KHR";
				case VK_QUEUE_VIDEO_ENCODE_BIT_KHR: return "VK_QUEUE_VIDEO_ENCODE_BIT_KHR";
				case VK_QUEUE_OPTICAL_FLOW_BIT_NV: return "VK_QUEUE_OPTICAL_FLOW_BIT_NV";
				case VK_QUEUE_FLAG_BITS_MAX_ENUM: return "VK_QUEUE_FLAG_BITS_MAX_ENU";
				default: return "UNKNOWN_VK_QUEUE";
			}
		};
    };
}