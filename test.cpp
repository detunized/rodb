#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE rodb
#include <boost/test/unit_test.hpp>

#include "rodb.h"

class WhatIs
{
public:
	WhatIs(char const *what): what_(what)
	{
	}

	bool operator ()(std::exception const &e)
	{
		return what_ == e.what();
	}

private:
	std::string what_;
};

class WhatStartsWith
{
public:
	WhatStartsWith(char const *what): what_(what)
	{
	}

	bool operator ()(std::exception const &e)
	{
		// Check that e.what() start with what_.  It's needed because e.what() might
		// have source file location information appended to it.
		// Example: Index is out of bounds in rodb.h, line 118
		return what_.compare(0, what_.size(), e.what(), 0, std::min(what_.size(), strlen(e.what()))) == 0;
	}

private:
	std::string what_;
};

#define YAML_FILENAME "unit_test.yaml"
#define RODB_FILENAME "unit_test.rodb"

char const *compile_rodb(char const *yaml)
{
	{
		std::ofstream out(YAML_FILENAME);
		out << yaml;
	}

	system("./yaml2rodb.rb " YAML_FILENAME " " RODB_FILENAME);

	return RODB_FILENAME;
}

#define DB(db, yaml) rodb::Database db(compile_rodb(yaml))

BOOST_AUTO_TEST_CASE(create_db)
{
	BOOST_CHECK(rodb::Database::load("") == 0);
	BOOST_REQUIRE_EXCEPTION(rodb::Database(""), std::runtime_error, WhatIs("Cannot open input file"));
}

BOOST_AUTO_TEST_CASE(scalar)
{
	DB(db, "[0]");

	BOOST_CHECK(db.root()[0].is_scalar());
	BOOST_CHECK(db.root()[0].size() == 1);
}

BOOST_AUTO_TEST_CASE(access_array)
{
	DB(db, "[0, 1, 2, 3, 4]");

	BOOST_CHECK(db.root().is_array());
	BOOST_CHECK(db.root().is_compound());

	BOOST_CHECK(db.root().size() == 5);

	BOOST_CHECK(db.root()[0] == 0);
	BOOST_CHECK(db.root()[1] == 1);
	BOOST_CHECK(db.root()[2] == 2);
	BOOST_CHECK(db.root()[3] == 3);
	BOOST_CHECK(db.root()[4] == 4);
	
	BOOST_REQUIRE_EXCEPTION(db.root()[-1], std::runtime_error, WhatStartsWith("Index is out of bounds"));
	BOOST_REQUIRE_EXCEPTION(db.root()[5], std::runtime_error, WhatStartsWith("Index is out of bounds"));
}

BOOST_AUTO_TEST_CASE(access_map)
{
	DB(db, "{key0: 0, key1: 1, key2: 2, key3: 3, key4: 4}");

	BOOST_CHECK(db.root().is_map());
	BOOST_CHECK(db.root().is_compound());

	BOOST_CHECK(db.root().size() == 5);

	BOOST_CHECK(db.root().keys().is_array());
	BOOST_CHECK(db.root().keys().size() == 5);
	
	BOOST_CHECK(strcmp(db.root().keys()[0], "key0") == 0);
	BOOST_CHECK(strcmp(db.root().keys()[1], "key1") == 0);
	BOOST_CHECK(strcmp(db.root().keys()[2], "key2") == 0);
	BOOST_CHECK(strcmp(db.root().keys()[3], "key3") == 0);
	BOOST_CHECK(strcmp(db.root().keys()[4], "key4") == 0);

	BOOST_CHECK(db.root().has_key("key0"));
	BOOST_CHECK(db.root().has_key("key1"));
	BOOST_CHECK(db.root().has_key("key2"));
	BOOST_CHECK(db.root().has_key("key3"));
	BOOST_CHECK(db.root().has_key("key4"));

	BOOST_CHECK(!db.root().has_key(""));
	BOOST_CHECK(!db.root().has_key("key5"));

	BOOST_CHECK(db.root().values().is_array());
	BOOST_CHECK(db.root().values().size() == 5);

	BOOST_CHECK(db.root().values()[0] == 0);
	BOOST_CHECK(db.root().values()[1] == 1);
	BOOST_CHECK(db.root().values()[2] == 2);
	BOOST_CHECK(db.root().values()[3] == 3);
	BOOST_CHECK(db.root().values()[4] == 4);

	BOOST_CHECK(db.root()["key0"] == 0);
	BOOST_CHECK(db.root()["key1"] == 1);
	BOOST_CHECK(db.root()["key2"] == 2);
	BOOST_CHECK(db.root()["key3"] == 3);
	BOOST_CHECK(db.root()["key4"] == 4);

	BOOST_REQUIRE_EXCEPTION(db.root()[""], std::runtime_error, WhatStartsWith("Key is not in the map"));
	BOOST_REQUIRE_EXCEPTION(db.root()["key5"], std::runtime_error, WhatStartsWith("Key is not in the map"));
}

