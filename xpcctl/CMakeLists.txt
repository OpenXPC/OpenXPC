set(LAUNCHCTL_PATH ${CMAKE_INSTALL_BINDIR}/xpcctl)

add_executable(xpcctl Main.m XPC.m)
target_link_libraries(xpcctl ${OBJC_LIBS} OpenXPC)

install(TARGETS xpcctl DESTINATION ${CMAKE_INSTALL_BINDIR})
