#include <cassert>
#include <vector>
#include <string>
#include <functional>

#define SERIALIZER_VERSION 1

// Serializer holds the information needed to serialize data into file in binary format.
// To minimize disk access, a memory buffer is used to store temporary result. When full, 
// the buffer is dumped into file and cleared.
struct Serializer {
    FILE* file = nullptr;
    int version = 0;

    unsigned char* buffer = nullptr;
    size_t buffer_capacity = 0;
    size_t buffer_count = 0;
};

void serialize_error(const std::string& message) {
    printf("\n*** SERIALIZE ERROR ***\n");
    printf("message: %s\n", message.c_str());
    printf("\n");
    abort();
}

// write and read function are different from the standard fwrite and fread.
// This is because a memory buffer is used to miminize disk access.
void write(Serializer& srl, void* data, size_t size);
void read(Serializer& srl, void* data, size_t size);

// Serialize (write or read) struct with no allocated resource
template <typename Type>
void serialize(Serializer& srl, Type& data, bool save) {
    if(save) write(srl, &data, sizeof(Type));
    else      read(srl, &data, sizeof(Type));
}

Serializer make_serializer(const std::string& filename, bool save, size_t buffer_capacity = 0) {
    Serializer srl;
    srl.file = fopen(filename.c_str(), save? "w" : "r");
    if (not srl.file) {
        if(save) serialize_error("could not save data into file " + filename);
        else     serialize_error("could not load data from file " + filename);
    }

    srl.buffer = (unsigned char*) malloc(buffer_capacity);
    if(not srl.buffer) serialize_error("could not allocate buffer for file " + filename);
    srl.buffer_capacity = buffer_capacity;
    fread(srl.buffer, buffer_capacity, 1, srl.file);

    srl.version = SERIALIZER_VERSION;
    serialize(srl, srl.version, save);
    return srl;
}


void close_serializer(Serializer& srl) {
    if(srl.buffer) {
        fwrite(srl.buffer, srl.buffer_count, sizeof(unsigned char), srl.file);
        free(srl.buffer);
    }
    if(srl.file) fclose(srl.file);
    srl.buffer_capacity = 0;
    srl.buffer_count = 0;
}


void write(Serializer& srl, void* data, size_t size) {
    // If data is very big, don't use buffer.
    if(size >= srl.buffer_capacity) {
        fwrite(data, size, 1, srl.file);
        return;
    }
    
    // If buffer capacity is not enough, dump buffer to file and clear it.
    if(srl.buffer_count + size > srl.buffer_capacity) {
        fwrite(srl.buffer, srl.buffer_count, 1, srl.file);
        srl.buffer_count = 0;
    }

    // Write to buffer.
    memcpy(srl.buffer + srl.buffer_count, data, size);
    srl.buffer_count += size;
}


void read(Serializer& srl, void* data, size_t size) {
    // Buffer has not the whole data.
    if(srl.buffer_count + size >= srl.buffer_capacity) {   
        size_t offset = srl.buffer_capacity - srl.buffer_count;
        memcpy((unsigned char*)data, srl.buffer + srl.buffer_count, offset);

        if(size - offset >= srl.buffer_capacity) {
            fread((unsigned char*)data + offset, size - offset, 1, srl.file);
            fread(srl.buffer, srl.buffer_capacity, 1, srl.file);
            srl.buffer_count = 0;
            return;
        }
        fread(srl.buffer, srl.buffer_capacity, 1, srl.file);
        memcpy((unsigned char*)data + offset, srl.buffer, size - offset);
        srl.buffer_count = size - offset;
    }
    else {
        memcpy(data, srl.buffer + srl.buffer_count, size);
        srl.buffer_count += size;
    }
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

