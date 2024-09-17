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

BOOST_AUTO_TEST_SUITE(RunTestTerminalColor)

BOOST_AUTO_TEST_CASE(TestTerminalColor, *boost::unit_test::disabled())
{
    printf("\n");
    printf("\x1B[31mTexting\033[0m\t\t");
    printf("\x1B[32mTexting\033[0m\t\t");
    printf("\x1B[33mTexting\033[0m\t\t");
    printf("\x1B[34mTexting\033[0m\t\t");
    printf("\x1B[35mTexting\033[0m\n");

    printf("\x1B[36mTexting\033[0m\t\t");
    printf("\x1B[36mTexting\033[0m\t\t");
    printf("\x1B[36mTexting\033[0m\t\t");
    printf("\x1B[37mTexting\033[0m\t\t");
    printf("\x1B[93mTexting\033[0m\n");

    printf("\033[3;42;30mTexting\033[0m\t\t");
    printf("\033[3;43;30mTexting\033[0m\t\t");
    printf("\033[3;44;30mTexting\033[0m\t\t");
    printf("\033[3;104;30mTexting\033[0m\t\t");
    printf("\033[3;100;30mTexting\033[0m\n");

    printf("\033[3;41;97mTexting\033[0m\t\t");
    printf("\033[3;47;35mTexting\033[0m\t\t");
    printf("\033[3;47;35mTexting\033[0m\t\t");
    printf("\t\t");
    printf("\n");

    std::cout << "=================================" << std::endl;
    std::cout << UT::GlobalCasters::castTraceTitle("Trace") << std::endl;
    std::cout << UT::GlobalCasters::castLogTitle("Log") << std::endl;
    std::cout << UT::GlobalCasters::castMessageTitle("Message") << std::endl;
    std::cout << UT::GlobalCasters::castWarnTitle("Warn") << std::endl;
    std::cout << UT::GlobalCasters::castErrorTitle("Error") << std::endl;
    std::cout << UT::GlobalCasters::castFatalTitle("Fatal") << std::endl;

    std::cout << UT::GlobalCasters::castConstructorTitle("Constructor Title") << std::endl;
    std::cout << UT::GlobalCasters::castDestructorTitle("Destructor Title") << std::endl;

    std::cout << UT::GlobalCasters::castHighlightCyan("Highlight Cyan") << std::endl;
    std::cout << UT::GlobalCasters::castHighlightYellow("Highlight Yellow") << std::endl;
    std::cout << UT::GlobalCasters::castHighlightRed("Highlight Red") << std::endl;
    std::cout << UT::GlobalCasters::castHighlightGreen("Highlight Green") << std::endl;
}

class TestClass {
    public:
    TestClass(){
        UT::Ut_Logger::Trace(typeid(this), "Create Test object");
    }
};

BOOST_AUTO_TEST_CASE(TestUtLogger /*, *boost::unit_test::disabled()*/)
{
    TestClass c;
    UT::Ut_Logger::Trace(typeid(this), "Trace, no args");
    UT::Ut_Logger::Trace(typeid(this), "Trace, int arg {}", 1);
    UT::Ut_Logger::Trace(typeid(this), "Trace, str arg {}", "str");
    UT::Ut_Logger::Trace(typeid(this), "Trace, str {} arg {}", "str", 1);
    UT::Ut_Logger::Trace(typeid(this), "Trace, {} str {} arg {}", "str", 1, 2.0);

    UT::Ut_Logger::Log(typeid(this), "Log, no args");
    UT::Ut_Logger::Log(typeid(this), "Log, int arg {}", 1);
    UT::Ut_Logger::Log(typeid(this), "Log, str arg {}", "str");
    UT::Ut_Logger::Log(typeid(this), "Log, str {} arg {}", "str", 1);
    UT::Ut_Logger::Log(typeid(this), "Log, {} str {} arg {}", "str", 1, 2.0);

    UT::Ut_Logger::Message(typeid(this), "Message, no args");
    UT::Ut_Logger::Message(typeid(this), "Message, int arg {}", 1);
    UT::Ut_Logger::Message(typeid(this), "Message, str arg {}", "str");
    UT::Ut_Logger::Message(typeid(this), "Message, str {} arg {}", "str", 1);
    UT::Ut_Logger::Message(typeid(this), "Message, {} str {} arg {}", "str", 1, 2.0);

    UT::Ut_Logger::Warn(typeid(this), "Warn, no args");
    UT::Ut_Logger::Warn(typeid(this), "Warn, int arg {}", 1);
    UT::Ut_Logger::Warn(typeid(this), "Warn, str arg {}", "str");
    UT::Ut_Logger::Warn(typeid(this), "Warn, str {} arg {}", "str", 1);
    UT::Ut_Logger::Warn(typeid(this), "Warn, {} str {} arg {}", "str", 1, 2.0);

    UT::Ut_Logger::Error(typeid(this), "Error, no args");
    UT::Ut_Logger::Error(typeid(this), "Error, int arg {}", 1);
    UT::Ut_Logger::Error(typeid(this), "Error, str arg {}", "str");
    UT::Ut_Logger::Error(typeid(this), "Error, str {} arg {}", "str", 1);
    UT::Ut_Logger::Error(typeid(this), "Error, {} str {} arg {}", "str", 1, 2.0);

    //UT::Ut_Logger::Fatal(typeid(this), "Fatal, no args");
}

