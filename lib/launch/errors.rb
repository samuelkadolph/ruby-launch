module Launch
  class Error < StandardError
  end

  class JobError < Error
    attr_reader :job

    def initialize(job, message = nil)
      @job = job
      super(message)
    end
  end

  class InvalidJob < JobError
  end

  class JobAlreadyExists < JobError
  end

  class UnknownJob < JobError
  end
end
