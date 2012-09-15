#!/usr/bin/env ruby

require File.join(File.dirname(__FILE__), 'rodb')

# TODO: Catch exceptions here and report errors to the user!
File.open ARGV[1], "wb" do |file|
	file.write Rodb::compile_file(ARGV[0])
end
