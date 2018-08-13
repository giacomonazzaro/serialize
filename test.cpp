#include "serialize.h"

struct Prova {
    std::string name;
    std::vector<int> vec;
    float val;
    int i;
};

void serialize_prova(Serializer& srl, Prova& var, bool save) {
    serialize_string(srl, var.name, save);
    serialize_vector(srl, var.vec, save);
    serialize(srl, var.val, save);
    serialize(srl, var.i, save);
}

static bool operator==(const Prova& a, const Prova& b) {
    return a.val == b.val and a.i == b.i;
}

void print(Prova p) {
    printf("name: %s\n", p.name.c_str());
    printf("val: %lf\n", p.val);
    printf("i: %d\n", p.i);
}

template <typename Type>
void write_test(Type& x, const std::string& filename, size_t buffer_capacity) {
    bool save = true;
    auto writer = make_serializer("test.bin", save, 3);
    serialize_vector<Prova>(writer, x, save, serialize_prova);
    close_serializer(writer);   
}

template <typename Type>
void read_test(Type& x, const std::string& filename, size_t buffer_capacity) {
    bool save = false;
    auto reader = make_serializer(filename, save, buffer_capacity);
    Type y;
    serialize_vector<Prova>(reader, y, save, serialize_prova);
    assert(x == y);
    close_serializer(reader);
}


int main() {
    bool save = true;
    auto filename = "test.bin";
    auto x0 = std::vector<Prova>{ Prova{"ciao",{1,2,3,4}, 10.0, 1}, Prova{"bello", {7,7,7}, 20.0, 2} };
    write_test(x0, filename, 3);
    
    for (int i = 0; i < 100; ++i)
        read_test(x0, filename, i);

    printf("Test successful!\n");
}
