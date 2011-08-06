$:.push File.expand_path("../lib", __FILE__)
require "launch/version"

Gem::Specification.new do |s|
  s.name        = "launch"
  s.version     = Launch::VERSION
  s.authors     = ["Eric Hodel", "Samuel Kadolph"]
  s.email       = ["drbrain@segment7.net", "samuel@kadolph.com"]
  s.homepage    = "https://github.com/samuelkadolph/ruby-launch"
  s.summary     = %q{Wrapper for liblaunch which allows you to manage jobs and checkin from processes spawned by launchd.}
  s.description = <<-DESC
launch is a wrapper for liblaunch which lets you load, unload, reload, submit, remove, start, stop and list jobs for
launchd as well as checkin from a process spawned by launchd and retrieve the sockets for the job that spawned the process.
DESC

  s.required_ruby_version = ">= 1.8.7"

  s.files      = Dir["ext/*", "lib/**/*"] + ["CHANGELOG.md", "LICENSE", "README.md"]
  s.test_files = Dir["test/**/*_test.rb"]
  s.extensions = ["ext/extconf.rb"]
end
