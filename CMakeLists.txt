cmake_minimum_required(VERSION 2.8)
project(SoundManager)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_CXX_STANDARD 11)
set(SOUND_SRC
    src/soundmanager.cc
    src/soundplayer.cc
    src/soundmixer.cc
    src/soundpcm.cc
    src/soundpcmhwparams.cc
    src/soundpcmenums.cc
    src/soundstreamcreator.cc
    src/wavstream.cc
    )

include_directories(${PROJECT_SOURCE_DIR}/include)
add_library(sound ${SOUND_SRC})
target_link_libraries(sound pthread asound)

add_executable(sound_cli main.cpp)
target_link_libraries(sound_cli sound )
