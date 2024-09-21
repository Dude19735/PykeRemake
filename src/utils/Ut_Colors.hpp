#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <cmath>
#include <format>
#include <sstream>

#include <cctype>
#include <iomanip>
#include <utility>

namespace UT {
    struct Ut_RGBColor {
		float r, g, b;

        static int innerDimensionLen(){ return 3; }
	};

    struct Ut_OklabColor {
		float L, a, b;
	};

    namespace RGB {
        Ut_RGBColor Red     { 1.0f, 0.0f, 0.0f };
        Ut_RGBColor Green   { 0.0f, 1.0f, 0.0f };
        Ut_RGBColor Olive   { 0.5f, 0.5f, 0.0f };
        Ut_RGBColor Blue    { 0.0f, 0.0f, 1.0f };
        Ut_RGBColor Navy    { 0.0f, 0.0f, 0.5f };
        Ut_RGBColor Yellow  { 1.0f, 1.0f, 0.0f };
        Ut_RGBColor Magenta { 1.0f, 0.0f, 0.0f };
        Ut_RGBColor Cyan    { 0.0f, 1.0f, 1.0f };
        Ut_RGBColor Teal    { 0.0f, 0.5f, 0.5f };
        Ut_RGBColor Black   { 0.0f, 0.0f, 0.0f };
        Ut_RGBColor White   { 1.0f, 1.0f, 1.0f };
        Ut_RGBColor Sliver  { 0.764705882f, 0.764705882f, 0.764705882f };
        Ut_RGBColor Gray    { 0.5f, 0.5f, 0.5f };
        Ut_RGBColor Maroon  { 0.5f, 0.0f, 0.0f };
        Ut_RGBColor Purple  { 0.5f, 0.0f, 0.5f };
    }

    /**
    * Name            FG  BG
    * Black           30  40
    * Red             31  41
    * Green           32  42
    * Yellow          33  43
    * Blue            34  44
    * Magenta         35  45
    * Cyan            36  46
    * White           37  47
    * Bright Black    90  100
    * Bright Red      91  101
    * Bright Green    92  102
    * Bright Yellow   93  103
    * Bright Blue     94  104
    * Bright Magenta  95  105
    * Bright Cyan     96  106
    * Bright White    97  107
    */

    // basic text colors
    class TextColors {
    public:
        static constexpr std::string TE_COL_BLACK =   "\x1B[30m";
        static constexpr std::string TE_COL_RED =     "\x1B[31m";
        static constexpr std::string TE_COL_GREEN =   "\x1B[32m";
        static constexpr std::string TE_COL_YELLOW =  "\x1B[33m";
        static constexpr std::string TE_COL_BLUE =    "\x1B[34m";
        static constexpr std::string TE_COL_MAGENTA = "\x1B[35m";
        static constexpr std::string TE_COL_CYAN =    "\x1B[36m";
        static constexpr std::string TE_COL_WHITE =   "\x1B[37m";

        static constexpr std::string TE_COL_BRIGHT_BLACK =   "\x1B[90m";
        static constexpr std::string TE_COL_BRIGHT_RED =     "\x1B[91m";
        static constexpr std::string TE_COL_BRIGHT_GREEN =   "\x1B[92m";
        static constexpr std::string TE_COL_BRIGHT_YELLOW =  "\x1B[93m";
        static constexpr std::string TE_COL_BRIGHT_BLUE =    "\x1B[94m";
        static constexpr std::string TE_COL_BRIGHT_MAGENTA = "\x1B[95m";
        static constexpr std::string TE_COL_BRIGHT_CYAN =    "\x1B[96m";
        static constexpr std::string TE_COL_BRIGHT_WHITE =   "\x1B[97m";

        // format terminator
        static constexpr std::string TE_COL_END = "\x1B[0m";

