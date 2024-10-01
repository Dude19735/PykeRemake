
### Do everything with Vk_GpuTask
1. rename Vk_GpuTask to Vk_Task
2. Vk_Task needs some section for all connected objects that is serialized
    * assume N obj
    * assume M clients (GPU + CPU mix)
        * for example M 3D viewers and N 3D objects
        * all 3D objects are connected to a subset of the M 3D viewers
    * if obj_i is updated
        * create Vk_GpuBufferUpdateTask for all GPU buffers
        * needs a Vk_TaskTree that is updated on the fly
            * level 0: already submitted
            * level 1: next Vk_Task batch => connect all semaphores
            * level -1...: maybe keep some history, just to make it funny
            * Vk_TaskTree made of Vk_TaskDef struct
                * Vk_TaskDef needs
                    * std::function<void(Vk_TaskParams)> before
                    * void(*TRecordFunction)(...., Vk_TaskParams)
                    * void(*TSubmitFunction)(...., Vk_TaskParams)
                    * std::function<void(Vk_TaskParams)> after
        * move all updates into Vk_Tasks
3. figure out some system how to synchronize all operations on the clients
    * obj_i is updated from a program using manual obj_i.update
    * obj_i is updated from the viewer using a camera move
        => all clients that are connected to obj_i need some way of performing the updates without interfearing with the computation/rendering process
        * how to deal with synchronous updates
        * how to deal with asynchronous updates
        => somehow all connected clients have to stop for a while
        * maybe insert some <stop> ... <continue> tasks?









