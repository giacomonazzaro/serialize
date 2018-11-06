#include "../serialize.h"

struct Object {
    std::string name;
    std::vector<int> vec;
    int i;
    float val;
};

void serialize_object(Serializer& srl, Object& var, bool save) {
    serialize(srl, var.i, save);
    serialize(srl, var.val, save);
    serialize_string(srl, var.name, save);
    serialize_vector(srl, var.vec, save);
}

int main() {
    auto filename = "test.bin";
    auto object = Object{"Hello", {1,2,3,4}, 1, 10.0};
    Object object_reloaded;
    
    int buffer_capacity = 0;
    auto writer = make_serializer(filename, true, buffer_capacity);
    serialize_object(writer, object, true);
    close_serializer(writer);

    auto reader = make_serializer(filename, false, buffer_capacity);
    serialize_object(reader, object_reloaded, false);
    close_serializer(reader);
    
    assert(object.name == object_reloaded.name);
    assert(object.vec == object_reloaded.vec);
    assert(object.val == object_reloaded.val);
    assert(object.i == object_reloaded.i);

    printf("Test successful!\n");
}
