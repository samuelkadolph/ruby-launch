require "launch/test_helper"

describe "launch_service" do
  before do
    @job = Launch::Job.new
    @job.label = "org.launch.test"
    @job.environment_variables = ENV.to_hash
    @job.program_arguments << Gem.ruby
    @job.program_arguments << "-e"
    @job.program_arguments << <<-PROGRAM
      requrie "rubygems"
      require "launch"


    PROGRAM
  end

  describe "when submitting job" do
    it "should return true" do
      lambda { Launch::Job.checkin }.must_raise(Launch::JobError)
    end
  end
end
