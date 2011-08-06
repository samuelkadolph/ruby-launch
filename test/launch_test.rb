require "launch/test_helper"

describe Launch do
  describe "when listing jobs" do
    it "should return an array of Launch::Jobs" do
      jobs = Launch.jobs
      jobs.must_be_instance_of Array
      jobs.first.must_be_instance_of Launch::Job if jobs.first
    end
  end
end