BOOST_AUTO_TEST_CASE(map_sorted)
{
	DB(db, "{key4: 4, key0: 0, key2: 2, key1: 1, key3: 3}");
	
	BOOST_CHECK(strcmp(db.root().keys()[0], db.root().keys()[1]) <= 0);
	BOOST_CHECK(strcmp(db.root().keys()[1], db.root().keys()[2]) <= 0);
	BOOST_CHECK(strcmp(db.root().keys()[2], db.root().keys()[3]) <= 0);
	BOOST_CHECK(strcmp(db.root().keys()[3], db.root().keys()[4]) <= 0);

	BOOST_CHECK(db.root()["key0"] == 0);
	BOOST_CHECK(db.root()["key1"] == 1);
	BOOST_CHECK(db.root()["key2"] == 2);
	BOOST_CHECK(db.root()["key3"] == 3);
	BOOST_CHECK(db.root()["key4"] == 4);
}

BOOST_AUTO_TEST_CASE(numbers)
{
	DB(db, "[false, true, 0, 1000, 0x7fffffff, 0xffffffff, 0.0, 100.0, 1000.0, -10000.0]");

	// Bools
	BOOST_CHECK(db.root()[0].is_bool());
	BOOST_CHECK(db.root()[1].is_bool());

	BOOST_CHECK(!db.root()[0]);
	BOOST_CHECK(db.root()[1]);

	// Ints
	BOOST_CHECK(db.root()[2].is_int());
	BOOST_CHECK(db.root()[3].is_int());
	BOOST_CHECK(db.root()[4].is_int());
	BOOST_CHECK(db.root()[5].is_int());

	BOOST_CHECK(db.root()[2] == 0);
	BOOST_CHECK(db.root()[3] == 1000);
	BOOST_CHECK(db.root()[4] == 0x7fffffff);
	BOOST_CHECK(db.root()[5] == 0xffffffffu);

	// Floats
	BOOST_CHECK(db.root()[6].is_float());
	BOOST_CHECK(db.root()[7].is_float());
	BOOST_CHECK(db.root()[8].is_float());
	BOOST_CHECK(db.root()[9].is_float());
	
	BOOST_CHECK(db.root()[6] == 0.0f);
	BOOST_CHECK(db.root()[7] == 100.0f);
	BOOST_CHECK(db.root()[8] == 1000.0f);
	BOOST_CHECK(db.root()[9] == -10000.0f);
}

BOOST_AUTO_TEST_CASE(strings)
{
	DB(db, "[string0, string1, string2, string3]");

	BOOST_CHECK(db.root()[0].is_string());
	BOOST_CHECK(db.root()[1].is_string());
	BOOST_CHECK(db.root()[2].is_string());

	BOOST_CHECK(strcmp(db.root()[0], "string0") == 0);
	BOOST_CHECK(db.root()[0] == "string0");
	BOOST_CHECK("string0" == db.root()[0]);

	BOOST_CHECK(strcmp(db.root()[1], "string1") == 0);
	BOOST_CHECK(db.root()[1] == "string1");
	BOOST_CHECK("string1" == db.root()[1]);

	BOOST_CHECK(strcmp(db.root()[2], "string2") == 0);
	BOOST_CHECK(db.root()[2] == "string2");
	BOOST_CHECK("string2" == db.root()[2]);

	BOOST_CHECK(strcmp(db.root()[3], "string3") == 0);
	BOOST_CHECK(db.root()[3] == "string3");
	BOOST_CHECK("string3" == db.root()[3]);
}

