
cmake_minimum_required(VERSION 3.10)

project(ds_sch_people_count)

# Set CUDA Version
set(CUDA_VER "12.1" CACHE STRING "CUDA version")

# Include CUDA paths
include_directories(/usr/local/cuda-${CUDA_VER}/include)
link_directories(/usr/local/cuda-${CUDA_VER}/lib64/)

# Set NVDS Version
set(NVDS_VERSION "6.3" CACHE STRING "NVDS version")
set(LIB_INSTALL_DIR "/opt/nvidia/deepstream/deepstream-${NVDS_VERSION}/lib/")
set(APP_INSTALL_DIR "/opt/nvidia/deepstream/deepstream-${NVDS_VERSION}/bin/")

# Check platform
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    add_definitions(-DPLATFORM_TEGRA)
endif()

# Source and Header files
set(SRCS
        main.cpp
        nvdsanalytics_meta_utils.cpp
        pad_probe_utils.cpp
        msg_build.cpp
        multistream_utils.cpp
        )
file(GLOB INCS "*.h")

# Find packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

# Include paths
include_directories(${GST_INCLUDE_DIRS} ../../../includes ./includes)

# Add executable
add_executable(${PROJECT_NAME} ${SRCS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
        ${GST_LIBRARIES}
        -L${LIB_INSTALL_DIR}
        -lnvdsgst_meta
        -lnvds_meta
        -lnvdsgst_helper
        -lm
        -lnvbufsurface
        -lnvbufsurftransform
        -lnvdsgst_helper
        -lnvds_batch_jpegenc
        -lcudart
        -pthread
        -lcurl
        -lcuda
        )
# Link with rpath
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR})

# Install the executable
install(TARGETS ${PROJECT_NAME} DESTINATION ${APP_INSTALL_DIR})


