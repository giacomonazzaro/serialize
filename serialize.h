#include <cassert>
#include <vector>
#include <string>
#include <functional>

// Serializer holds the information needed to serialize data expressed in binary format into/from file. 
// To minimize disk access, a memory buffer is used to store temporary result.

struct Serializer {
    FILE* file = nullptr;
    bool is_writer;

    unsigned char* buffer = nullptr;
    size_t buffer_capacity = 0;
    size_t buffer_count = 0;

    ~Serializer() {
        if(file or buffer)
            assert(0 && "Close serializer before destruction!");
    } 
};


Serializer make_serializer(const std::string& filename, bool save, size_t buffer_capacity) {
    Serializer srl;
    srl.is_writer = save;
    srl.file = fopen(filename.c_str(), save? "w" : "r");
    if (not srl.file) {
        ("SERIALIZER ERROR: could not open file " + filename);
        assert(0);
    }

    srl.buffer_capacity = buffer_capacity;
    if(srl.buffer_capacity > 0) {
        srl.buffer = new unsigned char[buffer_capacity];
        if(not srl.buffer) ("SERIALIZER ERROR: could not allocate buffer for file " + filename);
    }
    else
        srl.buffer = nullptr;

    if(not srl.is_writer and srl.buffer_capacity > 0)
        fread(srl.buffer, srl.buffer_capacity, 1, srl.file);

    return srl;
}

Serializer make_reader(const std::string& filename, size_t buffer_capacity) {
    return make_serializer(filename, false, buffer_capacity);
}

Serializer make_writer(const std::string& filename, size_t buffer_capacity) {
    return make_serializer(filename, true, buffer_capacity);
}

// Release resources.
void close_serializer(Serializer& srl) {
    if(srl.buffer) {
        if(srl.is_writer)
            fwrite(srl.buffer, srl.buffer_count, 1, srl.file);

        delete [] srl.buffer;
        srl.buffer = nullptr;
        srl.buffer_capacity = 0;
        srl.buffer_count = 0;
    }
    if(srl.file) fclose(srl.file);
    srl.file = nullptr;
}

// Write/read from/to memory buffer.
void buffer_serialize(Serializer& srl, void* data, size_t size) {
    if(srl.buffer_capacity == 0) return;
    if(size == 0) return;
    assert(size <= srl.buffer_capacity - srl.buffer_count);
    assert(data);

    //void* memcpy(void* destination, const void* source, size_t num);
    if(srl.is_writer) memcpy(srl.buffer + srl.buffer_count, data, size);
    else         memcpy(data, srl.buffer + srl.buffer_count, size);
    
    srl.buffer_count += size;
}

// Read using buffer when possible.
void read(Serializer& srl, void* data, size_t size) {
    assert(size > 0);
    
    // Complete current buffer if needed.
    if(size >= srl.buffer_capacity - srl.buffer_count) {
        auto count = srl.buffer_capacity - srl.buffer_count;
        buffer_serialize(srl, data, count);
        data = (void*)((unsigned char*) data + count);
        size -= count;

        // If, the rest is too big, don't use buffer
        if(size >= srl.buffer_capacity) {
            if(srl.is_writer) fwrite(data, size, 1, srl.file);
            else               fread(data, size, 1, srl.file);
            size = 0;
        }

        // Refill buffer.
        assert(srl.buffer_count == srl.buffer_capacity);
        fread(srl.buffer, srl.buffer_capacity, 1, srl.file);
        srl.buffer_count = 0;
    }

    buffer_serialize(srl, data, size);
}

// Write using buffer when possible.
void write(Serializer& srl, void* data, size_t size) { 
    assert(size > 0);

    if(size >= srl.buffer_capacity - srl.buffer_count) {
        fwrite(srl.buffer, srl.buffer_count, 1, srl.file);
        fwrite(data, size, 1, srl.file);
        srl.buffer_count = 0;
    }
    else {
        buffer_serialize(srl, data, size);
    }
}

// Serialize (write or read) struct with no allocated resource
template <typename Type>
void serialize(Serializer& srl, Type& data) {
    if(srl.is_writer) write(srl, &data, sizeof(Type));
    else               read(srl, &data, sizeof(Type));
}

// Serialize std::vector
template <typename Type>
void serialize_vector(Serializer& srl, std::vector<Type>& vec) {
    size_t count = vec.size();
    serialize(srl, count);
    if(count == 0) return;
    if(srl.is_writer) {
        write(srl, vec.data(), sizeof(Type) * count);
    }
    else {
        vec = std::vector<Type>(count);
        read(srl, vec.data(), sizeof(Type) * count);
    }
}

// Serialize std::vector of structs with custom serialize function
template <typename Type, typename Serialize_func>
void serialize_vector_custom(Serializer& srl, std::vector<Type>& vec, Serialize_func serialize_obj) {
    size_t count = vec.size();
    serialize(srl, count);
    if(count == 0) return;
    if(srl.is_writer) {
        for (int i = 0; i < count; ++i)
            serialize_obj(srl, vec[i]);
    }
    else {
        vec = std::vector<Type>(count);
        for (int i = 0; i < count; ++i)
            serialize_obj(srl, vec[i]);
    }
}

// Serialize std::string
void serialize_string(Serializer& srl, std::string& str) {
    size_t count = str.size();
    serialize(srl, count);
    if(count == 0) return;
    if(srl.is_writer) {
        write(srl, (void*)str.data(), sizeof(char) * count);
    }
    else {
        str = std::string(count, '?');
        read(srl, (void*)str.data(), sizeof(char) * count);
    }
}


template <typename Type>
void save_to_file(const std::string& filename, Type& object) {
    auto writer = make_writer(filename, 0);
    serialize(writer, object);
    close_serializer(writer);
}

template <typename Type>
void init_from_file(const std::string& filename, Type& object) {
    auto reader = make_reader(filename, 0);
    serialize(reader, object);
    close_serializer(reader);
}

template <typename Type>
Type make_from_file(const std::string& filename) {
    auto reader = make_reader(filename, 0);
    Type object;
    serialize(reader, object);
    close_serializer(reader);
    return object;
}


