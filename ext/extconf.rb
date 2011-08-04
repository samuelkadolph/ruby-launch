require "mkmf"

abort "errno.h is required" unless have_header("errno.h")
abort "launch.h is required" unless have_header("launch.h")

abort "launch_data_free() is required" unless have_func("launch_data_free")

create_makefile "launch"
