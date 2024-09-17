#pragma once

#include <WinUser.h>
#include "lwws_key.hpp"

namespace LWWS {
    class LWWS_Key_Win: public LWWS_Key {
        static bool IsPressed(int key) {
            HKL local = GetKeyboardLayout(0);
            short vKey;
            if(key > 1000){
                Special k = static_cast<Special>(key-1000);
                vKey = VkKeyScanExA(FromSpecialKey(k), local);
            }
            else{
                vKey = VkKeyScanExA(static_cast<char>(key), local);
            }
            short state = GetKeyState(vKey);
            bool res = (state & 0x8000) > 0;
            return res;
        }
        
    public:
        static bool LWWS_ShowCursor(bool show){
            ShowCursor(show);
            return true;
        }

        static bool CharFilter(char c){
            int cc = static_cast<int>(c);
            if(cc < 0) return true;
            switch(cc){
                case 27: return true; // ESC
                case 8: return true; // backspace
                case 36: return true; // oem_5
                case 39: return true; // oem_4
                case 94: return true; // oem_6
                default: return false;
            }
        }

        static std::set<int> getStillPressed(const std::set<int>& toCheck){
            std::set<int> stillPressed;
            for(const auto& k : toCheck){
                if(LWWS_Key_Win::IsPressed(k)) stillPressed.insert(k);
            }
            return stillPressed;
        }

        static uint64_t FromSpecialKey(Special key){
            switch(key) {
            case(LWWS_Key::Special::Sleep): return VK_SLEEP;
            case(LWWS_Key::Special::F1): return VK_F1;
            case(LWWS_Key::Special::F2): return VK_F2;
            case(LWWS_Key::Special::F3): return VK_F3;
            case(LWWS_Key::Special::F4): return VK_F4;
            case(LWWS_Key::Special::F5): return VK_F5;
            case(LWWS_Key::Special::F6): return VK_F6;
            case(LWWS_Key::Special::F7): return VK_F7;
            case(LWWS_Key::Special::F8): return VK_F8;
            case(LWWS_Key::Special::F9): return VK_F9;
            case(LWWS_Key::Special::F10): return VK_F10;
            case(LWWS_Key::Special::F11): return VK_F11;
            case(LWWS_Key::Special::F12): return VK_F12;      
            case(LWWS_Key::Special::NumLock): return VK_NUMLOCK;
            case(LWWS_Key::Special::RControl): return VK_RCONTROL;
            case(LWWS_Key::Special::LControl): return VK_LCONTROL;
            case(LWWS_Key::Special::RShift): return VK_RSHIFT;
            case(LWWS_Key::Special::LShift): return VK_LSHIFT;
            case(LWWS_Key::Special::Up): return VK_UP;
            case(LWWS_Key::Special::Left): return VK_LEFT;
            case(LWWS_Key::Special::Down): return VK_DOWN;
            case(LWWS_Key::Special::Right): return VK_RIGHT;
            case(LWWS_Key::Special::BackSpace): return VK_BACK;
            case(LWWS_Key::Special::Escape): return VK_ESCAPE;
            case(LWWS_Key::Special::Insert): return VK_INSERT;
            case(LWWS_Key::Special::Home): return VK_HOME;
            case(LWWS_Key::Special::End): return VK_END;
            case(LWWS_Key::Special::PageUp): return VK_PRIOR;
            case(LWWS_Key::Special::PageDown): return VK_NEXT;
            case(LWWS_Key::Special::Delete): return VK_DELETE;
            case(LWWS_Key::Special::AltGr): return VK_RMENU;
            case(LWWS_Key::Special::Alt): return VK_LMENU;
            case(LWWS_Key::Special::Oem_1): return VK_OEM_1;
            case(LWWS_Key::Special::Oem_2): return VK_OEM_2;
            case(LWWS_Key::Special::Oem_3): return VK_OEM_3;
            case(LWWS_Key::Special::Oem_4): return VK_OEM_4;
            case(LWWS_Key::Special::Oem_5): return VK_OEM_5;
            case(LWWS_Key::Special::Oem_6): return VK_OEM_6;
            case(LWWS_Key::Special::Oem_7): return VK_OEM_7;
            case(LWWS_Key::Special::Oem_8): return VK_OEM_8;
            default: return 0;
            }
        }

