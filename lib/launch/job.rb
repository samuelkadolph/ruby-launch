module Launch
  require "launch/errors"

  class Job
    class << self
      def checkin
        from_launch Launch.message(Messages::CHECKIN)
      rescue Errno::EPERM
        raise JobError.new(nil, "must checkin from process spawned by lauchd for job with sockets")
      end

      def find(label)
        from_launch Launch.message(Messages::GETJOB => label)
      rescue Errno::ESRCH
        raise UnknownJob.new(nil, "no job exists with the label `#{label}'")
      end

      private
        def from_launch(launch)
          job = allocate
          job.launch_attributes = launch
          job
        end

        @@launch_attributes = Hash.new { |h, k| h[k] = { :lazy_value => lambda { nil } } }
        def launch_attr(name, launch_key, options = {}, &block)
          @@launch_attributes[launch_key][:name] = name
          @@launch_attributes[launch_key][:lazy_value] = block if block

          class_eval <<-EVAL, __FILE__, __LINE__ + 1 unless options[:write_only]
            def #{name}
              unless @launch_attributes.key?("#{launch_key}")
                @launch_attributes["#{launch_key}"] = @@launch_attributes["#{launch_key}"][:lazy_value].call
              end

              @launch_attributes["#{launch_key}"]
            end
          EVAL
          class_eval <<-EVAL, __FILE__, __LINE__ + 1 unless options[:read_only]
            def #{name}=(value)
              @launch_attributes["#{launch_key}"] = value
            end
          EVAL
        end
    end

    launch_attr :environment_variables, ENVIRONMENTVARIABLES do
      Hash.new
    end
    launch_attr :label, LABEL
    launch_attr :on_demand, ONDEMAND
    launch_attr :program, PROGRAM
    launch_attr :program_arguments, PROGRAMARGUMENTS do
      []
    end
    launch_attr :sockets, SOCKETS do
      Hash.new { |h, k| h[k] = [] }
    end

    def attributes=(attributes)
      attributes.each do |key, value|
        send(:"#{key}=", value)
      end
    end

    def initialize(attributes = {})
      @launch_attributes = {}

      self.attributes = attributes

      yield self if block_given?
    end

    def inspect
      "#<#{self.class} #{attributes_inspect}>"
    end

    def launch_attributes
      @launch_attributes
    end

    def launch_attributes=(attributes)
      @launch_attributes = attributes
    end

    def remove
      message(Messages::REMOVEJOB => label)
    end

    def start
      message(Messages::STARTJOB => label)
    end

    def stop
      message(Messages::STOPJOB => label)
    end

    def submit
      message(Messages::SUBMITJOB => launch_attributes)
    end

    private
      def attributes_inspect
        @@launch_attributes.select do |launch_key, attribute|
          @launch_attributes.key?(launch_key)
        end.map do |launch_key, attribute|
          "#{attribute[:name]}: #{@launch_attributes[launch_key].inspect}"
        end.join(", ")
      end

      def message(msg)
        Launch.message(msg)
      rescue Errno::ESRCH
        raise UnknownJob.new(self, "no job exists with the label `#{label}'")
      rescue Errno::EINVAL
        raise InvalidJob.new(self, "job is invalid")
      rescue Errno::EEXIST
        raise JobAlreadyExists.new(self, "job with label `#{label}' already exists")
      rescue SystemCallError => e
        raise JobError.new(self, e.message)
      end
  end
end
