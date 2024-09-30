#ifndef BOOST_TEST_INCLUDED
    #include "boost/test/included/unit_test.hpp"
#endif

#include <iostream>
#include <typeinfo>
#include <assert.h>
#include <chrono>
#include <format>

#include <math.h>
#define _USE_MATH_DEFINES

#include "test_utilities.hpp"
#include "test_data.hpp"
#include "../src/buffers/Vk_DataBuffer.hpp"

BOOST_AUTO_TEST_SUITE(RunTestVk5DataBuffers)

auto new_test = boost::unit_test::disabled();
auto all_tests = boost::unit_test::enabled();

// template<class T_DataType, class ..._Types>
// void testBuffer1(std::ostream& stream, int trials, VK5::Vk_PhysicalDevice* const device, std::string name, const _Types&... test_config) {

// 	auto args = std::make_tuple(test_config...);
// 	auto data0 = std::get<0>(args); // geometry[0]; 
// 	auto data1 = std::get<1>(args); // geometry[1]; 
// 	auto updateBehaviour = std::get<2>(args); // VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock,  
// 	auto sizeBehaviour = std::get<3>(args); //   VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5;

// 	std::unique_ptr<VK5::Vk_DataBuffer<T_DataType>> buffer
// 		= std::make_unique<VK5::Vk_DataBuffer<T_DataType>>(
// 			device, "TestObj", data0.data(), data0.size(), updateBehaviour, sizeBehaviour, name);

// 	// do unique checks
// 	assert(buffer->bufferCount() <= buffer->maxBufferCount());
// 	assert(buffer->bufferCount() == data0.size());
// 	int64_t duration = 0;
// 	UT::TestUtilities::compareStructuresVectors<T_DataType>(&buffer->getData(), &data0);
// 	duration = buffer->update(data1.data(), data1.size(), 0);

// 	std::vector<int64_t> times;
// 	for(int i=0; i<device->bridge.nFrames(); ++i){
// 		auto t1 = std::chrono::high_resolution_clock::now();
// 		device->bridge.runCurrentFrameUpdates();
// 		device->bridge.incrFrameNr();
// 		auto t2 = std::chrono::high_resolution_clock::now();
// 		times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
// 	}
// 	auto newData = buffer->getData();
// 	UT::TestUtilities::compareStructuresVectors<T_DataType>(&newData, &data1);
// 	std::string timesStr = std::to_string(duration) + ", ";
// 	for(auto& t : times){
// 		timesStr += std::to_string(t) + ", ";
// 	}
// 	stream << std::vformat("{0} \n\t [{1}]us", std::make_format_args(buffer->toString(), timesStr)) << std::endl;
// }

template<class T_DataType>
void resizeTestCases(VK5::Vk_DataBuffer<T_DataType>* buffer, size_t baseCount, std::vector<T_DataType>& data) {
	// these are the initial conditions
	assert(buffer->bufferCount() <= buffer->getRequiredInitCount(baseCount));
	assert(buffer->maxBufferCount() == buffer->getRequiredInitCount(baseCount));
	
	UT::TestUtilities::compareStructuresVectors<T_DataType>(&buffer->getData(), &data);
	
	buffer->resize(static_cast<size_t>(std::ceil(2 * baseCount)));
	// resize changes the maxCount, not the count
	assert(buffer->bufferCount() == baseCount);
	assert(buffer->maxBufferCount() == 2 * baseCount);

	UT::TestUtilities::compareStructuresVectors<T_DataType>(&buffer->getData(), &data);

	buffer->resize(static_cast<size_t>(std::ceil(baseCount)));
	assert(buffer->bufferCount() == baseCount);
	assert(buffer->maxBufferCount() == baseCount);

	UT::TestUtilities::compareStructuresVectors<T_DataType>(&buffer->getData(), &data);
}

template<class T_DataType>
void testUpdateDataBuffer(VK5::Vk_DataBuffer<T_DataType>* buffer, std::vector<T_DataType>& data, size_t keep) {
	// the "keep" is the from argument. update will update the buffer from the "from" position onward.
	size_t newCount = buffer->getNewRequiredMaxCount(data.size());
	buffer->update(data.data(), data.size(), keep);
	assert(buffer->maxBufferCount() == newCount);
	assert(buffer->bufferCount() == data.size());
	UT::TestUtilities::compareStructuresVectors<T_DataType>(&buffer->getData(), &data);
}

