![YARP logo](doc/images/yarp-robot-24.png?raw=true "yarp-device-template")
Yarp device for xxx
=====================

This repository contains the YARP plugin for Llama2.

:construction: This repository is currently work in progress. :construction:
:construction: The software contained is this repository is currently under testing. :construction: APIs may change without any warning. :construction: This code should be not used before its first official release :construction:

Documentation
-------------

Documentation of the individual devices is provided in the official Yarp documentation page:
[![YARP documentation](https://img.shields.io/badge/Documentation-yarp.it-19c2d8.svg)](https://yarp.it/latest/group__dev__impl.html)


Installation
-------------
To install Llama2Device we simply need to clone this repository and run setup_submodule.sh:
~~~
# Clone the correct version of llama.cpp library
./setup_submodule.sh
~~~
this file clones llama.cpp library to a specific working version. It is recommended to stick to this version and not to use a newer one since the functioning is not guaranteed.

Before compilation move inside build folder and type:
~~~
ccmake ..
~~~
The following flags must be enabled: 
~~~
 ALLOW_DEVICE_PARAM_PARSER_GENE   ON
 BUILD_SHARED_LIBS                ON
 LLAMA_ALL_WARNINGS               ON
 LLAMA_BUILD_COMMON               ON
 LLAMA_BUILD_EXAMPLES             ON
 LLAMA_BUILD_SERVER               ON
~~~

It is also fundamental to add the /build/bin folder to system PATH, it can be done by running these commands:
~~~
echo 'export PATH="path_to_build/bin_folder::$PATH"' >> ~/.bashrc
source ~/.bashrc
~~~

[Curl](https://curl.se/) must be installed to use the device, it can be installed by executing these commands:
~~~
sudo apt update
sudo apt install curl
~~~

### Build with pure CMake commands

~~~
# Configure, compile and install
cmake -S. -Bbuild -DCMAKE_INSTALL_PREFIX=<install_prefix>
cmake --build build
cmake --build build --target install
~~~
These commands will compile and install llama.cpp library and Llama2Device.

Enable llama.cpp GPU computation
-------------
In order to allow llama.cpp to use the gpu power to compute the answers it is mandatory to follow these steps.
Open project build folder, then type:
~~~
ccmake ..
~~~
Scroll down until you find the flag "GGML_CUDA ", turn it on, press "c" to configure and then "g" to generate.
Now compile the project again using these instructions:
 ~~~
# Configure, compile and install
cmake -S. -Bbuild -DCMAKE_INSTALL_PREFIX=<install_prefix>
cmake --build build
cmake --build build --target install
~~~
The library will automatically detect the gpu as a device and will use it to compute results faster.
The GGML_CUDA flag automatically turns on other related flags, double check that all of the following flags have been enabled:
 ~~~
GGML_ACCELERATE                  ON
GGML_CCACHE                      ON
GGML_CUDA                        ON
GGML_CUDA_GRAPHS                 ON
GGML_LASX                        ON
GGML_LLAMAFILE                   ON
GGML_LSX                         ON
GGML_NATIVE                      ON
GGML_OPENMP                      ON
~~~

Note: it is mandatory to have an Nvidia gpu and have CUDA toolkit version > 10 installed.
To install CUDA toolkit follow this [official guide](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/).

Configuration
-------------

The device is able to run every .gguf LLM model.
The models can be downloaded from [huggingface](https://huggingface.co/models?library=gguf&sort=trending).
The downloaded model must be placed inside models folder.

Usage
---------

Assuming that the user has already installed [YARP](https://www.github.com/robotology/yarp) and the related LLM devices, one can use this basic configuration example:
~~~
yarpserver
~~~

~~~
yarprobotinterface --config assets/llama2Device_full.xml
~~~

~~~
yarp rpc /LLM_nws/rpc/rpc:i
>>help
Responses:
  *** Available commands:
  setPrompt
  readPrompt
  ask
  getConversation
  deleteConversation
  refreshConversation
  help
>>
>>setPrompt "You are a geography expert who is very concise in its answers"
Response: [ok]
>>ask "What is the capital of Germany?"
Response: [ok] Berlin.
~~~

CI Status
---------

:construction: This repository is currently work in progress. :construction:

[![Build Status](https://github.com/robotology/yarp-device-template/workflows/CI%20Workflow/badge.svg)](https://github.com/robotology/yarp-device-template/actions?query=workflow%3A%22CI+Workflow%22)

License
---------

:construction: This repository is currently work in progress. :construction:

Maintainers
--------------
This repository is maintained by:

| | |
|:---:|:---:|
| [<img src="https://github.com/randaz81.png" width="40">](https://github.com/randaz81) | [@randaz81](https://github.com/randaz81) |