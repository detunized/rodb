#!/usr/bin/env ruby

require 'rodb'
require 'test/unit'

class TestYaml2Rodb < Test::Unit::TestCase
	def test_loose_scalar
		assert_doesnt_compile "false"
		assert_doesnt_compile "true"
		assert_doesnt_compile "0"
		assert_doesnt_compile "0.0"
		assert_doesnt_compile "string"
	end

	def test_unsupported_type
		assert_doesnt_compile nil.to_yaml
		assert_doesnt_compile [nil].to_yaml
		assert_doesnt_compile [Time.now].to_yaml
		assert_doesnt_compile [Object.new].to_yaml
	end

	def test_invalid_yaml
		assert_doesnt_compile ""
		assert_doesnt_compile "]["
	end

	def test_blank_yaml
		assert_doesnt_compile "---\n"
	end

	def test_array
		assert_compiles "[]"
		assert_compiles "[0]"
		assert_compiles "[0, 1, 2, 3, 4]"
	end

	def test_map
		assert_compiles "{}"
		assert_compiles "{a: 0}"
		assert_compiles "{a: 0, b: 1, c: 2, d: 3, e: 4}"
	end

	def test_map_string_keys
		assert_doesnt_compile "{0: 0}"
		assert_doesnt_compile "{0: 0, 1: 1, 2: 2}" # all numbers
		assert_doesnt_compile "{0: 1, c: 2, e: f}" # mixed
	end

	# Helpers
private
	def assert_compiles(yaml)
		assert_nothing_raised { Rodb::compile yaml }
	end

	def assert_doesnt_compile(yaml)
		assert_raise(RuntimeError, ArgumentError) { Rodb::compile yaml }
	end
end
