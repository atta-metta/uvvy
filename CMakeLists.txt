cmake_minimum_required(VERSION 2.8)

project(mettanode)
include(CTest)

include(cmake/setup.cmake)

# Export compile commands for ST3 clang plugin.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set flag for submodules build
set(BUILD_IN_UVVY ON)

if (APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
endif (APPLE)

include_directories(3rdparty/endian) # Additional unofficial boost libraries
include_directories(3rdparty) # sodiumpp

include_directories(arsenal/include) # For routing access to sss host and link_receiver
include_directories(krypto/include) # For cryptographic primitives
include_directories(uia/include)
include_directories(sss/include) # For routing access to sss host and link_receiver
include_directories(routing/include) # For sss access to routing client
include_directories(voicebox/include)

add_subdirectory(3rdparty/sodiumpp)
add_subdirectory(arsenal)
#add_subdirectory(krypto)
#add_subdirectory(voicebox)
#add_subdirectory(filesyncbox)

add_subdirectory(uia)
add_subdirectory(sss)
add_subdirectory(routing)
#add_subdirectory(naming)
#add_subdirectory(nat)

#add_subdirectory(3rdparty/opus)
#add_subdirectory(3rdparty/rtaudio)
#add_subdirectory(3rdparty/metakit)
#add_subdirectory(ui)

#add_subdirectory(demos)
#add_subdirectory(playground)
