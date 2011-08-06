require "mkmf"

abort "errno.h is required" unless have_header("errno.h")
abort "launch.h is required" unless have_header("launch.h")

unless have_header("ruby/io.h") or have_header("rubyio.h")
  abort "ruby/io.h or rubyio.h is required"
end

unless have_struct_member("rb_io_t", "fd", "ruby/io.h") or have_struct_member("rb_io_t", "f", %w[ruby.h rubyio.h])
  abort "rb_io_t.fd or rb_io_t.f is required"
end

# $defs << "-HAVE_RUBY_19" if RUBY_VERSION >= "1.9"
# $CFLAGS << " -Werror"

create_makefile "launch"
