require "rubygems"

begin
  gem "launch"
rescue Gem::LoadError
  raise "You must install the launch gem to use the test helper."
end

require "launch"
require "minitest/autorun"
require "minitest/spec"
