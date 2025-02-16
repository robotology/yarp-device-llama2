# SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

yarp_prepare_plugin(llama2Device
  CATEGORY device
  TYPE Llama2Device
  INCLUDE Llama2Device.h
  INTERNAL ON
)


if(NOT SKIP_llama2Device)
  yarp_add_plugin(yarp_llama2Device)
  generateDeviceParamsParser(Llama2Device yarp_llama2Device)

  target_sources(yarp_llama2Device
    PRIVATE
      Llama2Device.cpp
      Llama2Device.h
      Llama2Device_ParamsParser.cpp
      Llama2Device_ParamsParser.h
      Utils.cpp
      Utils.h
  )

  # cmake_path(SET LLAMA_INCLUDE_PATH "${LLAMA_INSTALL_PATH}/include")
  # cmake_path(SET LLAMA_LIBRARY_PATH "${LLAMA_INSTALL_PATH}/lib")

  target_link_directories(yarp_llama2Device
  PUBLIC
    # ${CMAKE_PREFIX_PATH}/lib
  )

  target_include_directories(yarp_llama2Device
  PUBLIC
    "${LLAMA_INCLUDE_PATH}"
    ${CMAKE_PREFIX_PATH}/include
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/llama.cpp/common"
    "${CMAKE_SOURCE_DIR}/llama.cpp/include"
    # "/home/leonardo/Repos/llama.cpp/common"
    # "/home/leonardo/Repos/llama.cpp/include"
  )
  message(STATUS "LLAMA_INCLUDE_PATH: ${LLAMA_INCLUDE_PATH}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(CURL REQUIRED)
target_link_libraries(yarp_llama2Device PRIVATE CURL::libcurl)

  target_link_libraries(yarp_llama2Device
    PRIVATE
      YARP::YARP_os
      YARP::YARP_sig
      YARP::YARP_dev
      llama
      common
      #"${CMAKE_SOURCE_DIR}/llama.cpp/src/libllama.so"
      #"${CMAKE_SOURCE_DIR}/llama.cpp/build/common/libcommon.a"
      # "/home/leonardo/Repos/llama.cpp/build/src/libllama.so"
      # "/home/leonardo/Repos/llama.cpp/build/common/libcommon.a"
  )

  yarp_install(
    TARGETS yarp_llama2Device
    EXPORT yarp-device-llama2Device
    COMPONENT yarp-device-llama2Device
    LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
    ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
    YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR}
  )

  if(YARP_COMPILE_TESTS)
    add_subdirectory(tests)
  endif()

  set_property(TARGET yarp_llama2Device PROPERTY FOLDER "Plugins/Device")
endif()
