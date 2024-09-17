#pragma once

#include "lwws_key.hpp"

namespace LWWS {
    class LWWS_Key_X11: public LWWS_Key {
    public:
        static void LWWS_ShowCursor(bool show){

        }

        static bool KeyFilter(unsigned int key){
            switch(key){
                case 66: return true;
                default: return false;
            }
        }

        static LWWS_Key::Special ToSpecialKey(int key) {
            switch(key){
                case 9: return LWWS_Key::Special::Escape;
                // filter this one: case 66: CAPS => NOT SUPPORTED because Windows is incapable of that too
                case 50: return LWWS_Key::Special::LShift;
                case 62: return LWWS_Key::Special::RShift;
                case 37: return LWWS_Key::Special::LControl;
                case 105: return LWWS_Key::Special::RControl;
                case 64: return LWWS_Key::Special::Alt;
                case 108: return LWWS_Key::Special::AltGr;
                case 34: return LWWS_Key::Special::Oem_1;
                case 49: return LWWS_Key::Special::Oem_2;
                case 35: return LWWS_Key::Special::Oem_3;
                case 20: return LWWS_Key::Special::Oem_4;
                case 48: return LWWS_Key::Special::Oem_5;
                case 21: return LWWS_Key::Special::Oem_6;
                case 47: return LWWS_Key::Special::Oem_7;
                case 51: return LWWS_Key::Special::Oem_8;
                case 127: return LWWS_Key::Special::Sleep; // todo: make sure, it's really this one
                case 67: return LWWS_Key::Special::F1;
                case 68: return LWWS_Key::Special::F2;
                case 69: return LWWS_Key::Special::F3;
                case 70: return LWWS_Key::Special::F4;
                case 71: return LWWS_Key::Special::F5;
                case 72: return LWWS_Key::Special::F6;
                case 73: return LWWS_Key::Special::F7;
                case 74: return LWWS_Key::Special::F8;
                case 75: return LWWS_Key::Special::F9;
                case 76: return LWWS_Key::Special::F10;
                case 95: return LWWS_Key::Special::F11;
                case 96: return LWWS_Key::Special::F12;
                case 77: return LWWS_Key::Special::NumLock;
                case 22: return LWWS_Key::Special::BackSpace;
                case 111: return LWWS_Key::Special::Up;
                case 113: return LWWS_Key::Special::Left;
                case 116: return LWWS_Key::Special::Down;
                case 114: return LWWS_Key::Special::Right;
                case 118: return LWWS_Key::Special::Insert;
                case 110: return LWWS_Key::Special::Home;
                case 115: return LWWS_Key::Special::End;
                case 112: return LWWS_Key::Special::PageUp;
                case 117: return LWWS_Key::Special::PageDown;
                case 119: return LWWS_Key::Special::Delete;
                default: return LWWS_Key::Special::RandomKey;
            }
        }
    };
}
