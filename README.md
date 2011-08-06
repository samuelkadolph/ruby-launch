# launch

launch is a wrapper for launchd which lets you load, unload, reload, submit, remove, start, stop and list jobs for
launchd as well as checkin from a process spawned by launchd and retrieve the sockets for the job that spawned the process.

launchd is an open source framework for launching and managing daemons, programs and scripts provided by Apple.

## Installing

### Recommended

```
gem install launch
```

### Edge

```
git clone https://github.com/samuelkadolph/ruby-launch
cd ruby-launch && rake install
```

## Usage

Submit and remove jobs.

```ruby
require "launch"

job = Launch::Job.new
job.label = "org.ruby.foo"
job.program = "/usr/bin/foo"
job.submit # => true

job.remove # => true
```

Start and stop jobs.

```ruby
require "launch"

job = Launch::Job.find("org.ruby.foo")
job.start # => true
sleep 5
job.stop # => true
```

## Requirements

A system with launchd installed.

## Developers

### Contributing

Pull requests are the desired method of contributing.

### Running Tests

```
bundle install
rake install
rake test
```
