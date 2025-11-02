# Vulkan Compute

Simple framework used for running compute shaders using Vulkan API.

## Building

This project uses a few external dependencies

- [glfw](https://www.glfw.org/) for the window management.
- [stb](https://github.com/nothings/stb) for image loading.

these projects are included as git submodules, so before building the repo should be cloned using the recursive option:

```sh
git clone --recursive git@github.com:SignedSnow0/VulkanCompute.git
```

or alternatively, if the repo has already been cloned, pull the submodules:

```sh
git submodule init && git submodule update
```

The project also requires the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) to be installed into the system, the build system checks the environment variable `VULKAN_SDK` to find the install location.

The project can be built using `CMake` with the following commands:

```sh
cmake -S . -B bin/
cmake --build bin/
```