        // error headers
        static constexpr std::string TE_COL_TRACE_TITLE =   "\x1B[3;44;97m";
        static constexpr std::string TE_COL_LOG_TITLE =     "\x1B[3;42;97m";
        static constexpr std::string TE_COL_MESSAGE_TITLE = "\x1B[3;107;30m";
        static constexpr std::string TE_COL_WARN_TITLE =    "\x1B[3;43;97m";
        static constexpr std::string TE_COL_ERROR_TITLE =   "\x1B[3;41;97m";
        static constexpr std::string TE_COL_FATAL_TITLE =   "\x1B[3;45;97m";

        // GPU error headers
        static constexpr std::string TE_COL_GPU_TRACE_TITLE =   "\x1B[3;44;94m";
        static constexpr std::string TE_COL_GPU_LOG_TITLE =     "\x1B[3;42;94m";
        static constexpr std::string TE_COL_GPU_MESSAGE_TITLE = "\x1B[3;107;34m";
        static constexpr std::string TE_COL_GPU_WARN_TITLE =    "\x1B[3;43;94m";
        static constexpr std::string TE_COL_GPU_ERROR_TITLE =   "\x1B[3;41;94m";
        static constexpr std::string TE_COL_GPU_FATAL_TITLE =   "\x1B[3;45;94m";

        // text highlighters
        static constexpr std::string TE_COL_HIGHLIGHT_YELLOW = "\x1B[3;43;30m";
        static constexpr std::string TE_COL_HIGHLIGHT_GREEN = "\x1B[3;102;30m";
        static constexpr std::string TE_COL_HIGHLIGHT_CYAN = "\x1B[3;46;30m";
        static constexpr std::string TE_COL_HIGHLIGHT_RED = "\x1B[3;41;97m";
    };

    class GlobalCasters {
    public:
        GlobalCasters() {}
        ~GlobalCasters() {}

        static std::string castConstructorTitle(std::string message) {
            return TextColors::TE_COL_GREEN + std::string("====================[") + message + std::string("]====================\n") + TextColors::TE_COL_END;
        }

        static std::string castVkAttach(std::string message) {
            return TextColors::TE_COL_BRIGHT_CYAN + std::string("====================[") + message + std::string("]====================\n") + TextColors::TE_COL_END;
        }

        static std::string castVkBuild(std::string message) {
            return TextColors::TE_COL_BRIGHT_YELLOW + std::string("====================[") + message + std::string("]====================\n") + TextColors::TE_COL_END;
        }

        static std::string castVkDetach(std::string message) {
            return TextColors::TE_COL_BRIGHT_MAGENTA + std::string("====================[") + message + std::string("]====================\n") + TextColors::TE_COL_END;
        }

        static std::string castDestructorTitle(std::string message) {
            return TextColors::TE_COL_BRIGHT_BLUE + std::string("====================[") + message + std::string("]====================\n") + TextColors::TE_COL_END;
        }

        static std::string castValicationLayer(std::string message) {
            return castHighlightYellow("[Validation layer]") + message;
        }

        static std::string castHighlightYellow(std::string message) { return TextColors::TE_COL_HIGHLIGHT_YELLOW + message + TextColors::TE_COL_END; }
        static std::string castHighlightGreen(std::string message) { return TextColors::TE_COL_HIGHLIGHT_GREEN + message + TextColors::TE_COL_END; }
        static std::string castHighlightCyan(std::string message) { return TextColors::TE_COL_HIGHLIGHT_CYAN + message + TextColors::TE_COL_END; }
        static std::string castHighlightRed(std::string message) { return TextColors::TE_COL_HIGHLIGHT_RED + message + TextColors::TE_COL_END; }

        static std::string castTraceTitle(std::string message) { return TextColors::TE_COL_TRACE_TITLE + message + TextColors::TE_COL_END; }
        static std::string castLogTitle(std::string message) { return TextColors::TE_COL_LOG_TITLE + message + TextColors::TE_COL_END; }
        static std::string castMessageTitle(std::string message) { return TextColors::TE_COL_MESSAGE_TITLE + message + TextColors::TE_COL_END; }
        static std::string castWarnTitle(std::string message) { return TextColors::TE_COL_WARN_TITLE + message + TextColors::TE_COL_END; }
        static std::string castErrorTitle(std::string message) { return TextColors::TE_COL_ERROR_TITLE + message + TextColors::TE_COL_END; }
        static std::string castFatalTitle(std::string message) { return TextColors::TE_COL_FATAL_TITLE + message + TextColors::TE_COL_END; }