template<class T_DataType>
void updateTestCases(VK5::Vk_DataBuffer<T_DataType>* buffer, std::vector<T_DataType>& data1) {
	size_t size1 = data1.size();

	size_t size2 = static_cast<size_t>(std::ceil(1.25 * size1));
	std::vector<T_DataType> tempData2 = UT::VK5TestData::ConstLarge_Data<T_DataType>(size2 - size1, 2.0);
	std::vector<T_DataType> data2 = std::vector<T_DataType>(data1);
	data2.insert(data2.end(), tempData2.begin(), tempData2.end());
	testUpdateDataBuffer<T_DataType>(buffer, data2, size1);

	size_t size3 = static_cast<size_t>(std::ceil(1.5 * size1));
	std::vector<T_DataType> tempData3 = UT::VK5TestData::ConstLarge_Data<T_DataType>(size3 - size2, 3.0);
	std::vector<T_DataType> data3 = std::vector<T_DataType>(data2);
	data3.insert(data3.end(), tempData3.begin(), tempData3.end());
	testUpdateDataBuffer<T_DataType>(buffer, data3, size2);

	size_t size4 = static_cast<size_t>(std::ceil(1.75 * size1));
	std::vector<T_DataType> tempData4 = UT::VK5TestData::ConstLarge_Data<T_DataType>(size4 - size3, 4.0);
	std::vector<T_DataType> data4 = std::vector<T_DataType>(data3);
	data4.insert(data4.end(), tempData4.begin(), tempData4.end());
	testUpdateDataBuffer<T_DataType>(buffer, data4, size3);

	size_t size5 = static_cast<size_t>(std::ceil(2.0 * size1));
	std::vector<T_DataType> tempData5 = UT::VK5TestData::ConstLarge_Data<T_DataType>(size5 - size4, 5.0);
	std::vector<T_DataType> data5 = std::vector<T_DataType>(data4);
	data5.insert(data5.end(), tempData5.begin(), tempData5.end());
	testUpdateDataBuffer<T_DataType>(buffer, data5, size4);

	size_t size6 = static_cast<size_t>(std::ceil(2.25 * size1));
	std::vector<T_DataType> tempData6 = UT::VK5TestData::ConstLarge_Data<T_DataType>(size6 - size5, 6.0);
	std::vector<T_DataType> data6 = std::vector<T_DataType>(data5);
	data6.insert(data6.end(), tempData6.begin(), tempData6.end());
	testUpdateDataBuffer<T_DataType>(buffer, data6, size5);

	size_t size7 = static_cast<size_t>(std::ceil(3.1 * size1));
	std::vector<T_DataType> tempData7 = UT::VK5TestData::ConstLarge_Data<T_DataType>(size7 - size6, 7.0);
	std::vector<T_DataType> data7 = std::vector<T_DataType>(data6);
	data7.insert(data7.end(), tempData7.begin(), tempData7.end());
	testUpdateDataBuffer<T_DataType>(buffer, data7, size6);

	size_t size8 = static_cast<size_t>(std::ceil(5.5 * size1));
	std::vector<T_DataType> tempData8 = UT::VK5TestData::ConstLarge_Data<T_DataType>(size8 - size7, 8.0);
	std::vector<T_DataType> data8 = std::vector<T_DataType>(data7);
	data8.insert(data8.end(), tempData8.begin(), tempData8.end());
	testUpdateDataBuffer<T_DataType>(buffer, data8, size7);
}

// BOOST_AUTO_TEST_CASE(Test_BufferResize2, *all_tests) {
// 	std::string name = "test_app";
// 	VK5::Vk_Device device(name);

// 	size_t testCount = static_cast<size_t>(std::ceil(std::pow(2, 10)));
// 	{
// 		// Vertices
// 		typedef VK5::Vk_Vertex_PC Type;
// 		auto data = UT::VK5TestData::ConstLarge_Data<Type>(testCount, 1.0);
// 		VK5::Vk_DataBuffer<Type> buffer(device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5, "TestBuffer");
// 		resizeTestCases<Type>(buffer.get(), testCount, data);
// 		updateTestCases<Type>(buffer.get(), data);
// 	}
// 	{
// 		// Vertices
// 		typedef VK5::Vk_Vertex_PC Type;
// 		auto data = UT::VK5TestData::ConstLarge_Data<Type>(testCount, 1.0);
// 		std::unique_ptr<VK5::Vk_DataBuffer<Type>> buffer = std::make_unique<VK5::Vk_DataBuffer<Type>>(
// 			device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_Empty_Grow_2, "TestBuffer");
// 		resizeTestCases<Type>(buffer.get(), testCount, data);
// 		updateTestCases<Type>(buffer.get(), data);
// 	}
// 	{
// 		// Vertices
// 		typedef VK5::Vk_Vertex_PC Type;
// 		auto data = UT::VK5TestData::ConstLarge_Data<Type>(testCount, 1.0);
// 		std::unique_ptr<VK5::Vk_DataBuffer<Type>> buffer = std::make_unique<VK5::Vk_DataBuffer<Type>>(
// 			device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");
// 		resizeTestCases<Type>(buffer.get(), testCount, data);
// 		updateTestCases<Type>(buffer.get(), data);
// 	}
// 	{
// 		// Vertices
// 		typedef VK5::Vk_Vertex_PCN Type;
// 		auto data = UT::VK5TestData::ConstLarge_Data<Type>(testCount, 1.0);
// 		std::unique_ptr<VK5::Vk_DataBuffer<Type>> buffer = std::make_unique<VK5::Vk_DataBuffer<Type>>(
// 			device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_2, "TestBuffer");

