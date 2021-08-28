add_executable(launchd2 job.cc main.cc)
target_link_libraries(launchd2 xpc-dbus ${KQUEUE_LIBS})
