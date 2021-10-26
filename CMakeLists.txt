cmake_minimum_required(VERSION 3.5)

project(netlink-cmake LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(BUILD_HELLOWORLD "build helloworld demo" ON)

include(ExternalProject)

if (NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    message(FATAL_ERROR "${PROJECT_NAME} not supported ${CMAKE_SYSTEM_NAME}")
endif()

execute_process(COMMAND "uname" "-r" OUTPUT_VARIABLE KERNAL_RELEASE OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Linux kernel release: ${KERNAL_RELEASE}")

function(libNetlinkUserspaceMain)

    set(USERSPACE_SOURCES netlink-userspace/NetlinkTransport.cpp netlink-userspace/select_event.cpp)

    string(FIND ${KERNAL_RELEASE} "2.6.18" OLD_RELEASE)
    if (${OLD_RELEASE} GREATER 0)
        list(APPEND USERSPACE_SOURCES "netlink-userspace/versions/1.0-pre6/NetlinkTransportPrivate.cpp")
    else()
        list(APPEND USERSPACE_SOURCES "netlink-userspace/versions/kernel-2.8.32/NetlinkTransportPrivate.cpp")
    endif()

    add_library(thirdparty_libnetlink_userspace_static ${USERSPACE_SOURCES})
    target_include_directories(thirdparty_libnetlink_userspace_static PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/netlink-userspace )

    if (${OLD_RELEASE} GREATER 0)
        target_include_directories(thirdparty_libnetlink_userspace_static PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/netlink-userspace/versions/1.0-pre6/")
    else()
        target_include_directories(thirdparty_libnetlink_userspace_static PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/netlink-userspace/versions/kernel-2.8.32/")
    endif()

    add_library(thirdparty_libnetlink_userspace INTERFACE)
    target_link_libraries(thirdparty_libnetlink_userspace INTERFACE thirdparty_libnetlink_userspace_static)
    target_include_directories(thirdparty_libnetlink_userspace PUBLIC INTERFACE  ${CMAKE_CURRENT_SOURCE_DIR}/netlink-userspace)

endfunction()

function(libNetlinkKerenlMain)
    set(kernel_module_name "netlink_kernel")
    set(kernel_dir "/lib/modules/${KERNAL_RELEASE}/build")
    message(STATUS "kernel_dir: ${kernel_dir}")
    find_file(kernel_makefile NAMES Makefile PATHS ${kernel_dir} NO_DEFAULT_PATH)
    if (NOT kernel_makefile)
        message(FATAL_ERROR "There is no makefile in ${kernel_dir}")
    endif()

    set(KERNEL_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/netlink-kernel")

    # add static library, exclude module.c or main.c
    list(APPEND module_sources "${KERNEL_MODULE_DIR}/src/nl_kernel.c")

    set(KBUILD_TEMPLATE "${KERNEL_MODULE_DIR}/Kbuild.in")

    # generate a "Kbuild" file
    string(REPLACE ";" " " module_source_string "${module_sources}")
    string(REPLACE ".c" ".o" module_source_string "${module_source_string}")
    string(REPLACE "${KERNEL_MODULE_DIR}/" "" module_source_string "${module_source_string}")

    message(STATUS "Kbuild objs: ${module_source_string}")

    configure_file(${KBUILD_TEMPLATE} ${KERNEL_MODULE_DIR}/Kbuild)

    # make the source files accessible to the kernel Makefile

    set(MAKE_CMD make)

    find_program(compiledb_path compiledb)
    if (compiledb_path)
        message(STATUS "compiledb path: ${compiledb_path}")
        set(MAKE_CMD compiledb make)
    endif()

    set(module_cmd ${CMAKE_MAKE_PROGRAM} -c ${kerneldir} M=${CMAKE_CURRENT_BINARY_DIR_DIR})
    add_custom_command(OUTPUT ${kernel_module_name}.ko
        COMMAND ${MAKE_CMD}
        WORKING_DIRECTORY ${KERNEL_MODULE_DIR}
        DEPENDS ${module_sources} "${CMAKE_CURRENT_SOURCE_DIR}/netlink-kernel/Kbuild"
        VERBATIM
        )
    add_custom_target(module ALL DEPENDS ${kernel_module_name}.ko)
    add_custom_target(module-clean COMMAND ${module_cmd} clean)
    message(STATUS "kernel module dir: ${KERNEL_MODULE_DIR}")

#    ExternalProject_Add(netlink-kernel
#        CONFIGURE_COMMAND ""
#        BUILD_COMMAND make
#        BUILD_IN_SOURCE 1
#        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/netlink-kernel)
endfunction()

function(helloworldMain)
    add_executable(helloworld_netlink_userspace "examples/helloworld.cpp")
    target_link_libraries(helloworld_netlink_userspace thirdparty_libnetlink_userspace pthread libc++abi.a dl rt)
endfunction()

libNetlinkUserspaceMain()
libNetlinkKerenlMain()

if (BUILD_HELLOWORLD)
    helloworldMain()
endif()
