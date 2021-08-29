set(LAUNCHCTL_PATH ${CMAKE_INSTALL_BINDIR}/launchctl2)

add_executable(launchctl2 Main.m XPC.m)
target_link_libraries(launchctl2 ${OBJC_LIBS} xpc-dbus)

install(TARGETS launchctl2 DESTINATION ${CMAKE_INSTALL_BINDIR})
