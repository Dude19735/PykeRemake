#pragma once

#include <vector>
#include "Defines.h"

namespace VK5 {
	struct OutOfDeviceMemoryException : public std::exception
	{
		const char* what() const throw ()
		{
			return "GPU Device out of memory";
		}
	};
	
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
			return std::vformat("[{0}] {1}", std::make_format_args(name, message));
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
				default: return "VK_ERROR_UNKNOWN";
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
				default: return "VK_QUEUE_UNKNOWN";
			}
		};

		static std::string Vk_CropVkQueueFlagBitsStr(const std::string& propStr){
            std::vector<int> inds;
            int i=0;
            for(const auto& c : propStr) {
                if(c == '_') inds.push_back(i);
                i++;
            }

			if(propStr.ends_with("_BIT_KHR")) 
				return std::string(propStr.begin()+inds.at(1)+1, propStr.begin()+inds.back()-1) + " (KHR)";
			else if(propStr.ends_with("_BIT_NV")) 
				return std::string(propStr.begin()+inds.at(1)+1, propStr.begin()+inds.back()-1) + " (NV)";
            return std::string(propStr.begin()+inds.at(1)+1, propStr.begin()+inds.back());
        }

		static std::string Vk_VkQueueFlagBitsSet2Str(const std::set<VkQueueFlagBits>& propBits){
            std::stringstream res;
            size_t s = propBits.size();
            int i=0; 
            for(const auto& p : propBits){
                res << Vk_CropVkQueueFlagBitsStr(Vk_VkQueueFlagBits2String(p));
                if(i < s-1) res << " | ";
                i++;
            }
            return res.str();
        }

		static std::string Vk_VkMemoryPropertyFlags2String(const VkMemoryPropertyFlags flags) {
			switch (flags) {
				case 0: return "VK_MEMORY_PROPERTY_HOST_LOCAL_BIT"; // this one is custom, not Vulkan API
				case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: return "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT";
				case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: return "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT";
				case VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: return "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT";
				case VK_MEMORY_PROPERTY_HOST_CACHED_BIT: return "VK_MEMORY_PROPERTY_HOST_CACHED_BIT";
				case VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT: return "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT";
				case VK_MEMORY_PROPERTY_PROTECTED_BIT: return "VK_MEMORY_PROPERTY_PROTECTED_BIT";
				case VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: return "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD";
				case VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: return "VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD";
				case VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV: return "VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV";
				default: return "VK_MEMORY_PROPERTY_UNKNOWN";
			}
		}

		static std::string Vk_CropVkMemoryPropertyFlagsStr(const std::string& flagsStr){
            std::vector<int> inds;
            int i=0;
            for(const auto& c : flagsStr) {
                if(c == '_') inds.push_back(i);
                i++;
            }

			if(flagsStr.ends_with("_BIT_KHR")) 
				return std::string(flagsStr.begin()+inds.at(2)+1, flagsStr.begin()+inds.back()-1) + " (KHR)";
			else if(flagsStr.ends_with("_BIT_NV")) return std::string(flagsStr.begin()+inds.at(2)+1, flagsStr.begin()+inds.back()-1) + " (NV)";
			else if(flagsStr.ends_with("_BIT_AMD")) return std::string(flagsStr.begin()+inds.at(2)+1, flagsStr.begin()+inds.back()-1) + " (AMD)";
            return std::string(flagsStr.begin()+inds.at(2)+1, flagsStr.begin()+inds.back());
        }

		static std::string Vk_VkMemoryPropertyFlagsSet2Str(const std::set<VkMemoryPropertyFlagBits>& propBits){
            std::stringstream res;
            size_t s = propBits.size();
            int i=0; 
            for(const auto& p : propBits){
                res << Vk_CropVkMemoryPropertyFlagsStr(Vk_VkMemoryPropertyFlags2String(p));
                if(i < s-1) res << " | ";
                i++;
            }
            return res.str();
        }
    };
}