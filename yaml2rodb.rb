#!/usr/bin/env ruby

require 'yaml'

module Rodb
	class Compiler
		def compile(yaml)
			header + dump_value(load_yaml(yaml))
		end

	private
		VERSION = 1

		def header
			['rodb', VERSION].pack "a4V"
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

		# b - boolean
		# i - integer
		# f - floating point
		# s - stirng
		# a - array
		# m - map
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
				if not_a_string = value.keys.any? { |i| !i.is_a? String }
					raise "Map keys should be strings (#{not_a_string})"
				end

				sorted_keys, sorted_values = value.empty? ? [[], []] : value.sort.transpose
				keys = dump_value sorted_keys
				values = dump_value sorted_values
				dump_binary 'm', [value.length, keys.length].pack('V2') + keys + values
			else
				raise "Unsupported type #{value.class} (value: #{value})"
			end
		end

		def load_yaml(yaml)
			o = YAML::load yaml
			if o.is_a? Array or o.is_a? Hash
				o
			else
				raise "Root object must be either array or map (#{o.class}, #{o})"
			end
		end
	end

	def Rodb.compile(yaml)
		Compiler.new.compile yaml
	end
end

if $0 == __FILE__
	# TODO: Catch exceptions here!
	File.open ARGV[1], "wb" do |file|
		file.write Rodb::compile(File.read(ARGV[0]))
	end
end
