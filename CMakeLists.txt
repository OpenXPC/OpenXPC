cmake_minimum_required(VERSION 3.15)
project(LaunchD-Reloaded LANGUAGES C CXX OBJC OBJCXX)

#
# ObjC setup
#

set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -fexceptions -fobjc-exceptions \
	-Xclang -fobjc-runtime=gnustep -Xclang -fblocks \
	-Wno-nullability-completeness")
set(CMAKE_OBJCXX_FLAGS "${CMAKE_OBJCXX_FLAGS} ${CMAKE_OBJC_FLAGS}")

if (APPLE)
	set(OBJC_LIBRARIES "-framework Foundation")
	set(OBJC_FLAGS "")
elseif (UNIX)
	find_program(GNUSTEP_CONFIG gnustep-config)
	if(NOT DEFINED GNUSTEP_CONFIG)
		message(FATAL_ERROR "Error: gnustep-config not found. exiting")
	endif()
	set(GC_ENV "OBJC_RUNTIME_LIB=ng RUNTIME_VERSION=gnustep")

	execute_process(COMMAND env ${GC_ENV} ${GNUSTEP_CONFIG} --base-libs
		OUTPUT_VARIABLE OBJC_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND env ${GC_ENV} ${GNUSTEP_CONFIG} --objc-flags
		OUTPUT_VARIABLE OBJC_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	foreach(VAR IN ITEMS OBJC_LIBS OBJC_FLAGS)
		string(REPLACE "-MMD -MP " "" ${VAR} ${${VAR}})
		string(REPLACE "-g -O2" "" ${VAR} ${${VAR}})
	endforeach ()
	set(CMAKE_OBJC_FLAGS "${OBJC_FLAGS}")
	set(CMAKE_OBJCXX_FLAGS "${OBJC_FLAGS}")
	set(OBJC_LIBS "${OBJC_LIBS} -lobjc")
	mark_as_advanced(GNUSTEP_CONFIG)
endif ()

#
# Dependencies
#

find_package(PkgConfig REQUIRED)

pkg_check_modules(DBUS REQUIRED IMPORTED_TARGET dbus-1)

add_subdirectory(launchctl2)
add_subdirectory(launchd2)
add_subdirectory(libxpc-dbus)