        static std::string castBlack(std::string message) { return TextColors::TE_COL_BLACK + message + TextColors::TE_COL_END; }
        static std::string castRed(std::string message) { return TextColors::TE_COL_RED + message + TextColors::TE_COL_END; }
        static std::string castGreen(std::string message) { return TextColors::TE_COL_GREEN + message + TextColors::TE_COL_END; }
        static std::string castYellow(std::string message) { return TextColors::TE_COL_YELLOW + message + TextColors::TE_COL_END; }
        static std::string castBlue(std::string message) { return TextColors::TE_COL_BLUE + message + TextColors::TE_COL_END; }
        static std::string castMagenta(std::string message) { return TextColors::TE_COL_MAGENTA + message + TextColors::TE_COL_END; }
        static std::string castCyan(std::string message) { return TextColors::TE_COL_CYAN + message + TextColors::TE_COL_END; }
        static std::string castWhite(std::string message) { return TextColors::TE_COL_WHITE + message + TextColors::TE_COL_END; }

        static std::string castGpuTraceTitle(std::string message) { return TextColors::TE_COL_GPU_TRACE_TITLE + message + TextColors::TE_COL_END; }
        static std::string castGpuLogTitle(std::string message) { return TextColors::TE_COL_GPU_LOG_TITLE + message + TextColors::TE_COL_END; }
        static std::string castGpuMessageTitle(std::string message) { return TextColors::TE_COL_GPU_MESSAGE_TITLE + message + TextColors::TE_COL_END; }
        static std::string castGpuWarnTitle(std::string message) { return TextColors::TE_COL_GPU_WARN_TITLE + message + TextColors::TE_COL_END; }
        static std::string castGpuErrorTitle(std::string message) { return TextColors::TE_COL_GPU_ERROR_TITLE + message + TextColors::TE_COL_END; }
        static std::string castGpuFatalTitle(std::string message) { return TextColors::TE_COL_GPU_FATAL_TITLE + message + TextColors::TE_COL_END; }
    };

    class Ut_ColorUtils {
        static inline const std::set<char> HexComponents = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    public:
        static Ut_RGBColor str2rgb(const std::string& strColor){
            std::string errmsg = "Hex color must have format '#RRRRGGGGBBBB' where each character is between 0 and F (in hex), but is {0}!";
            if(strColor.size() != 13 || 
               strColor.at(0) != '#' ||
               !std::all_of(strColor.begin()+1, strColor.begin()+5, [&](char x){ return HexComponents.contains(x); }) ||
               !std::all_of(strColor.begin()+5, strColor.begin()+9, [&](char x){ return HexComponents.contains(x); }) ||
               !std::all_of(strColor.begin()+9, strColor.begin()+13, [&](char x){ return HexComponents.contains(x); })
            ){
                throw std::runtime_error(std::vformat(errmsg, std::make_format_args(strColor)));
            }

            unsigned int r, g, b;
            std::stringstream ss;
            ss << std::hex << strColor.substr(1,4);
            ss >> r;
            ss.clear();
            ss << std::hex << strColor.substr(5,4);
            ss >> g;
            ss.clear();
            ss << std::hex << strColor.substr(9,4);
            ss >> b;

            constexpr float cc = 65535.0f;
            return Ut_RGBColor{ .r=static_cast<float>(r)/cc, .g=static_cast<float>(g)/cc, .b=static_cast<float>(b)/cc };
        }