// 		resizeTestCases<Type>(buffer.get(), testCount, data);
// 		updateTestCases<Type>(buffer.get(), data);
// 	}
// 	{
// 		// Vertices
// 		typedef VK5::Vk_Vertex_PCNT Type;
// 		auto data = UT::VK5TestData::ConstLarge_Data<Type>(testCount, 1.0);
// 		std::unique_ptr<VK5::Vk_DataBuffer<Type>> buffer = std::make_unique<VK5::Vk_DataBuffer<Type>>(
// 			device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5, "TestBuffer");
	
// 		resizeTestCases<Type>(buffer.get(), testCount, data);
// 		updateTestCases<Type>(buffer.get(), data);
// 	}
// 	{
// 		// Vertices
// 		typedef VK5::index_type Type;
// 		auto data = UT::VK5TestData::ConstLarge_Data<Type>(testCount, 1.0);
// 		std::unique_ptr<VK5::Vk_DataBuffer<Type>> buffer = std::make_unique<VK5::Vk_DataBuffer<Type>>(
// 			device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_1_5_Grow_2, "TestBuffer");
	
// 		resizeTestCases<Type>(buffer.get(), testCount, data);
// 		updateTestCases<Type>(buffer.get(), data);
// 	}
// }
/*
BOOST_AUTO_TEST_CASE(Test_BufferResize, *all_tests) {
	std::string name = "test_app";

	std::unique_ptr<VK5::Vk_Instance> instance = std::make_unique<VK5::Vk_Instance>(name);
	std::unique_ptr<VK5::Vk_PhysicalDevice> device = std::make_unique<VK5::Vk_PhysicalDevice>(name);

	{
		// Vertices
		auto data = UT::VK5TestData::Cube1_PC();
		typedef VK5::Vk_Vertex_PC Type;

		std::unique_ptr<VK5::Vk_DataBuffer<Type>> buffer = std::make_unique<VK5::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");

		assert(buffer->cpuDataBufferCount() == 0);
		const std::vector<Type> wbData = buffer->getData();
		assert(wbData.size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData.at(i), data.at(i))); }

		device->printActiveDeviceMemoryProperties();

		double maxSize = static_cast<double>(buffer->localBufferMemoryBudget());
		double dataSize = static_cast<double>(data.size() * sizeof(Type));
		size_t newSize1 = static_cast<size_t>(std::ceil(maxSize / 2) / sizeof(Type));
		buffer->resize(newSize1);	

		const std::vector<Type> wbData2 = buffer->getData();
		assert(wbData2.size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData2.at(i), data.at(i))); }

		device->printActiveDeviceMemoryProperties();

		// this one should yield a buffer larger than half of the device local space
		// so, it should trigger the exception that will read back the data to main memory
		size_t newSize2 = static_cast<size_t>(std::ceil(0.75 * maxSize) / sizeof(Type));
		buffer->resize(static_cast<size_t>(std::ceil(newSize2)));

		const std::vector<Type> wbData3 = buffer->getData();
		assert(wbData3.size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData3.at(i), data.at(i))); }
		
		device->printActiveDeviceMemoryProperties();

		buffer->clearCpuBuffer();
		assert(buffer->cpuDataBufferCount() == 0);
	}
}

BOOST_AUTO_TEST_CASE(Test_BuffersWriteback, *all_tests) {
	std::string name = "test_app";

	std::unique_ptr<VK5::Vk_Instance> instance = std::make_unique<VK5::Vk_Instance>(name);
	std::unique_ptr<VK5::Vk_Surface> surface = std::make_unique<VK5::Vk_Surface>(instance.get(), name, 500, 300, true);
	std::unique_ptr<VK5::Vk_PhysicalDevice> device = std::make_unique<VK5::Vk_PhysicalDevice>(name);

	{
		// Vertices
		auto data = UT::VK5TestData::Cube1_PC();
		typedef VK5::Vk_Vertex_PC Type;

		std::unique_ptr<VK5::Vk_DataBuffer<Type>> buffer = std::make_unique<VK5::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");

		assert(buffer->cpuDataBufferCount() == 0);
		const std::vector<Type> wbData = buffer->getData();

		assert(wbData.size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData.at(i), data.at(i))); }

		buffer->clearCpuBuffer();
		assert(buffer->cpuDataBufferCount() == 0);
	}
	
	{
		// Colors
		auto data = UT::VK5TestData::Cube1_PCN();
		typedef VK5::Vk_Vertex_PCN Type;

		std::unique_ptr<VK5::Vk_DataBuffer<Type>> buffer = std::make_unique<VK5::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");

		assert(buffer->cpuDataBufferCount() == 0);
		const std::vector<Type> wbData = buffer->getData();

		assert(wbData.size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData.at(i), data.at(i))); }

		buffer->clearCpuBuffer();
		assert(buffer->cpuDataBufferCount() == 0);
	}

	{
		// Normals
		auto data = UT::VK5TestData::Cube1_PCNT();
		typedef VK5::Vk_Vertex_PCNT Type;

		std::unique_ptr<VK5::Vk_DataBuffer<Type>> buffer = std::make_unique<VK5::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");

		assert(buffer->cpuDataBufferCount() == 0);
		const std::vector<Type> wbData = buffer->getData();

		assert(wbData.size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(Type::compare(wbData.at(i), data.at(i))); }

		buffer->clearCpuBuffer();
		assert(buffer->cpuDataBufferCount() == 0);
	}

	{
		// Indices
		auto data = UT::VK5TestData::Cube1_PCNT_Indices();
		typedef VK5::index_type Type;

		std::unique_ptr<VK5::Vk_DataBuffer<Type>> buffer = std::make_unique<VK5::Vk_DataBuffer<Type>>(
			device.get(), "TestObj", data.data(), data.size(), VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5, "TestBuffer");

		assert(buffer->cpuDataBufferCount() == 0);
		const std::vector<Type> wbData = buffer->getData();

		assert(wbData.size() == data.size());
		for (int i = 0; i < data.size(); ++i) { assert(wbData.at(i) == data.at(i)); }

		buffer->clearCpuBuffer();
		assert(buffer->cpuDataBufferCount() == 0);
	}
}

template<class T_DataType>
void testBuffer1Batch(std::ostream& outStream, int trials, VK5::Vk_PhysicalDevice* const device, std::string name, VK5::Vk_BufferUpdateBehaviour updateBehaviour){
	size_t size = 25000;
	auto pc1 = UT::VK5TestData::ConstLarge_Data<VK5::Vk_Vertex_PC>(size, 2.3);
	auto pcn1 = UT::VK5TestData::ConstLarge_Data<VK5::Vk_Vertex_PCN>(size, 2.3);
	auto pcnt1 = UT::VK5TestData::ConstLarge_Data<VK5::Vk_Vertex_PCNT>(size, 2.3);
	auto indices1 = UT::VK5TestData::ConstLarge_Data<VK5::index_type>(size, 2.3);

	auto pc2 = UT::VK5TestData::ConstLarge_Data<VK5::Vk_Vertex_PC>(size, 8.2);
	auto pcn2 = UT::VK5TestData::ConstLarge_Data<VK5::Vk_Vertex_PCN>(size, 8.2);
	auto pcnt2 = UT::VK5TestData::ConstLarge_Data<VK5::Vk_Vertex_PCNT>(size, 8.2);
	auto indices2 = UT::VK5TestData::ConstLarge_Data<VK5::index_type>(size, 8.2);

	testBuffer1<VK5::Vk_Vertex_PC>(outStream, trials, device, 		name, pc1, pc2,				updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5);
	testBuffer1<VK5::Vk_Vertex_PCN>(outStream, trials, device, 	name, pcn1, pcn2, 			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5);
	testBuffer1<VK5::Vk_Vertex_PCNT>(outStream, trials, device, 	name, pcnt1, pcnt2,			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5);
	testBuffer1<VK5::index_type>(outStream, trials, device, 		name, indices1, indices2,	updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5);
	outStream << "-------------------------------" << std::endl;
	testBuffer1<VK5::Vk_Vertex_PC>(outStream, trials, device, 		name, pc1, pc2,				updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
	testBuffer1<VK5::Vk_Vertex_PCN>(outStream, trials, device, 	name, pcn1, pcn2, 			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
	testBuffer1<VK5::Vk_Vertex_PCNT>(outStream, trials, device, 	name, pcnt1, pcnt2,			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
	testBuffer1<VK5::index_type>(outStream, trials, device, 		name, indices1, indices2,	updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5);
	outStream << "-------------------------------" << std::endl;
	testBuffer1<VK5::Vk_Vertex_PC>(outStream, trials, device, 		name, pc1, pc2,				updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5);
	testBuffer1<VK5::Vk_Vertex_PCN>(outStream, trials, device, 	name, pcn1, pcn2,			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5);
	testBuffer1<VK5::Vk_Vertex_PCNT>(outStream, trials, device, 	name, pcnt1, pcnt2,			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5);
	testBuffer1<VK5::index_type>(outStream, trials, device, 		name, indices1, indices2,	updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5);
	outStream << "-------------------------------" << std::endl;
	testBuffer1<VK5::Vk_Vertex_PC>(outStream, trials, device, 		name, pc1, pc2,				updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_Empty_Grow_2);
	testBuffer1<VK5::Vk_Vertex_PCN>(outStream, trials, device, 	name, pcn1, pcn2, 			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_Empty_Grow_2);
	testBuffer1<VK5::Vk_Vertex_PCNT>(outStream, trials, device, 	name, pcnt1, pcnt2,			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_Empty_Grow_2);
	testBuffer1<VK5::index_type>(outStream, trials, device, 		name, indices1, indices2,	updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_Empty_Grow_2);
	outStream << "-------------------------------" << std::endl;
	testBuffer1<VK5::Vk_Vertex_PC>(outStream, trials, device, 		name, pc1, pc2,				updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_2);
	testBuffer1<VK5::Vk_Vertex_PCN>(outStream, trials, device, 	name, pcn1, pcn2, 			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_2);
	testBuffer1<VK5::Vk_Vertex_PCNT>(outStream, trials, device, 	name, pcnt1, pcnt2,			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_2);
	testBuffer1<VK5::index_type>(outStream, trials, device, 		name, indices1, indices2,	updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_0_Grow_2);
	outStream << "-------------------------------" << std::endl;
	testBuffer1<VK5::Vk_Vertex_PC>(outStream, trials, device, 		name, pc1, pc2,				updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_5_Grow_2);
	testBuffer1<VK5::Vk_Vertex_PCN>(outStream, trials, device, 	name, pcn1, pcn2,			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_5_Grow_2);
	testBuffer1<VK5::Vk_Vertex_PCNT>(outStream, trials, device, 	name, pcnt1, pcnt2,			updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_5_Grow_2);
	testBuffer1<VK5::index_type>(outStream, trials, device, 		name, indices1, indices2,	updateBehaviour, VK5::Vk_BufferSizeBehaviour::Init_1_5_Grow_2);
	outStream << "==============================================================================================================" << std::endl;
}

BOOST_AUTO_TEST_CASE(Test_Buffers, *all_tests) {
	std::string name = "test_app";

	std::unique_ptr<VK5::Vk_Instance> instance = std::make_unique<VK5::Vk_Instance>(name);
	std::unique_ptr<VK5::Vk_PhysicalDevice> device = std::make_unique<VK5::Vk_PhysicalDevice>(name);

	int trials = 100;
	auto outStream = std::fstream("testBuffer1Batch_output.txt", std::ios::out);
	testBuffer1Batch<VK5::Vk_Vertex_PC>(outStream, trials, device.get(), "Testbuffer", VK5::Vk_BufferUpdateBehaviour::Staged_GlobalLock);
	testBuffer1Batch<VK5::Vk_Vertex_PC>(outStream, trials, device.get(), "Testbuffer", VK5::Vk_BufferUpdateBehaviour::Staged_DoubleBuffering);
	// testBuffer1Batch<VK5::Vk_Vertex_PC>(outStream, trials, device.get(), "Testbuffer", VK5::Vk_BufferUpdateBehaviour::Staged_LazyDoubleBuffering);
	testBuffer1Batch<VK5::Vk_Vertex_PC>(outStream, trials, device.get(), "Testbuffer", VK5::Vk_BufferUpdateBehaviour::Direct_GlobalLock);
	// testBuffer1Batch<VK5::Vk_Vertex_PC>(outStream, trials, device.get(), "Testbuffer", VK5::Vk_BufferUpdateBehaviour::Direct_DoubleBuffering);
	outStream.close();
}
*/
BOOST_AUTO_TEST_SUITE_END()