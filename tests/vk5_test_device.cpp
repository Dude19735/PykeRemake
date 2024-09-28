#ifndef BOOST_TEST_INCLUDED
    #include "boost/test/included/unit_test.hpp"
#endif

#include <iostream>
#include <typeinfo>
#include <string>
#include <tuple>
#include <format>

#include "../src/utils/Ut_Colors.hpp"
#include "../src/utils/Ut_Logger.hpp"
#include "../src/application/Vk_Device.hpp"

BOOST_AUTO_TEST_SUITE(TestDevice)

auto new_test = boost::unit_test::enabled();
auto all_tests = boost::unit_test::disabled();

void calc(
    const std::vector<VK5::Vk_GpuOp>& priorities,
    std::vector<VK5::Vk_GpuOp>& cur,
    int index,
    /*out*/std::vector<std::vector<VK5::Vk_GpuOp>>& result
){
    if(!cur.empty()) result.push_back(cur);
    for(int i=index; i<priorities.size(); ++i){
        cur.push_back(priorities.at(i));
        calc(priorities, cur, i+1, result);
        cur.pop_back();
    }
}

std::vector<std::vector<VK5::Vk_GpuOp>> subsets(const std::vector<VK5::Vk_GpuOp>& priorities){
    std::vector<std::vector<VK5::Vk_GpuOp>> result;
    std::vector<VK5::Vk_GpuOp> cur;
    calc(priorities, cur, 0, result);
    return result;
}

void vec2stream(const std::vector<VK5::Vk_GpuOp>& prios, std::ostream& stream){
    size_t s = prios.size()-1;
    size_t i=0;
    for(const auto& p : prios){
        stream << VK5::Vk_GpuOp2String(p);
        if(i<s) stream << "|";
        i++;
    }
    stream << std::endl;
}

std::vector<std::vector<VK5::Vk_GpuOp>> allPermutationsAndSubsets(const std::vector<VK5::Vk_GpuOp>& priorities){
    std::vector<std::vector<VK5::Vk_GpuOp>> result;
    auto prioritySubsets = subsets(priorities);
    for(auto ps : prioritySubsets){
        std::sort(ps.begin(), ps.end());
        do {
            result.push_back(ps);
        } while(std::next_permutation(ps.begin(), ps.end()));
    }
    return result;
}

bool assertPrioritiesVec(const std::vector<VK5::Vk_GpuOp>& required, const std::vector<VK5::Vk_GpuOp>& given){
    int requiredInd=0;
    int givenInd=0;
    std::vector<VK5::Vk_GpuOp> res;
    while(givenInd<given.size()){
        if(requiredInd == required.size()) break;
        while(requiredInd<required.size()){
            if(required.at(requiredInd) == given.at(givenInd)){
                res.push_back(required.at(requiredInd));
                break;
            }
            else{
                requiredInd++;
            }
        }
        givenInd++;
    }
    return std::equal(given.begin(), given.end(), res.begin());
}

BOOST_AUTO_TEST_CASE(TestDeviceInit1, *all_tests)
{
    std::ofstream ff("TestDeviceInit1.log", std::ios::out);
    std::vector<VK5::Vk_GpuOp> priorities = {VK5::Vk_GpuOp::Graphics, VK5::Vk_GpuOp::Transfer, VK5::Vk_GpuOp::Compute};
    const auto prioritySubsets = allPermutationsAndSubsets(priorities);
    for(const auto& p : prioritySubsets) {
        vec2stream(p, ff);
        // if(p.size() == 2 && p[0] == VK5::Vk_GpuOp::Graphics && p[1] == VK5::Vk_GpuOp::Compute){
        //     std::cout << "lol" << std::endl;
        // }
        VK5::Vk_Device device("test", VK5::Vk_DevicePreference::USE_ANY_GPU, p);
        device.physicalDevicesToStream(ff);
        device.logicalDeviceQueuesToStream(ff);
        for(const auto& pd : device.PhysicalDevices){
            for(const auto& given : pd.second.physicalDeviceQueues().queueFamilies()){
                bool correct = assertPrioritiesVec(p, given.second.opPriorities);
                if(!correct){
                    std::cout << "!! given:    ";
                    vec2stream(given.second.opPriorities, std::cout);
                    std::cout << "!! required: ";
                    vec2stream(p, std::cout);
                }
                assert(correct == true);
            }
        }
    }
    ff.close();
}

