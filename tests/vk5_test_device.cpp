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
    std::vector<VK5::Vk_GpuOp> priorities = {VK5::Vk_GpuOp::Graphics, VK5::Vk_GpuOp::Transfer, VK5::Vk_GpuOp::Compute};
    const auto prioritySubsets = allPermutationsAndSubsets(priorities);
    for(const auto& p : prioritySubsets) {
        vec2stream(p, std::cout);
        VK5::Vk_Device device("test", VK5::Vk_DevicePreference::USE_ANY_GPU, p);
        device.tableStream(std::cout);
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

}

BOOST_AUTO_TEST_CASE(TestDeviceQueuePrio, *new_test)
{
    std::vector<VK5::Vk_GpuOp> priorities = {VK5::Vk_GpuOp::Graphics, VK5::Vk_GpuOp::Transfer};
    VK5::Vk_Device device("test", VK5::Vk_DevicePreference::USE_ANY_GPU, priorities);
    device.tableStream(std::cout);
}

BOOST_AUTO_TEST_SUITE_END()