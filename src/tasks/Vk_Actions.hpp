#pragma once

#include <functional>

#include "../Defines.h"
#include "Vk_Action.hpp"
#include "Vk_ActionPool.hpp"

namespace VK5 {
    class Vk_Actions {
    private:
        Vk_ActionPool _actionPool;
#ifdef PYVK
		std::map<int, py::function> _actions;
#else
		std::map<int, std::shared_ptr<VK5::Vk_Action>> _actions;
#endif

    public:
        Vk_Actions(){
            _actionPool.start(1, this);
        }

        ~Vk_Actions(){
            _actionPool.stop();
        }

        template<class ObjType>
		bool registerAction(LWWS::LWWS_Key::Special key, ObjType* obj, t_func<ObjType> f, int cameraId=-1){
			return registerAction(LWWS::LWWS_Key::KeyToInt(key), obj, f, cameraId);
		}

		template<class ObjType>
		bool registerAction(char c, ObjType* obj, t_func<ObjType> f, int cameraId=-1){
			return registerAction(LWWS::LWWS_Key::KeyToInt(c), obj, f, cameraId);
		}

		template<class ObjType>
		bool registerAction(int key, ObjType* obj, t_func<ObjType> f, int cameraId=-1){
			if(cameraId >= 0){
				UT::Ut_Logger::Warn(typeid(this), "Per camera localized actions not supported yet!");
				return false;
			}
			else{
				if(_actions.find(key) != _actions.end()){
					UT::Ut_Logger::Warn(typeid(this), "Tried to register the same key twice! Unregister key first.");
					return false;
				}
				_actions.insert({
					key,
					Vk_TFunc(obj, f, {}).get()
				});
				return true;
			}
		}

		bool unregisterAction(LWWS::LWWS_Key::Special key){
			return unregisterAction(LWWS::LWWS_Key::KeyToInt(key));
		}

		bool unregisterAction(char key){
			return unregisterAction(LWWS::LWWS_Key::KeyToInt(key));
		}

		bool unregisterAction(int key){
			if(_actions.find(key) == _actions.end()){
				UT::Ut_Logger::Warn(typeid(this), "Tried to unregister non-existing key! Register key first.");
				return false;
			}

			_actions.erase(key);
			return true;
		}

		void execAction(LWWS::LWWS_Key::Special key){
			execAction(LWWS::LWWS_Key::KeyToInt(key));
		}

		void execAction(char key){
			execAction(LWWS::LWWS_Key::KeyToInt(key));
		}

		void execAction(int key, std::function<void()> followup){
			if(_actions.find(key) != _actions.end()){
#ifdef PYVK
				// _threadPool.enqueueJob(&_actions.at(key), std::bind(&Vk_Viewer::_redraw, this));
                _actionPool.enqueueJob(&_actions.at(key), followup);
#else
				// _actionPool.enqueueJob(_actions.at(key), std::bind(&Vk_Viewer::_redraw, this));
                _actionPool.enqueueJob(_actions.at(key), followup);
#endif
			}
		}
    private:
    };
}