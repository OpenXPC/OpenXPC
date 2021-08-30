add_library(sbuf STATIC sbuf.c)
target_include_directories(sbuf
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
