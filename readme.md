# Serialize
Header-only library for binary serialization of data structures and fast loading and writing of the state of a program.  
In `serialize.h` are provided the basic tools to easily define the serialization code of custom data structures.  
In order to minimize disk access, a memory buffer is used during serialization.  
The library features the following built-in serializtion functions:
  * `serialize()`: write/read any POD (struct with no allocated resources).
  * `serialize_string()`: write/read a `std::string`.
  * `serialize_vector()`: write/read a `std::vector` of PODs.
  * `serialize_vector()`: write/read a `std::vector` of structs with custom serialization function.

# Example
In `example/test.cpp` the library is tested, showing its usage on a toy data structure.  
Build it with `build.sh`.

```C++
#include "serialize.h"

struct Object {
    std::string name;
    std::vector<int> vec;
    int i;
    float val;
};

void serialize_object(Serializer& srl, Object& var) {
    serialize(srl, var.i);
    serialize(srl, var.val);
    serialize_string(srl, var.name);
    serialize_vector(srl, var.vec);
}

int main() {
    Object object = Object{"Hello", {1,2,3,4}, 77, 12.0};
    std::string filename = "object.bin";
    int capacity = 64;
    
    // Let's save object into binary a binary file.
    auto writer = make_writer(filename, capacity);
    serialize_object(writer, object);
    close_serializer(writer);

    // Now, let's reload object from disk.
    Object object_reloaded;
    auto reader = make_reader(filename, capacity);
    serialize_object(reader, object_reloaded);
    close_serializer(reader);

    // object == object_reloaded
}
```


