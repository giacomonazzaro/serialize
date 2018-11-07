# Serialize

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

# Example
`example/test.cpp` tests the library and shows its usage on a sample data structure.

Run it with `make`.

