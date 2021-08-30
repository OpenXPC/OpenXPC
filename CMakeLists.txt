cmake_minimum_required(VERSION 3.15)
project(LaunchD-Reloaded LANGUAGES C CXX OBJC OBJCXX)

include(CheckIncludeFiles)

set(GATE_NAME "launchd_reloaded")
set(GATE_VERSION "1-devel")
set(GATE_STRING "${GATE_NAME}-${GATE_VERSION}")

set(LAUNCHD2_NAME "LaunchdD-Reloaded Bootstrap Service")
set(LAUNCHD2_VERSION "0.9-devel")
set(LAUNCHD2_STRING "${LAUNCHD2_NAME} version ${LAUNCHD2_VERSION}")

execute_process(COMMAND date
	OUTPUT_VARIABLE BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# ObjC setup
#

set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -fexceptions -fobjc-exceptions \
	-fblocks -Xclang -fblocks -Wno-nullability-completeness")
set(CMAKE_OBJCXX_FLAGS "${CMAKE_OBJCXX_FLAGS} ${CMAKE_OBJC_FLAGS}")

if (APPLE)
	set(OBJC_LIBS "-framework Foundation")
	set(OBJC_FLAGS "")
elseif (UNIX)
	find_program(GNUSTEP_CONFIG gnustep-config)
	if(NOT DEFINED GNUSTEP_CONFIG)
		message(FATAL_ERROR "Error: gnustep-config not found. exiting")
	endif()
	set(GC_ENV "OBJC_RUNTIME_LIB=ng RUNTIME_VERSION=gnustep-2.0")

	execute_process(COMMAND env ${GC_ENV} ${GNUSTEP_CONFIG} --base-libs
		OUTPUT_VARIABLE OBJC_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND env ${GC_ENV} ${GNUSTEP_CONFIG} --objc-flags
		OUTPUT_VARIABLE OBJC_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	foreach(VAR IN ITEMS OBJC_LIBS OBJC_FLAGS)
		string(REPLACE "-MMD -MP " "" ${VAR} ${${VAR}})
		string(REPLACE "-g -O2" "" ${VAR} ${${VAR}})
	endforeach ()
	set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -fobjc-runtime=gnustep-2.0")
	set(CMAKE_OBJCXX_FLAGS "${CMAKE_OBJC_FLAGS} -fobjc-runtime=gnustep-2.0")
	set(OBJC_LIBS "${OBJC_LIBS} -lobjc -fobjc-runtime=gnustep-2.0")
	mark_as_advanced(GNUSTEP_CONFIG)
endif ()

#
# System compatibility
#

if (CMAKE_SYSTEM_NAME MATCHES ".*Linux")
	set(PLATFORM "GNU/Linux")
	set(PLATFORM_GNULinux TRUE)
elseif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	set(PLATFORM "FreeBSD")
	set(PLATFORM_FreeBSD TRUE)
	set(PLATFORM_BSD TRUE)
	set(HAVE_sys_sbuf_h TRUE)
elseif (APPLE)
	set(PLATFORM "MacOSX")
	set(PLATFORM_MacOSX TRUE)
	set(PLATFORM_BSD TRUE)
else ()
	error("Unsupported Platform ${CMAKE_SYSTEM_NAME}")
endif ()

#check_include_files("sys/types.h;sys/sbuf.h" HAVE_sys_sbuf_h) # hassle on macOS
check_include_files("dispatch/dispatch.h" HAVE_dispatch)

if (NOT HAVE_sys_sbuf_h)
	add_subdirectory(libsbuf)
endif ()

#
# Dependencies
#

find_package(PkgConfig REQUIRED)

pkg_check_modules(DBUS REQUIRED IMPORTED_TARGET dbus-1)

if (CMAKE_SYSTEM_NAME MATCHES ".*Linux")
	pkg_check_modules(KQUEUE REQUIRED IMPORTED_TARGET libkqueue)
	set(KQUEUE_LIBS PkgConfig::KQUEUE)
	pkg_check_modules(NVP REQUIRED IMPORTED_TARGET nvp)
	set(NVP_LIBS PkgConfig::NVP)
endif ()

add_subdirectory(launchctl2)
add_subdirectory(launchd2)
add_subdirectory(libxpc-dbus)

configure_file(libxpc-dbus/xpc2/config.h.in libxpc-dbus/xpc2/config.h)