BOOST_AUTO_TEST_CASE(TestStr2Color /*, *boost::unit_test::disabled()*/)
{
    UT::Ut_RGBColor rgb1;

    rgb1 = UT::Ut_ColorUtils::str2rgb("#FFFFFFFFFFFF");
    assert(rgb1.r == 1.0f);
    assert(rgb1.g == 1.0f);
    assert(rgb1.b == 1.0f);

    rgb1 = UT::Ut_ColorUtils::str2rgb("#7FFF7FFF7FFF");
    constexpr float exp = 32767.0f/65535.0f;
    assert(rgb1.r == exp);
    assert(rgb1.g == exp);
    assert(rgb1.b == exp);

    rgb1 = UT::Ut_ColorUtils::str2rgb("#000F000F000D");
    // assert(rgb1.r == 0.000228885f);
    // assert(std::round(rgb1.g * 10000.0f)/10000.0f == 0.00023f);
    // assert(std::round(rgb1.b * 10000.0f)/10000.0f == 0.0002f);

    rgb1 = UT::Ut_ColorUtils::str2rgb("#000000000000");
    assert(rgb1.r == 0.0f);
    assert(rgb1.g == 0.0f);
    assert(rgb1.b == 0.0f);http://en.cppreference.com/w/cpp/string/byte/toupper
}

BOOST_AUTO_TEST_CASE(TestColor2Str /*, *boost::unit_test::disabled()*/)
{
    std::string rgb1 = UT::Ut_ColorUtils::rgb2str(UT::Ut_RGBColor{ .r=1.0f,.g=1.0f,.b=1.0f });
    assert(rgb1.compare("#FFFFFFFFFFFF") == 0);

    std::string rgb2 = UT::Ut_ColorUtils::rgb2str(UT::Ut_RGBColor{ .r=0.0f,.g=0.0f,.b=0.0f });
    assert(rgb1.compare("#000000000000") == 0);

    std::string rgb3 = UT::Ut_ColorUtils::rgb2str(UT::Ut_RGBColor{ .r=0.5f,.g=0.5f,.b=0.5f });
    assert(rgb1.compare("#800080008000") == 0);

    std::string rgb4 = UT::Ut_ColorUtils::rgb2str(UT::Ut_RGBColor{ .r=0.000228885f,.g=0.00023f,.b=0.0002f });
    assert(rgb1.compare("#000F000F000D") == 0);

    std::cout << UT::Ut_ColorUtils::rgb2str(UT::Ut_RGBColor{ .r=0.000228885f,.g=0.00023f,.b=0.0002f }) << std::endl;
    std::cout << UT::Ut_ColorUtils::rgb2str(UT::Ut_RGBColor{ .r=0.000228885f,.g=0.00023f,.b=0.0002f }) << std::endl;
    std::cout << UT::Ut_ColorUtils::rgb2str(UT::Ut_RGBColor{ .r=0.000228885f,.g=0.00023f,.b=0.0002f }) << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()