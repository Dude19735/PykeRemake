#pragma once

#include "../../Defines.h"

namespace VK5 {
    struct Vk_GpuTaskParams {
        Vk_GpuOp Op;

        Vk_GpuTaskParams(Vk_GpuOp op) : Op(op) {}
        Vk_GpuTaskParams(const Vk_GpuTaskParams& other) = delete;
        Vk_GpuTaskParams(Vk_GpuTaskParams&& other) : Op(other.Op) {}
        Vk_GpuTaskParams& operator=(const Vk_GpuTaskParams& other) = delete;
        Vk_GpuTaskParams& operator=(Vk_GpuTaskParams&& other) { 
            if(this == &other) return *this;
            Op = std::move(other.Op);
            return *this;
        }
    };
    typedef std::unordered_map<Vk_GpuTargetOp, std::vector<TQueueFamilyIndex>> TGpuTargetOpFamilies;
    typedef void(*TGpuTaskRecord)(VkCommandBuffer, TGpuTargetOpFamilies*, const Vk_GpuTaskParams&);
    typedef void(*TGpuTaskSubmit)(VkCommandBuffer, VkQueue, VkFence, const Vk_GpuTaskParams&);

    class Vk_GpuTaskLib {
    public:
        struct Vk_CopyGpuToGpu : public Vk_GpuTaskParams {
            VkBuffer SrcBuffer;
            VkDeviceSize SrcOffset;
            VkBuffer DstBuffer;
            VkDeviceSize DstOffset;
            VkDeviceSize Size;
            Vk_GpuTargetOp BufferTargetOp;
            Vk_CopyGpuToGpu(
                VkBuffer& srcBuffer,
                VkDeviceSize srcOffset,
                VkBuffer& dstBuffer,
                VkDeviceSize dstOffset,
                VkDeviceSize size,
                Vk_GpuTargetOp bufferTargetOp = Vk_GpuTargetOp::Auto
            ) 
            : 
            Vk_GpuTaskParams(Vk_GpuOp::Transfer),
            SrcBuffer(srcBuffer), SrcOffset(srcOffset),
            DstBuffer(dstBuffer), DstOffset(dstOffset),
            Size(size), BufferTargetOp(bufferTargetOp)
            {}

            Vk_CopyGpuToGpu(const Vk_CopyGpuToGpu& other) = delete;
            Vk_CopyGpuToGpu(Vk_CopyGpuToGpu&& other)
            :
            Vk_GpuTaskParams(std::move(other)),
            SrcBuffer(std::move(other.SrcBuffer)), SrcOffset(std::move(other.SrcOffset)),
            DstBuffer(std::move(other.DstBuffer)), DstOffset(std::move(other.DstOffset)),
            Size(std::move(other.Size)), BufferTargetOp(std::move(other.BufferTargetOp))
            {}

            Vk_CopyGpuToGpu& operator=(const Vk_CopyGpuToGpu& other) = delete;
            Vk_CopyGpuToGpu& operator=(Vk_CopyGpuToGpu&& other){
                Vk_GpuTaskParams::operator=(std::move(other));
                SrcBuffer = std::move(other.SrcBuffer);
                SrcOffset = std::move(other.SrcOffset);
                DstBuffer = std::move(other.DstBuffer);
                DstOffset = std::move(other.DstOffset);
                Size = std::move(other.Size);
                BufferTargetOp = std::move(other.BufferTargetOp);

                return *this;
            }

            /**
             * TODO:Vk_GpuTargetOp - use targetOpFamilies to include the propper VkBufferMemoryBarrier
             *    - 1. transfer target buffer to a Transfer family
             *    - 2. insert copy command
             *    - 3. transfer target buffer to a queue family that fits the target operation
             * 
             *    - the Vk_GpuTargetOp is included in Vk_CopyGpuToGpu
             *    - currently it's set to Vk_GpuTargetOp::Auto default value
             */
            static void record(VkCommandBuffer commandBuffer, const TGpuTargetOpFamilies& targetOpFamilies, const Vk_CopyGpuToGpu& taskParams){
                auto beginInfo = Vk_CI::VkCommandBufferBeginInfo_W(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT).data;
                vkBeginCommandBuffer(commandBuffer, &beginInfo); // begin recording command
                VkBufferCopy copyRegion = {};
                copyRegion.srcOffset = taskParams.SrcOffset; // optional
                copyRegion.dstOffset = taskParams.DstOffset; // optional
                copyRegion.size = taskParams.Size;
                vkCmdCopyBuffer(commandBuffer, taskParams.SrcBuffer, taskParams.DstBuffer, 1, &copyRegion);
                vkEndCommandBuffer(commandBuffer); // end recording command
            }

            static void submit(VkCommandBuffer commandBuffer, VkQueue queue, VkFence fence, const Vk_GpuTaskParams& taskParams){
                // submitInfo is not in Vk_CI because it's rather customized every time it shows up
                VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;
                VkResult res = vkQueueSubmit(queue, 1, &submitInfo, fence);
            }
        };
    };
}