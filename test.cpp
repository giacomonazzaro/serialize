#include "serialize.h"

struct Prova {
    std::string name;
    float val;
    int i;
};

void serialize(Serializer& srl, Prova& var, bool save) {
    serialize_string(srl, var.name, save);
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

int main() {
    bool save = true;
    auto x0 = std::vector<Prova>{ Prova{"ciao", 10.0, 1}, Prova{"bello", 20.0, 2} };
    auto writer = make_serializer("test.bin", save, 3);
    serialize_vector(writer, x0, save);
    close_serializer(writer);

    save = false;
    for(int i = 0; i<30; i++) {
        auto reader = make_serializer("test.bin", save, i);
        std::vector<Prova> y0;
        serialize_vector(reader, y0, save);
        assert(x0 == y0);
        close_serializer(reader);
    }
    printf("Test successful!\n");
}