void record(VkCommandBuffer cb, const VK5::Vk_GpuTaskParams& params){

}

void submit(VkCommandBuffer cb, VkQueue queue, const VK5::Vk_GpuTaskParams& params){
    
}

BOOST_AUTO_TEST_CASE(TestDeviceQueuePrio, *all_tests)
{
    {
        std::vector<VK5::Vk_GpuOp> priorities = {VK5::Vk_GpuOp::Compute, VK5::Vk_GpuOp::Graphics, VK5::Vk_GpuOp::Transfer};
        VK5::Vk_Device device("test", VK5::Vk_DevicePreference::USE_ANY_GPU, priorities);
        device.physicalDevicesToStream(std::cout);
        device.logicalDeviceQueuesToStream(std::cout);

        auto iter = std::find_if(device.PhysicalDevices.begin(), device.PhysicalDevices.end(), [](const auto& device){
            return device.second.physicalDevicePR().properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        });
        if(iter != device.PhysicalDevices.end()){
            auto& dev = iter->second;
            // get queue
            std::list<std::unique_ptr<VK5::Vk_Queue>> buffer;
            std::unique_ptr<VK5::Vk_Queue> queue = nullptr;

            auto task = std::make_unique<VK5::Vk_GpuTask>(dev.vk_logicalDevice(), VK5::Vk_GpuOp::Graphics);
            while(true) {
                queue = dev.getQueue(VK5::Vk_GpuOp::Graphics);
                if(queue){
                    std::cout << "===========================================================" << std::endl;
                    std::cout << "Queue for " << VK5::Vk_GpuOp2String(VK5::Vk_GpuOp::Graphics) << ": " << queue->toString() << std::endl;
                    std::cout << "===========================================================" << std::endl;
                    device.logicalDeviceQueuesToStream(std::cout);

                    task = queue->enqueue(std::move(task))->waitResponsively();

                    buffer.emplace_back(std::move(queue));
                }
                else{
                    break;
                }
            }

            std::cout << "===========================================================" << std::endl;
            std::cout << "Empty... => put all back..." << std::endl;
            std::cout << "===========================================================" << std::endl;

            while(!buffer.empty()){
                task = buffer.front()->enqueue(std::move(task))->waitResponsively();
                dev.addQueue(VK5::Vk_GpuOp::Graphics, std::move(buffer.front()));
                buffer.pop_front();
                std::cout << "===========================================================" << std::endl;
                device.logicalDeviceQueuesToStream(std::cout);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(TestDeviceTaskRunner, *new_test)
{
    {
        std::vector<VK5::Vk_GpuOp> priorities = {VK5::Vk_GpuOp::Compute, VK5::Vk_GpuOp::Graphics, VK5::Vk_GpuOp::Transfer};
        VK5::Vk_Device device("test", VK5::Vk_DevicePreference::USE_ANY_GPU, priorities);
        device.physicalDevicesToStream(std::cout);
        device.logicalDeviceQueuesToStream(std::cout);

        auto iter = std::find_if(device.PhysicalDevices.begin(), device.PhysicalDevices.end(), [](const auto& device){
            return device.second.physicalDevicePR().properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        });

        // std::stringstream ssBack;
        // for(int i=0; i<200; ++i) ssBack << "\033[F";
        // std::string back = ssBack.str();
        std::string back = "\x1B[2J\x1B[H";

        if(iter != device.PhysicalDevices.end()){
            auto& dev = iter->second;
            // get queue
            std::list<std::unique_ptr<VK5::Vk_Queue>> buffer;
            std::unique_ptr<VK5::Vk_Queue> queue = nullptr;

            std::list<std::unique_ptr<VK5::Vk_GpuTask>> taskList;
            for(int i=0; i<100; ++i)
                taskList.emplace_back(std::make_unique<VK5::Vk_GpuTask>(dev.vk_logicalDevice(), VK5::Vk_GpuOp::Graphics));

            std::list<VK5::Vk_GpuTask*> running;

            while(!taskList.empty()) {
                running.emplace_back(dev.enqueue(VK5::Vk_GpuOp::Graphics, std::move(taskList.front())));
                taskList.pop_front();
                std::cout << back;
                device.logicalDeviceQueuesToStream(std::cout);
            }
            while(!running.empty()){
                taskList.emplace_back(running.front()->waitResponsively());
                running.pop_front();
            }
            std::cout << "Finished all tasks: " << taskList.size() << std::endl;
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()