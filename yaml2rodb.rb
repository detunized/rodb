#!/usr/bin/env ruby

require 'yaml'
require 'zlib'

RODB_VERSION = 1

# b - boolean
# i - integer
# f - floating point
# s - stirng
# a - array
# m - map

def header
	['rodb', RODB_VERSION].pack "a4V"
end

def dump_binary(type, payload)
	[type, payload.length].pack('a4V') + payload
end

def offsets(items)
	sum = 0
	offsets = []
	items.map { |i| i.length }.each_with_index do |i, index|
		offsets << sum
		sum += i
	end

	offsets
end

def dump_value(value)
	case value
	when FalseClass
		dump_binary 'b', [0].pack('V')
	when TrueClass
		dump_binary 'b', [1].pack('V')
	when Fixnum
		dump_binary 'i', [value].pack('V')
	when Float
		dump_binary 'f', [value].pack('e')
	when String
		dump_binary 's', [value].pack('Z*')
	when Array
		items = value.map { |i| dump_value i }
		dump_binary 'a', [value.length].pack('V') + offsets(items).pack('V*') + items.join
	when Hash
		sorted_keys, sorted_values = value.sort.transpose
		keys = dump_value sorted_keys
		values = dump_value sorted_values
		dump_binary 'm', [value.length, keys.length].pack('V2') + keys + values
	else
		raise "Unsupported type #{value.class} (value: #{value})"
	end
end

value = nil

begin
	value = YAML::load_file(ARGV[0])
rescue
	puts "Failed to load #{ARGV[0]} (#{$!})"
	exit
end

packed_header = header
packed_value = dump_value value
packed = packed_header + packed_value
zipped = Zlib::Deflate.deflate packed, 9

#puts "Yaml length: #{yaml.length}"
#puts "Packed length: #{packed.length}"
#puts "Zipped length: #{zipped.length}"

#p packed_value

File.open ARGV[1], "wb" do |file|
	file.write packed
end
