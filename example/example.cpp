#include "../rodb.h"

struct Point
{
    Point(int x, int y): x(x), y(y)
    {
    }

    Point(rodb::Value const &v): x(v["x"]), y(v["y"])
    {
    }

    int x;
    int y;
};

void print_layer(char const *layer_name)
{
    std::cout << "Layer: " << layer_name << "\n";
}

int main(int argc, char **argv)
{
    // Load database into memory. The file is not needed after this point.
    rodb::Database db("example.rodb");

    // Database::dump_yaml is helpful to see what's inside a compiled database.
    if (argc == 2 && strcmp(argv[1], "--dump") == 0)
    {
        db.dump_yaml(std::cout);
    }

    // It's ok to make copies of values, they are very cheap.
    rodb::Value root = db.root();

    // Walk though an array.
    rodb::Value layers = root["world"]["layers"];
    for (size_t i = 0; i < layers.size(); ++i)
    {
        print_layer(layers[i]);
    }

    // Map element access is O(log N). If you don't care too much about performance it's
    // ok to access same elements multiple times. It's fast enough. Otherwise it makes
    // sense to store root["ball"] and root["ball"]["start_position"] into a temporary
    // variable.
    Point p1(root["ball"]["start_position"]["x"], root["ball"]["start_position"]["y"]);
    Point p2(root["ball"]["start_position"]);
    assert(p1.x == p2.x && p1.y == p2.y);
    std::cout << "Point: {" << p1.x << ", " << p1.y << "}\n";

    // Boolean values.
    if (root["game"]["debug"])
    {
        std::cout << "Debug mode is on\n";
    }

    // Arrays don't have be uniform. They can contain mixed type values, including
    // other arrays and maps.
    rodb::Value allowed_levels = root["game"]["allowed_levels"];
    for (size_t i = 0; i < allowed_levels.size(); ++i)
    {
        if (allowed_levels[i].is_int())
        {
            std::cout << "Level #" << (int)allowed_levels[i] << " is allowed\n";
        }
        else if (allowed_levels[i].is_string())
        {
            std::cout << "Level '" << (char const *)allowed_levels[i] << "' is allowed\n";
        }
        else if (allowed_levels[i].is_array() && allowed_levels[i].size() > 0)
        {
            std::cout << "Levels ";

            rodb::Value group = allowed_levels[i];
            for (size_t j = 0; j < group.size(); ++j)
            {
                if (j == group.size() - 1)
                {
                    std::cout << " and ";
                }
                else if (j > 0)
                {
                    std::cout << ",";
                }

                std::cout << (int)group[j];
            }

            std::cout << " are allowed\n";
        }
    }

    return 0;
}
