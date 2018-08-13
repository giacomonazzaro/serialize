#include "../serialize.h"

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

static bool operator==(const Object& a, const Object& b) {
    return a.val == b.val and a.i == b.i;
}

void print(const Object& obj) {
    printf("name: %s\n", obj.name.c_str());
    printf("val: %lf\n", obj.val);
    printf("i: %d\n", obj.i);
}

template <typename Type>
void write_test(Type& x, const std::string& filename, size_t buffer_capacity) {
    bool save = true;
    auto writer = make_serializer("test.bin", save, 3);
    serialize_vector<Object>(writer, x, save, serialize_object);
    close_serializer(writer);   
}

template <typename Type>
void read_test(Type& x, const std::string& filename, size_t buffer_capacity) {
    bool save = false;
    auto reader = make_serializer(filename, save, buffer_capacity);
    Type y;
    serialize_vector<Object>(reader, y, save, serialize_object);
    assert(x == y);
    close_serializer(reader);
}

int main() {
    // Let's test the serialization.
    bool save = true;
    auto filename = "test.bin";
    size_t buffer_capacity = 3;
    auto object = std::vector<Object>{ {"Hello", {1,2,3,4}, 10.0, 1}, {"World", {7,7,7}, 20.0, 2} };
    write_test(object, filename, buffer_capacity);
    
    // Test reading with varius buffer capacities.
    for (int i = 0; i < 100; ++i)
        read_test(object, filename, i);

    printf("Test successful!\n");
}
