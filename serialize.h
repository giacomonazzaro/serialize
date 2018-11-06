#include <cassert>
#include <vector>
#include <string>
#include <functional>

#define SERIALIZER_VERSION 0;

// Serializer holds the information needed to serialize data into file in binary format.
// To minimize disk access, a memory buffer is used to store temporary result. When full, 
// the buffer is dumped into file and cleared.
struct Serializer {
    FILE* file = nullptr;
    int version = SERIALIZER_VERSION;
    bool save;

    unsigned char* buffer = nullptr;
    size_t buffer_capacity = 0;
    size_t buffer_count = 0;

    ~Serializer() {
        if(file or buffer)
            assert(0 && "Close serializer before destruction!");
    } 
};

void serialize_error(const std::string& message) {
    printf("\n*** SERIALIZE ERROR ***\n");
    printf("message: %s\n", message.c_str());
    printf("\n");
    abort();
}

Serializer make_serializer(const std::string& filename, bool save, size_t buffer_capacity) {
    Serializer srl;
    srl.file = fopen(filename.c_str(), save? "w" : "r");
    if (not srl.file)
        serialize_error("could not open file " + filename);

    srl.buffer_capacity = buffer_capacity;
    if(srl.buffer_capacity > 0) {
        srl.buffer = (unsigned char*) malloc(buffer_capacity);
        if(not srl.buffer) serialize_error("could not allocate buffer for file " + filename);
    }
    else
        srl.buffer = nullptr;

//    if(save) fwrite(&srl.version, 1, sizeof(int), srl.file);
//    else      fread(&srl.version, 1, sizeof(int), srl.file);

    srl.save = save;
    if(not save)
        fread(srl.buffer, srl.buffer_capacity, 1, srl.file);

    return srl;
}

Serializer make_reader(const std::string& filename, size_t buffer_capacity = 0) {
    return make_serializer(filename, false, buffer_capacity);
}

Serializer make_writer(const std::string& filename, size_t buffer_capacity = 0) {
    return make_serializer(filename, true, buffer_capacity);
}




void reload_buffer(Serializer& srl) {
    assert(not srl.save);
    if(srl.buffer_capacity == 0) return;
    fread(srl.buffer, srl.buffer_capacity, 1, srl.file);
    srl.buffer_count = 0;
}

void dump_buffer(Serializer& srl) {
    assert(srl.save);
    if(srl.buffer_capacity == 0) return;
    fwrite(srl.buffer, srl.buffer_count, 1, srl.file);
    srl.buffer_count = 0;
}

void refresh_buffer(Serializer& srl) {
    if(srl.buffer_capacity == 0) return;
    assert(srl.buffer_count == srl.buffer_capacity);
    if(srl.save) fwrite(srl.buffer, srl.buffer_capacity, 1, srl.file);
    else          fread(srl.buffer, srl.buffer_capacity, 1, srl.file);
    srl.buffer_count = 0;
}


void close_serializer(Serializer& srl) {
    if(srl.buffer) {
        if(srl.save) dump_buffer(srl);
        free(srl.buffer);
        srl.buffer = nullptr;
        srl.buffer_capacity = 0;
        srl.buffer_count = 0;
    }
    if(srl.file) fclose(srl.file);
    srl.file = nullptr;
}



// write and read function are different from the standard fwrite and fread.
// A memory buffer is used to miminize disk access.

// void write(Serializer& srl, void* data, size_t size) {
//     // If buffer capacity is not enough, dump buffer to file and clear it.
//     if(size >= srl.buffer_capacity - srl.buffer_count) {
//         int count = srl.buffer_capacity - srl.buffer_count;
//         memcpy(srl.buffer + srl.buffer_count, data, count);
//         data = (unsigned char*) data + count;
//         size -= count;
//         dump_buffer(srl);

//         // If, the rest is too big, don't use buffer
//         if(size >= srl.buffer_capacity) {
//             fwrite(data , size, 1, srl.file);
//             size = 0;
//         }
//     }
//     if(size == 0) return;

//     memcpy(srl.buffer + srl.buffer_count, data, size);
//     srl.buffer_count += size;
// }


// void read(Serializer& srl, void* data, size_t size) {
//     assert(size > 0);

