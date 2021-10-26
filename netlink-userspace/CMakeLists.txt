cmake_minimum_required(VERSION 2.8.12)

project(nl_simple)


set(CMAKE_C_FLAGS "-static-libgcc")
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "." "/usr/local/sniper/lib")

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # copy versions/kernel-2.8.32/* to handle
    message(STATUS "copy versions/kernel-2.8.32/* to handle")
    file(GLOB HANDLES_FILES "versions/kernel-2.8.32/*")
    file(COPY ${HANDLES_FILES} DESTINATION ${CMAKE_SOURCE_DIR}/nl/handle)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    # 获取当前内核版本
    find_file (KERNEL_VERSION_FOUND version PATHS /proc)
    if (KERNEL_VERSION_FOUND)
        file (STRINGS ${KERNEL_VERSION_FOUND} KERNEL_VERSION)
        file (STRINGS ${KERNEL_VERSION_FOUND} NEED_LIBNL REGEX 2.6.18)
        message(STATUS "kernel version: ${KERNEL_VERSION}")
        # 针对2.6.18版本内核需要编译单独的libnl, 当前客户端不支持2.6.18版本内核
        if (NEED_LIBNL)
            # 库的编译未测试
            set(LIBNL_SOURCE_DIR_PATH ${CMAKE_SOURCE_DIR}/nl/libnl_source)
            set(LIBNL_SOURCE_TAR_PATH ${CMAKE_SOURCE_DIR}/nl/deps/libnl-1.0-pre6.tar.gz)

            file(REMOVE_RECURCE ${LIBNL_SOURCE_DIR_PATH})
            file(MAKE_DIRECTORY ${LIBNL_SOURCE_DIR_PATH})
            # unpack the source package
            execute_process(tar -xvf ${LIBNL_SOURCE_TAR_PATH} -C ${LIBNL_SOURCE_DIR_PATH} --strip-components 1)
            message(STATUS "build libnl")
            ExternalProject_Add(
              libnl
              SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libnl_source
              CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libnl_source/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/libnl
              PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/libnl
              BUILD_COMMAND make
              BUILD_IN_SOURCE 1
            )

            message(STATUS "copy versions/1.0-pre6/* to handle")
            file(GLOB HANDLES_FILES "versions/1.0-pre6/*")
            file(COPY ${HANDLES_FILES} DESTINATION ${CMAKE_SOURCE_DIR}/nl/handle)
        else()
            # copy versions/kernel-2.8.32/* to handle
            message(STATUS "copy versions/kernel-2.8.32/* to handle")
            file(GLOB HANDLES_FILES "versions/kernel-2.8.32/*")
            file(COPY ${HANDLES_FILES} DESTINATION ${CMAKE_SOURCE_DIR}/nl/handle)
        endif(NEED_LIBNL)

    endif (KERNEL_VERSION_FOUND)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libnl)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libnl/include)
    LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libnl/lib)
endif()

INCLUDE_DIRECTORIES(./)
INCLUDE_DIRECTORIES(./handle)

aux_source_directory(./ SOURCE_FILES)
aux_source_directory(./handle SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
SET(LIBRARY_OUTPUT_PATH lib)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libnl/lib/libnl.so)
    target_link_libraries(${PROJECT_NAME} nl)
endif()

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