BOOST_AUTO_TEST_CASE(nested_arrays)
{
	DB(db, "[[0], [1, 2], [3, 4, 5], [[0]], [[[0]]]]");

	BOOST_CHECK(db.root().is_array());
	BOOST_CHECK(db.root().size() == 5);

	BOOST_CHECK(db.root()[0].is_array());
	BOOST_CHECK(db.root()[0].size() == 1);

	BOOST_CHECK(db.root()[1].is_array());
	BOOST_CHECK(db.root()[1].size() == 2);

	BOOST_CHECK(db.root()[2].is_array());
	BOOST_CHECK(db.root()[2].size() == 3);

	BOOST_CHECK(db.root()[3].is_array());
	BOOST_CHECK(db.root()[3][0].is_array());
	BOOST_CHECK(db.root()[3][0].size() == 1);

	BOOST_CHECK(db.root()[4].is_array());
	BOOST_CHECK(db.root()[4][0].is_array());
	BOOST_CHECK(db.root()[4][0].size() == 1);
	BOOST_CHECK(db.root()[4][0][0].size() == 1);
}

BOOST_AUTO_TEST_CASE(dump_yaml)
{
	DB(db1, "[[0], [1, 2], [3, 4, 5], [[0]], [[[0]]]]");

	std::ostringstream stream;
	db1.dump_yaml(stream);

	DB(db2, stream.str().c_str());
	BOOST_CHECK(db2.root().size() == 5);
	BOOST_CHECK(db2.root()[4][0][0][0] == 0);
}

BOOST_AUTO_TEST_CASE(comparison)
{
	DB(db, "[10, 10, [10], [10], [10, 20], [20, 10], {a: b, cc: dd}, {cc: dd, a: b}, {a: dd, cc: b}, {cc: dd, a: b}]");

	BOOST_CHECK(db.root()[0] == db.root()[1]);
	BOOST_CHECK(db.root()[2] == db.root()[3]);
	BOOST_CHECK(db.root()[4] != db.root()[5]);
	BOOST_CHECK(db.root()[6] == db.root()[7]);
	BOOST_CHECK(db.root()[8] != db.root()[9]);
	BOOST_CHECK(db.root()[0] != db.root()[2]);
	BOOST_CHECK(db.root()[1] != db.root()[6]);
	BOOST_CHECK(db.root()[4] != db.root()[9]);
}

BOOST_AUTO_TEST_CASE(direct_database_root_array_access)
{
	DB(db, "[0, 1, 2, 3, 4]");

	BOOST_CHECK(db[0] == 0);
	BOOST_CHECK(db[1] == 1);
	BOOST_CHECK(db[2] == 2);
	BOOST_CHECK(db[3] == 3);
	BOOST_CHECK(db[4] == 4);

	BOOST_CHECK(db[0] == db.root()[0]);
	BOOST_CHECK(db[1] == db.root()[1]);
	BOOST_CHECK(db[2] == db.root()[2]);
	BOOST_CHECK(db[3] == db.root()[3]);
	BOOST_CHECK(db[4] == db.root()[4]);
}

BOOST_AUTO_TEST_CASE(direct_database_root_map_access)
{
	DB(db, "{key0: 0, key1: 1, key2: 2, key3: 3, key4: 4}");
	
	BOOST_CHECK(db["key0"] == 0);
	BOOST_CHECK(db["key1"] == 1);
	BOOST_CHECK(db["key2"] == 2);
	BOOST_CHECK(db["key3"] == 3);
	BOOST_CHECK(db["key4"] == 4);

	BOOST_CHECK(db["key0"] == db.root()["key0"]);
	BOOST_CHECK(db["key1"] == db.root()["key1"]);
	BOOST_CHECK(db["key2"] == db.root()["key2"]);
	BOOST_CHECK(db["key3"] == db.root()["key3"]);
	BOOST_CHECK(db["key4"] == db.root()["key4"]);
}
