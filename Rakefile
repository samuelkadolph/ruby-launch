# -*- ruby -*-

task :default => :compile

require 'rubygems'
require 'hoe'

Hoe.plugin :minitest
Hoe.plugin :git

hoe = Hoe.spec 'launch' do
  developer 'Eric Hodel', 'drbrain@segment7.net'

  extra_dev_deps << ['rake-compiler', '~> 0.7']
  self.spec_extras[:extensions] = %w[ext/launch/extconf.rb]

  self.spec_extras[:required_ruby_version] = '>= 1.9.2'

  self.clean_globs = %[
    lib/launch/launch.bundle
  ]
end

gem 'rake-compiler', '~> 0.7'
require 'rake/extensiontask'

Rake::ExtensionTask.new hoe.name, hoe.spec do |ext|
  ext.lib_dir = File.join 'lib', 'launch'
end

# vim: syntax=ruby
