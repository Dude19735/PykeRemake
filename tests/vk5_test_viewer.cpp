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
#include "../src/cameras/Vk_LayoutGrid.hpp"
#include "../src/cameras/Vk_Camera.hpp"
#include "../src/Vk_Viewer.hpp"

BOOST_AUTO_TEST_SUITE(TestViewer)

auto new_test = boost::unit_test::enabled();
auto all_tests = boost::unit_test::disabled();

BOOST_AUTO_TEST_CASE(TestCameraInit1, *new_test)
{
    VK5::Vk_CameraPinhole pinhole {
        .wPos = glm::tvec3<VK5::point_type> {5.0f, 5.0f, 5.0f },
        .wLook = glm::tvec3<VK5::point_type> {0.0f, 0.0f, 0.0f },
        .wUp = glm::tvec3<VK5::point_type> {0.0f, 0.0f, 1.0f },
        .fow = 80.0f / 180.0f * glm::pi<float>(),
        .wNear = 1.0f,
        .wFar = 100.0f
    };
    VK5::Vk_CameraMisc misc {
        .SteeringGroup=0,
        .RenderType=VK5::Vk_RenderType::Rasterizer_IM,
        .SteeringType=VK5::Vk_SteeringType::OBJECT_CENTRIC
    };
    VK5::Vk_CameraInit init{
        .Misc=misc,
        .Viewport=LWWS::LWWS_Viewport(0, 0, 0, 640, 480),
        .Pinhole=pinhole
    };
    VK5::Vk_Camera camera(init);
}

BOOST_AUTO_TEST_CASE(TestCameraGridLayoutGen, *new_test)
{
    VK5::Vk_CameraPinhole p {
        .wPos = glm::tvec3<VK5::point_type> {5.0f, 5.0f, 5.0f },
        .wLook = glm::tvec3<VK5::point_type> {0.0f, 0.0f, 0.0f },
        .wUp = glm::tvec3<VK5::point_type> {0.0f, 0.0f, 1.0f },
        .fow = 80.0f / 180.0f * glm::pi<float>(),
        .wNear = 1.0f,
        .wFar = 100.0f
    };
    VK5::Vk_CameraMisc m {
        .SteeringGroup=0,
        .RenderType=VK5::Vk_RenderType::Rasterizer_IM,
        .SteeringType=VK5::Vk_SteeringType::OBJECT_CENTRIC
    };
    VK5::Vk_LayoutGrid grid(2,2, 0, 0);
    grid.add(0,0,p,m)                                          .add(0,1,p,VK5::mRT(m,VK5::Vk_RenderType::Rasterizer_IM))
        .add(1,0,p,mST(m,VK5::Vk_SteeringType::CAMERA_CENTRIC)).add(1,1,p,VK5::mSG(m,2));

    auto layout = grid.layout(800, 640);
}

BOOST_AUTO_TEST_CASE(TestViewerFromLayout, *new_test)
{
    VK5::Vk_CameraPinhole p {
        .wPos = glm::tvec3<VK5::point_type> {5.0f, 5.0f, 5.0f },
        .wLook = glm::tvec3<VK5::point_type> {0.0f, 0.0f, 0.0f },
        .wUp = glm::tvec3<VK5::point_type> {0.0f, 0.0f, 1.0f },
        .fow = 80.0f / 180.0f * glm::pi<float>(),
        .wNear = 1.0f,
        .wFar = 100.0f
    };
    VK5::Vk_CameraMisc m {
        .SteeringGroup=0,
        .RenderType=VK5::Vk_RenderType::Rasterizer_IM,
        .SteeringType=VK5::Vk_SteeringType::OBJECT_CENTRIC
    };
    VK5::Vk_LayoutGrid grid(2,2, 0, 0);
    grid.add(0,0,p,m, LWWS::LWWS_ViewportMisc(10, UT::RGB::Navy, UT::RGB::Cyan))
        .add(0,1,p,m)
        .add(1,0,p,m)
        .add(1,1,p,m);

    VK5::Vk_Viewer viewer("test", 800, 640);
    viewer.addLayout(grid);
    viewer.runAsync();
    int counter = 0;
    while(viewer.isRunning()){
        viewer.sleepResponsively(std::chrono::milliseconds(1000));
        std::cout << ++counter << std::endl;
        if(counter == 5) viewer.close();
    }
}

BOOST_AUTO_TEST_SUITE_END()