        static std::string rgb2str(const Ut_RGBColor& rgb){
            constexpr float cc = 65535.0f;
            std::stringstream ss;
            ss << '#';
            ss << std::setfill('0') << std::setw(4) << std::hex << std::clamp(static_cast<int>(std::round(rgb.r*cc)), 0, static_cast<int>(cc));
            ss << std::setfill('0') << std::setw(4) << std::hex << std::clamp(static_cast<int>(std::round(rgb.g*cc)), 0, static_cast<int>(cc));
            ss << std::setfill('0') << std::setw(4) << std::hex << std::clamp(static_cast<int>(std::round(rgb.b*cc)), 0, static_cast<int>(cc));
            std::string res = ss.str();

            std::for_each(res.begin(), res.end(), [&](char& c){ c=std::toupper(c); });
            return res;
        }
    };

    class Ut_ColorOp {
        // TODO: synchronize all operations for RGB and RGB colors
    public:
        static Ut_OklabColor rgb_to_oklab(const Ut_RGBColor& rgb)
        {
            float l = 0.4122214708f * rgb.r + 0.5363325363f * rgb.g + 0.0514459929f * rgb.b;
            float m = 0.2119034982f * rgb.r + 0.6806995451f * rgb.g + 0.1073969566f * rgb.b;
            float s = 0.0883024619f * rgb.r + 0.2817188376f * rgb.g + 0.6299787005f * rgb.b;

            float l_ = cbrtf(l);
            float m_ = cbrtf(m);
            float s_ = cbrtf(s);

            Ut_OklabColor res;
            res.L = 0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_;
            res.a = 1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_;
            res.b = 0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_;

            return res;
        }

        // source: https://bottosson.github.io/posts/oklab/
        static Ut_RGBColor oklab_to_rgb(const Ut_OklabColor& oklab)
        {
            float l_ = oklab.L + 0.3963377774f * oklab.a + 0.2158037573f * oklab.b;
            float m_ = oklab.L - 0.1055613458f * oklab.a - 0.0638541728f * oklab.b;
            float s_ = oklab.L - 0.0894841775f * oklab.a - 1.2914855480f * oklab.b;

            float l = l_ * l_ * l_;
            float m = m_ * m_ * m_;
            float s = s_ * s_ * s_;

            Ut_RGBColor res;
            res.r = std::max(std::min(4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s, 1.0f), 0.0f);
            res.g = std::max(std::min(-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s, 1.0f), 0.0f);
            res.b = std::max(std::min(-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s, 1.0f), 0.0f);

            return res;
        }

        static Ut_OklabColor oklab_lerp(float p, Ut_OklabColor& from, Ut_OklabColor& to) {
            Ut_OklabColor res;
            res.L = (1.0f - p) * from.L + p * to.L;
            res.a = (1.0f - p) * from.a + p * to.a;
            res.b = (1.0f - p) * from.b + p * to.b;

            return res;
        }

        static Ut_RGBColor rgb_lerp(float p, const Ut_RGBColor& from, const Ut_RGBColor& to) {
            auto oklab_from = rgb_to_oklab(from);
            auto oklab_to = rgb_to_oklab(to);
            auto oklab_res = oklab_lerp(p, oklab_from, oklab_to);
            auto rgb_res = oklab_to_rgb(oklab_res);

            return rgb_res;
        }

        static std::vector<Ut_RGBColor> rgb_vector_lerp(const std::vector<float>& p, const Ut_RGBColor& from, const Ut_RGBColor& to) {
            std::vector<Ut_RGBColor> res;
            res.reserve(p.size());
            for(auto pp : p){
                if(pp < 0.0f) pp = 0.0f;
                else if(pp > 1.0f) pp = 1.0f;
                res.push_back(rgb_lerp(pp, from, to));
            }

            return res;
        }

        static void rgb_target_vector_lerp(const std::vector<float>& p, const Ut_RGBColor& from, const Ut_RGBColor& to, std::vector<Ut_RGBColor>& target) {
            for(auto pp : p){
                if(pp < 0.0f) pp = 0.0f;
                else if(pp > 1.0f) pp = 1.0f;
                target.push_back(rgb_lerp(pp, from, to));
            }
        }
    };
}