=begin
  Echo server using launchd to manage a socket opened on port 12345.

  Usage:
    gem install launch
    ruby echo.rb
=end

require "rubygems"

begin
  gem "launch"
rescue Gem::LoadError
  raise "You must install the launch gem to use the samples."
end

require "launch"

job = Launch::Job.new(:label => "org.ruby.echo")
job.program_arguments << Gem.ruby
job.program_arguments << "-e"
job.program_arguments << <<-PROGRAM
  require "rubygems"
  require "launch"

  sockets = Launch::Job.checkin.sockets["echo"]
  loop do
    ready = select(sockets).first
    ready.each do |socket|
      Thread.start(socket.accept) do |client|
        loop { client << client.gets }
      end
    end
  end
PROGRAM
job.environment_variables = ENV.to_hash
job.sockets["echo"] = TCPServer.new(12345)

begin
  job.submit

  $stdout.puts "Echo server started on port 12345."
  $stdout.puts "Press ^C or enter to shut down the server."
  $stdin.gets
rescue Interrupt
  $stdout.puts
ensure
  job.remove
end
