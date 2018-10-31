# Serialize
Header-only library to write and read file data as binary format

# Usage
```C++
#include "serialize.h"

struct Object {
    std::string name;
    std::vector<int> vec;
    float val;
    int i;
};

void serialize_object(Serializer& srl, Object& var, bool save) {
    serialize_string(srl, var.name, save);
    serialize_vector(srl, var.vec, save);
    serialize(srl, var.val, save);
    serialize(srl, var.i, save);
}

int main() {
    auto filename = "test.bin";
    auto object = Objec{"Hello, World!", {1,2,3,4}, 10.5f, 1}};
    
    // Let's save object into binary format
    auto writer = make_serializer("test.bin", true);
    serialize_object(srl, object, true);
    close_serializer(writer);
  
    // ... Program shuts down for whatever reason ...
    
    // Now let's reload object from disk
    Object object_reloaded;
    auto reader = make_serializer("test.bin", false);
    serialize_object(srl, object_read, false);
    close_serializer(reader);

    // Now we have object == object_reloaded
}
```

# Example
`example/example.cpp` shows the usage of the library.

Run it with `make`.

