# rodb

Rodb is a zero fragmentation micro database read-only access library. The
primary goal of rodb is to provide access to compiled config files in memory
restricted applications, like games. A very common problem in games is finding
an efficient and convenient way to store and access large number of different
config files, like level description, gameplay settings, effect configurations
and many others.

Rodb uses YAML as a source format. YAML files are converted to a binary blob
optimized for fairly efficient in-memory access. The blob is later loaded into a
single contiguous block of memory with only one read operation.

Rodb supports a limited subset of YAML types: UTF-8 strings, 32-bit integers,
single precision floats and booleans. Values of any of these types can be put
into arrays and maps (only string keys are allowed in maps). There's no
restriction on nesting of these data structures.

Since arrays can contain values of any types and lengths, they are implemented
with an additional table of offsets to elements. The access to elements is O(1).
Maps store their keys in a sorted array of strings, which allows for O(logN)
access.

Please note that rodb is not designed to provide access to gigabytes of data or
to churn millions of transaction per second. Its focus is simplicity and minimal
memory fragmentation. Some performance is sacrificed to achieve these goals.

## Config Example

```yaml
# World properties
world:
    layers:
        - ground
        - road
        - cracks

# Ball properties
ball:
    # Physics
    radius: 37.5
    start_position: {x: 160, y: -50}
    start_speed: 200

# Game settings
game:
    start_level: levels/woodbridge.rodb
    debug: true
    allowed_levels: [1, 2, levels/woodbridge.rodb, 7, 12, [100, 101, 102]]
```

_For complete example please check the example directory._

## Usage Example

```c++
// Load database into memory. The file is not needed after this point.
rodb::Database db("config.rodb");
rodb::Value root = db.root();

// Access some data
float x = root["ball"]["start_position"]["x"];
float y = root["ball"]["start_position"]["y"];

// Iterate over layers
rodb::Value layers = root["world"]["layers"];
for (size_t i = 0; i < layers.size(); ++i)
{
    // Do something with each layer
    print_layer(layers[i]);
}
```
_For complete example please check the example directory._

## License

The code is licensed under the terms of 
[MIT License](https://github.com/detunized/rodb/blob/master/LICENSE).