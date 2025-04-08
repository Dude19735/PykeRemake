# Introduction
This is the successor project of [Pyke](https://github.com/Dude19735/Pyke).

# Intention
The goal is to create a fully asynchronous viewer where data can be transfered between CPU and GPU and debug points can be set inside the program where the viewer is used to visualize things without blocking the rendering process. At the end, the viewer should allow someone to use for example numpy arrays to deal with 3D objects in Python and directly render them into a nice, representable scene using a slim and uncomplicated API.

As opposed to its predecessor, this one has a task-submission system where the CPU can submit rendering tasks to the GPU without using locks. The task system is a construct that ping-pongs the various stages required for rendering between a series of threads governed by condition variables and ensures that the parts of the rendering process that require mutual exclusion, for example vkQueueSubmit to a specific VkQueue are protected.

[Pyke](https://github.com/Dude19735/Pyke) uses GLFW that requires doing subdivision of the viewport on the GPU side to facilitate multiple renderers inside of one window. That introduces complexity. This one contains a custom windowing system that supports grid layouts (multiple sub-windows) while processing the input events based on the mouse position in the parent window. This enables binding one entire Vulkan renderer to an operating system viewport contained inside a parent window.

# How to run
* install the requirements as described in [Pyke](https://github.com/Dude19735/Pyke)
* open ```vk_test_all.cpp``` inside the ```test``` folder
* configure test case
  ```CPP
  #include "vk5_test_terminal_colors.cpp"
  #include "vk5_test_viewer.cpp"
  #include "vk5_test_device.cpp"
  #include "vk5_test_data_buffers.cpp"
  ```

# Current state
* no graphics are actually rendered yet
* device abstraction is complete
* an asynchronous task system is implemented
* windowing system works as intended with X11, Windows is untested, Wayland is postponed due to Nvidia graphics driver issues on Wayland
