#pragma once

#include <string>
#include <sstream>
#include <set>

namespace LWWS {
    enum class WindowAction {
        NoAction,
        SteadyPress,
        Activated,
        Deactivated,
        Resized,
        Maximized,
        Minimized,
        Moved,
        Created,
        Paint,
        Closed
    };

    static std::string WindowAction2String(WindowAction state){
        switch(state){
            case WindowAction::NoAction: return "NoAction";
            case WindowAction::SteadyPress: return "SteadyPress";
            case WindowAction::Activated: return "Activated";
            case WindowAction::Deactivated: return "Deactivated";
            case WindowAction::Resized: return "Resized";
            case WindowAction::Maximized: return "Maximized";
            case WindowAction::Minimized: return "Minimized";
            case WindowAction::Moved: return "Moved";
            case WindowAction::Created: return "Created";
            case WindowAction::Paint: return "Paint";
            case WindowAction::Closed: return "Closed";
        }
        return "WindowAction Unknown";
    }

    enum class ButtonOp {
        NoOp,
        Up,
        Down,
        SteadyPress
    };

    static std::string ButtonOp2String(ButtonOp op){
        switch(op){
            case ButtonOp::NoOp: return "None";
            case ButtonOp::Up: return "Up";
            case ButtonOp::Down: return "Down";
            case ButtonOp::SteadyPress: return "SteadyPress";
        }
        return "MouseOp Unknown";
    }

    enum class MouseButton {
        NoButton,
        Left,
        Middle,
        Right
    };

    static std::string MouseButton2String(MouseButton mouseButton){
        switch(mouseButton){
            case MouseButton::NoButton: return "None";
            case MouseButton::Left: return "Left";
            case MouseButton::Middle: return "Middle";
            case MouseButton::Right: return "Right";
        }
        return "MouseButton Unknown";
    }

    enum class MouseAction {
        NoAction,
        HoverStart,
        HoverStop,
        MouseEnter,
        MouseLeave,
        MouseMove,
        MouseButton,
        MouseScroll
    };

    static std::string MouseAction2String(MouseAction mouseAction){
        switch(mouseAction){
            case MouseAction::NoAction: return "None";
            case MouseAction::HoverStart: return "HoverStart";
            case MouseAction::HoverStop: return "HoverStop";
            case MouseAction::MouseEnter: return "MouseEnter";
            case MouseAction::MouseLeave: return "MouseLeave";
            case MouseAction::MouseMove: return "MouseMove";
            case MouseAction::MouseButton: return "MouseButton";
            case MouseAction::MouseScroll: return "MouseScroll";
        }
        return "MouseAction Unknown";
    }
    
    class LWWS_Key {
    public:
        enum class Special {
            RandomKey,
            Sleep,  
            F1, 
            F2, 
            F3,
            F4,
            F5,
            F6,
            F7,
            F8,
            F9,
            F10,
            F11,
            F12,
            NumLock,        
            LShift,         
            RShift,         
            LControl,       
            RControl,
            BackSpace, // 0x09
            Up,
            Left,
            Down,
            Right,
            Escape,
            Insert,
            Home,
            End,
            PageUp,
            PageDown,
            Delete,
            AltGr,
            Alt,
            Oem_1, // ü (CH), between L/P/0 and Backspace/Enter
            Oem_2, // § (CH), right above Tab
            Oem_3, // ¨ (CH), between L/P/0 and Backspace/Enter
            Oem_4, // ' (CH), between L/P/0 and Backspace/Enter
            Oem_5, // ä (CH), between L/P/0 and Backspace/Enter
            Oem_6, // ^ (CH), between L/P/0 and Backspace/Enter
            Oem_7, // ö (CH), between L/P/0 and Backspace/Enter
            Oem_8, // $ (CH), between L/P/0 and Backspace/Enter
            IntentionalSkip // this is just some placeholder to skip certain stuff
        };

        static std::string SpecialKey2String(Special key){
            switch(key){
            case Special::Sleep: return "Sleep";
            case Special::F1: return "F1";
            case Special::F2: return "F2";
            case Special::F3: return "F3";
            case Special::F4: return "F4";
            case Special::F5: return "F5";
            case Special::F6: return "F6";
            case Special::F7: return "F7";
            case Special::F8: return "F8";
            case Special::F9: return "F9";
            case Special::F10: return "F10";            
            case Special::F11: return "F11";            
            case Special::F12: return "F12";
            case Special::NumLock: return "NumLock";        
            case Special::LShift: return "LShift";         
            case Special::RShift: return "RShift";         
            case Special::LControl: return "LControl";       
            case Special::RControl: return "RControl";
            case Special::Up: return "Up";
            case Special::Left: return "Left";
            case Special::Down: return "Down";
            case Special::Right: return "Right";
            case Special::BackSpace: return "BackSpace";
            case Special::Escape: return "Escape";
            case Special::Insert: return "Insert";
            case Special::Home: return "Home";
            case Special::End: return "End";
            case Special::PageUp: return "PageUp";
            case Special::PageDown: return "PageDown";
            case Special::Delete: return "Delete";
            case Special::AltGr: return "AltGr";
            case Special::Alt: return "Alt";
            case Special::Oem_1: return "Oem_1";
            case Special::Oem_2: return "Oem_2";
            case Special::Oem_3: return "Oem_3";
            case Special::Oem_4: return "Oem_4";
            case Special::Oem_5: return "Oem_5";
            case Special::Oem_6: return "Oem_6";
            case Special::Oem_7: return "Oem_7";
            case Special::Oem_8: return "Oem_8";
            case Special::IntentionalSkip: return "IntentionalSkip";
            case Special::RandomKey: return "LWWS_RandomKey";
            }
            return "LWWS_RandomKey";
        }
        
        virtual Special ToSpecialKey(uint64_t K, uint64_t L) = 0;
        virtual char ToChar(uint64_t c) = 0;

        static int KeyToInt(char c){
            return static_cast<int>(c);
        }

        static int KeyToInt(Special k){
            int t = static_cast<int>(k) + 1000;
            if(t == 1000) return -1;
            return t;
        }

        static std::string IntKey2String(const std::set<int> keys){
            std::stringstream stream;
            stream << "[";
            unsigned int counter = 0;
            for(const auto& k : keys){
                stream << IntKey2String(k);
                if(counter < keys.size()-1) stream << " | ";
                counter++;
            }
            stream << "]";
            return stream.str();
        }

        static std::string IntKey2String(int k){
            if(k < 0) return "None";
            if(k > 1000){
                k -= 1000;
                Special sk = static_cast<Special>(k);
                return SpecialKey2String(sk);
            }
            
            char c = static_cast<char>(k);
            std::string res;
            res.push_back(c);
            return res;
        }
    };
}
