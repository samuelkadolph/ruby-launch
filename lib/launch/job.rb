module Launch
  class Job
    require "launch/job/socket"

    class << self
      def checkin
        Launch.message(Messages::CHECKIN)
      end

      def find(label)
        job = allocate
        job.attributes = Launch.message(Messages::GETJOB => label)
        job
      rescue Errno::ESRCH
        raise "no job"
      end

      private
        @@default_attributes = {}
        def launch_attr(name, key, options = {})
          class_eval <<-EVAL unless options[:write_only]
            def #{name}
              attributes["#{key}"]
            end
          EVAL
          class_eval <<-EVAL unless options[:read_only]
            def #{name}=(value)
              attributes["#{key}"] = value
            end
          EVAL

          @@default_attributes[key] = options[:default] if options[:default]
          # TODO: options[:required]
        end
    end

    launch_attr :label, LABEL, :required => true
    launch_attr :on_demand, ONDEMAND, :default => true
    launch_attr :program, PROGRAM, :required => true

    def initialize(attributes = {})
      attributes.each do |key, value|
        send(:"#{key}=", value)
      end

      yield self if block_given?
    end

    def attributes
      @attributes ||= @@default_attributes.dup
    end

    def attributes=(attributes)
      @attributes = attributes.dup
    end

    def submit
      Launch.message(Messages::SUBMITJOB => attributes)
    rescue Errno::EEXIST
      raise "job exists already"
    end

    def remove
      Launch.message(Messages::REMOVEJOB => label)
    end
  end
end
