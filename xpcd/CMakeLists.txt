add_executable(xpcd job.cc main.cc)
target_link_libraries(xpcd OpenXPC ${KQUEUE_LIBS})
