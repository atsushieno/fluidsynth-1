cmake_minimum_required(VERSION 3.6.0)

project ( fluidsynth-assetloader C )

option ( enable-android-amidi "compile Android AMidi support (requires AMidi)" off )

include ( CheckIncludeFile )

unset ( ANDROID_AMIDI_SUPPORT CACHE )
if ( enable-android-amidi )
  check_include_file ( amidi/AMidi.h ANDROID_AMIDI_SUPPORT )
  find_library ( ANDROID_AMIDI_LIBS amidi )
else ( enable-android-amidi )
  unset ( ANDROID_AMIDI_LIBS CACHE )
endif ( enable-android-amidi )

if ( ANDROID_AMIDI_SUPPORT )
  set ( fluid_android_amidi_SOURCES fluid_android_amidi.c )
  include_directories ( ${ANDROID_AMIDI_INCLUDE_DIRS} )
  link_libraries ( amidi )
endif ( ANDROID_AMIDI_SUPPORT )



set ( fluidsynth-assetloader_sources fluid_androidasset.c ${fluid_android_amidi_SOURCES} )

add_library ( fluidsynth-assetloader SHARED ${fluidsynth-assetloader_sources} )

target_compile_options ( fluidsynth-assetloader
                        PRIVATE -v
                        PRIVATE -Wall
                        PRIVATE "$<$<CONFIG:DEBUG>:-Werror>") # Only include -Werror when building debug config

include_directories ( ../../include )

set ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L../../../dist/${ANDROID_ABI} -lfluidsynth" )

target_link_libraries ( fluidsynth-assetloader PRIVATE log android )
