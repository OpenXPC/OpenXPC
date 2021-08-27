add_executable(launchd2 exec.cc main.cc)
target_link_libraries(launchd2 xpc-dbus)
