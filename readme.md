# Serialize
Header-only library to write/read to/from disk in binary format.  
This is useful to save on disk the state of a program and reload it any other moment.  
The libray provides the basic tools to easily define the serialization of custom data structures.  
In particular, `serialize.h` has the following built-in serializtion function
    * `serialize()`: write/read any POD (struct with no allocated resources)
    * `serialize_string()`: write/read a `std::string`
    * `serialize_vector()`: write/read a `std::vector` of PODs
    * `serialize_vector()`: write/read a `std::vector` of structs with custom `serialize` function

# Example
`example/test.cpp` tests the library and shows its usage on a sample data structure.

Run it with `make`.

# Usage
```C++
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


