# launch

launch allows you to submit, remove, start, stop and list jobs from the launchd
as well as checkin from a process that was started by launchd and retrieve the
sockets for the job. Launchd is an open source framework for launching and
managing daemons, programs and scripts provided by Apple.

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

To create a job.

```ruby
require "launch"

job = Launch::Job.new
job.label = "org.ruby.foo"
job.program = "/usr/bin/foo"
job.submit
```

To run a job.

```ruby
require "launch"

job = Launch::Job.find("org.ruby.foo")
```

## Requirements

A system with launchd installed.
