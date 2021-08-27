add_library(xpc-dbus log.c xpc.c)
target_link_libraries(xpc-dbus PkgConfig::DBUS)
target_include_directories(xpc-dbus
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
