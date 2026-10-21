# mkdir -p build
# cd build
# cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..

cmake_minimum_required(VERSION 3.16)

project(ta2ool LANGUAGES CXX OBJCXX)

if(NOT APPLE)
    message(FATAL_ERROR "ta2ool supports macOS only")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 11)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_executable(ta2ool
    main.mm
    GetDevice.mm
    common.cpp
    Options.cpp
    help.cpp
    listdevice.cpp
    deviceinfo.cpp
    listapp.cpp
    listappname.cpp
    listappraw.cpp
    fileshare_mkdir.cpp
    fileshare_ls.cpp
    fileshare_rename.cpp
    fileshare_rm.cpp
    fileshare_pull.cpp
    fileshare_push.cpp
    listfilesharedapp.cpp
    file_ls.cpp
    file_pull.cpp
    file_push.cpp
    file_rm.cpp
    app_ls.cpp
    app_pull.cpp
    app_push.cpp
    install_app.cpp
    uninstall_app.cpp
    location.cpp
    location_reset.cpp
    show_devmode.cpp
    syslog.cpp
)

target_compile_options(ta2ool PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>)

# MobileDevice is a private framework
find_library(MOBILEDEVICE_FRAMEWORK MobileDevice
    PATHS /Library/Apple/System/Library/PrivateFrameworks
    NO_DEFAULT_PATH
    REQUIRED
)

target_link_libraries(ta2ool PRIVATE
    "-framework Foundation"
    "-framework CoreFoundation"
    ${MOBILEDEVICE_FRAMEWORK}
)

install(TARGETS ta2ool RUNTIME DESTINATION bin)
