#include "../serialize.h"

struct Object {
    std::string name;
    std::vector<int> vec;
    int i;
    float val;
};

void serialize_object(Serializer& srl, Object& var) {
    serialize(srl, var.i);
    // fclose(srl.file);
    // srl.file = fopen("output/test.bin", "a");
    serialize(srl, var.val);
    // fclose(srl.file);
    // srl.file = fopen("output/test.bin", "a");
    serialize_string(srl, var.name);
    // fclose(srl.file);
    // srl.file = fopen("output/test.bin", "a");
    serialize_vector(srl, var.vec);
}

void test_serialization(std::string filename, int writer_capacity, int reader_capacity) {
    Object object = Object{"Hello", {1,2,3,4}, 77, 12.0};
    
    // Let's save object into binary a binary file.
    auto writer = make_writer(filename, writer_capacity);
    serialize_object(writer, object);
    close_serializer(writer);

    // Now, let's reload object from disk.
    Object object_reloaded;
    auto reader = make_reader(filename, reader_capacity);
    serialize_object(reader, object_reloaded);
    close_serializer(reader);
    
    // Proof that saving/loading works properly.
    assert(object.i == object_reloaded.i);
    assert(object.val == object_reloaded.val);
    assert(object.name == object_reloaded.name);
    assert(object.vec == object_reloaded.vec);
}

int main() {
    auto filename = "output/object.bin";

    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 100; j++)
            test_serialization(filename, i, j);

    printf("Test successful!\n");
}
