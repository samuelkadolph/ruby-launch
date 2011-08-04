require "launch.so"

module Launch
  require "launch/version"
  require "launch/job"

  class << self
    def jobs
      message(Messages::GETJOBS).values
    end
  end
end