        static LWWS_Key::Special ToSpecialKey(WPARAM K, LPARAM L){
            switch(K) {
            case(VK_SLEEP):
                return LWWS_Key::Special::Sleep;  
            case(VK_F1): // ok
                return LWWS_Key::Special::F1; 
            case(VK_F2): // ok
                return LWWS_Key::Special::F2;       
            case(VK_F3): // ok
                return LWWS_Key::Special::F3;       
            case(VK_F4): // ok
                return LWWS_Key::Special::F4;       
            case(VK_F5): // ok
                return LWWS_Key::Special::F5;       
            case(VK_F6): // ok
                return LWWS_Key::Special::F6;       
            case(VK_F7): // ok
                return LWWS_Key::Special::F7;       
            case(VK_F8): // ok
                return LWWS_Key::Special::F8;       
            case(VK_F9): // ok
                return LWWS_Key::Special::F9;       
            case(VK_F10): 
                return LWWS_Key::Special::F10;      
            case(VK_F11): // ok
                return LWWS_Key::Special::F11;      
            case(VK_F12): // ok
                return LWWS_Key::Special::F12;          
            case(VK_NUMLOCK): // ok
                return LWWS_Key::Special::NumLock;  
            case(VK_CONTROL):{
                uint8_t l = static_cast<uint8_t>(L >> 24);
                if(l & 0x1){
                    return LWWS_Key::Special::RControl;
                }
                /**
                 * Inspired by GLFW win32_window.c
                 * """
                 *   // NOTE: Alt Gr sends Left Ctrl followed by Right Alt
                 *   // HACK: We only want one event for Alt Gr, so if we detect
                 *   //       this sequence we discard this Left Ctrl message now
                 *   //       and later report Right Alt normally
                 * """
                 */
                MSG next;
                const DWORD time = GetMessageTime();

                if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
                {
                    if (next.message == WM_KEYDOWN ||
                        next.message == WM_SYSKEYDOWN ||
                        next.message == WM_KEYUP ||
                        next.message == WM_SYSKEYUP)
                    {
                        if (next.wParam == VK_MENU && next.time == time)
                        {
                            uint8_t nl = static_cast<uint8_t>(next.lParam >> 24);
                            if(nl & 0x1) {
                                // Return something special
                                return LWWS_Key::Special::IntentionalSkip;
                            }
                        }
                    }
                }
                return LWWS_Key::Special::LControl; 
            }
            case(VK_SHIFT):{
                uint8_t l = static_cast<uint8_t>(L >> 20);
                if(l & 0x1){
                    return LWWS_Key::Special::RShift;
                }
                return LWWS_Key::Special::LShift;
            }
            case(VK_UP): // ok
                return LWWS_Key::Special::Up; 
            case(VK_LEFT): // ok
                return LWWS_Key::Special::Left; 
            case(VK_DOWN): // ok
                return LWWS_Key::Special::Down; 
            case(VK_RIGHT): // ok
                return LWWS_Key::Special::Right; 
            case(VK_BACK):
                return LWWS_Key::Special::BackSpace;
            case(VK_ESCAPE):
                return LWWS_Key::Special::Escape;
            case(VK_INSERT):
                return LWWS_Key::Special::Insert;
            case(VK_HOME):
                return LWWS_Key::Special::Home;
            case(VK_END):
                return LWWS_Key::Special::End;
            case(VK_PRIOR):
                return LWWS_Key::Special::PageUp;
            case(VK_NEXT):
                return LWWS_Key::Special::PageDown;
            case(VK_DELETE):
                return LWWS_Key::Special::Delete;
            case(VK_MENU):{
                uint8_t l = L >> 24;
                if(l & 0x1){
                    return LWWS_Key::Special::AltGr;
                }
                return LWWS_Key::Special::Alt;
            }
            case(VK_OEM_1):
                return LWWS_Key::Special::Oem_1;
            case(VK_OEM_2):
                return LWWS_Key::Special::Oem_2;
            case(VK_OEM_3):
                return LWWS_Key::Special::Oem_3;
            case(VK_OEM_4):
                return LWWS_Key::Special::Oem_4;
            case(VK_OEM_5):
                return LWWS_Key::Special::Oem_5;
            case(VK_OEM_6):
                return LWWS_Key::Special::Oem_6;
            case(VK_OEM_7):
                return LWWS_Key::Special::Oem_7;
            case(VK_OEM_8):
                return LWWS_Key::Special::Oem_8;
            default: 
                return LWWS_Key::Special::RandomKey; 
            }
        }

        static char ToChar(WPARAM c){
            return static_cast<char>(c);
        }
    };
}
