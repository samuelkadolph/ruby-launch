require "socket"
require "launch.so"

module Launch
  require "launch/version"
  require "launch/job"

  class << self
    def jobs
      message(Messages::GETJOBS).values.map(&Job.method(:from_launch))
    end
  end
end
