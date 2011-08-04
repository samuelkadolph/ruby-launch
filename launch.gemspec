$:.push File.expand_path("../lib", __FILE__)
require "launch/version"

Gem::Specification.new do |s|
  s.name        = "launch"
  s.version     = Launch::VERSION
  s.authors     = ["Eric Hodel", "Samuel Kadolph"]
  s.email       = ["drbrain@segment7.net", "samuel@kadolph.com"]
  s.homepage    = "https://github.com/samuelkadolph/ruby-launch"
  s.summary     = %q{launch is a wrapper around launchd which allows you to create jobs and checkin to get the sockets.}
  s.description = %q{launch allows you to submit, remove, start, stop and list jobs from the launchd as well as checkin } +
                  %q{from a process that was started by launchd and retrieve the sockets for the job.}

  s.required_ruby_version = ">= 1.8.7"

  s.files      = Dir["ext/*", "lib/**/*"] + ["LICENSE", "README.md"]
  s.test_files = Dir["test/**/*_test.rb"]
  s.extensions = ["ext/extconf.rb"]
end
