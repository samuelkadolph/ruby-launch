require "launch/test_helper"

describe Launch::Job do
  describe "when checking in from a process not spawned by launchd" do
    it "should raise an error" do
      lambda { Launch::Job.checkin }.must_raise(Launch::JobError)
    end
  end
end