//     // Read from buffer remaining first part of data.
//     if(size >= srl.buffer_capacity - srl.buffer_count) {
//         int count = srl.buffer_capacity - srl.buffer_count;
//         memcpy(data, srl.buffer + srl.buffer_count, count);
//         data = (unsigned char*) data + count;
//         size -= count;
//         reload_buffer(srl);

//         // If, the rest is too big, don't use buffer
//         if(size >= srl.buffer_capacity) {
//             fread(data , size, 1, srl.file);
//             size = 0;
//         }
//     }
//     if(size == 0) return;

//     memcpy(data, srl.buffer + srl.buffer_count, size);
//     srl.buffer_count += size;
// }

// void buffer_serialize(void* from, void* to, size_t size, bool save) {
//     if(size == 0) return;
//     assert(from);
//     assert(to);
    
//     //void* memcpy(void* destination, const void* source, size_t num);
//     if(save) memcpy(to, from, size);
//     else     memcpy(from, to, size);
// }

void buffer_serialize(Serializer& srl, void* data, size_t size) {
    if(srl.buffer_capacity == 0) return;
    if(size == 0) return;
    assert(size <= srl.buffer_capacity - srl.buffer_count);
    assert(data);

    //void* memcpy(void* destination, const void* source, size_t num);
    if(srl.save) memcpy(srl.buffer + srl.buffer_count, data, size);
    else         memcpy(data, srl.buffer + srl.buffer_count, size);
    
    srl.buffer_count += size;
}

void do_everything(Serializer& srl, void* data, size_t size) {
    assert(size > 0);
    
    // Complete current buffer if needed.
    if(size >= srl.buffer_capacity - srl.buffer_count) {
        auto count = srl.buffer_capacity - srl.buffer_count;
        // buffer_serialize(srl.buffer + srl.buffer_count, data, count, srl.save);
        buffer_serialize(srl, data, count);
        data = (void*)((unsigned char*) data + count);
        size -= count;
        // If, the rest is too big, don't use buffer
        if(size >= srl.buffer_capacity) {
            if(srl.save) fwrite(data, size, 1, srl.file);
            else          fread(data, size, 1, srl.file);
            size = 0;
        }
        
        refresh_buffer(srl);
    }
    if(size == 0) return;

    // memcpy(data, srl.buffer + srl.buffer_count, size);
    // buffer_serialize(srl.buffer + srl.buffer_count, data, size, srl.save);
    buffer_serialize(srl, data, size);
}

void read(Serializer& srl, void* data, size_t size) {
    // fread(data, size, 1, srl.file);
    do_everything(srl, data, size);
}

void write(Serializer& srl, void* data, size_t size) { 
    fwrite(data, size, 1, srl.file);
    // do_everything(srl, data, size);
}

// Serialize (write or read) struct with no allocated resource
template <typename Type>
void serialize(Serializer& srl, Type& data, bool save) {
    if(save) write(srl, &data, sizeof(Type));
    else      read(srl, &data, sizeof(Type));
}

// Serialize std::vector
template <typename Type>
void serialize_vector(Serializer& srl, std::vector<Type>& vec, bool save) {
    size_t count;
    if(save) {
        count = vec.size();
        write(srl, &count, sizeof(size_t));
        write(srl, vec.data(), sizeof(Type) * count);
    }
    else {
        read(srl, &count, sizeof(size_t));
        assert(count < 10);
        vec = std::vector<Type>(count);
        read(srl, vec.data(), sizeof(Type) * count);
    }
}

// Serialize std::vector of structs with custom serialize function
template <typename Type>
void serialize_vector(Serializer& srl, std::vector<Type>& vec, bool save, std::function<void(Serializer&, Type&, bool)> serialize_obj) {
    size_t count;
    if(save) {
        count = vec.size();
        write(srl, &count, sizeof(size_t));
        for (int i = 0; i < count; ++i)
            serialize_obj(srl, vec[i], save);
    }
    else {
        read(srl, &count, sizeof(size_t));
        vec = std::vector<Type>(count);
        for (int i = 0; i < count; ++i)
            serialize_obj(srl, vec[i], save);
    }
}

// Serialize std::string
void serialize_string(Serializer& srl, std::string& str, bool save) {
    size_t count;
    if(save) {
        count = str.size();
        serialize(srl, count, true);
        write(srl, (void*)str.data(), sizeof(char) * count);
    }
    else {
        serialize(srl, count, false);
        str = std::string(count, '?');
        read(srl, (void*)str.data(), sizeof(char) * count);
    }
}

