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
};

void serialize_error(const std::string& message) {
    printf("\n*** SERIALIZE ERROR ***\n");
    printf("message: %s\n", message.c_str());
    printf("\n");
    abort();
}

Serializer make_serializer(const std::string& filename, bool save, size_t buffer_capacity = 0) {
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

    if(save) fwrite(&srl.version, 1, sizeof(int), srl.file);
    else      fread(&srl.version, 1, sizeof(int), srl.file);

    srl.save = save;
    if(not save)
        fread(srl.buffer, srl.buffer_capacity, 1, srl.file);

    return srl;
}

void reload_buffer(Serializer& srl) {
    assert(not srl.save);
    fread(srl.buffer, srl.buffer_capacity, 1, srl.file);
    srl.buffer_count = 0;
}

void dump_buffer(Serializer& srl) {
    assert(srl.save);
    fwrite(srl.buffer, srl.buffer_count, 1, srl.file);
    srl.buffer_count = 0;
}

void close_serializer(Serializer& srl) {
    if(srl.buffer and srl.save) {
        dump_buffer(srl);
        free(srl.buffer);
    }
    if(srl.file) fclose(srl.file);
    srl.buffer_capacity = 0;
    srl.buffer_count = 0;
}



// write and read function are different from the standard fwrite and fread.
// A memory buffer is used to miminize disk access.

void write(Serializer& srl, void* data, size_t size) {
    // If data is very big, don't use buffer.
    if(size >= srl.buffer_capacity) {
        fwrite(data, size, 1, srl.file);
        return;
    }
    
    // If buffer capacity is not enough, dump buffer to file and clear it.
    if(srl.buffer_count + size > srl.buffer_capacity) {
        dump_buffer(srl);
    }

    // Write to buffer.
    memcpy(srl.buffer + srl.buffer_count, data, size);
    srl.buffer_count += size;
}


void read(Serializer& srl, void* data, size_t size) {
    assert(size > 0);
//    if(size >= srl.buffer_capacity) {
//        fread(data, size, 1, srl.file);
//        return;
//    }

    if(size >= srl.buffer_capacity - srl.buffer_count) {
        int count = srl.buffer_capacity - srl.buffer_count;
        memcpy(data, srl.buffer + srl.buffer_count, count);
        data = (unsigned char*) data + count;
        size -= count;
        if(size > srl.buffer_capacity) {
            fread(data , size, 1, srl.file);
            size = 0;
        }
        reload_buffer(srl);
    }
    if(size == 0) return;

    memcpy(data, srl.buffer + srl.buffer_count, size);
    srl.buffer_count += size;



    // if(size > srl.buffer_capacity - srl.buffer_count) {
    //     count += srl.buffer_capacity - srl.buffer_count;
    //     memcpy(data, srl.buffer + srl.buffer_count, count);
    //     srl.buffer_count += count;
    // }

    // if(srl.buffer_count == srl.buffer_capacity) 
    //     reload_buffer(srl);
    
    // if(count == size) return;


    // // Check if needed data is in buffer.
    // int count = std::min(size, srl.buffer_capacity - srl.buffer_count);
    // memcpy(data, srl.buffer + srl.buffer_count, count);
    // srl.buffer_count += count;
    // if(srl.buffer_count == srl.buffer_capacity) 
    //     reload_buffer(srl);
    
    // if(count == size) return;
    // assert(count < size and srl.buffer_count == srl.buffer_capacity);

    // if(size - count >= srl.buffer_capacity)
    //     fread((unsigned char*)data + count, size - count, 1, srl.file);


    // auto destination = (unsigned char*) data + count;
    // auto source = srl.buffer + srl.buffer_count;
    // memcpy(destination, source, size - count);

    // while(count < size) {
    //     if(srl.buffer_count == srl.buffer_capacity) 
    //         reload_buffer(srl);

        // int chunk = std::min(size, srl.buffer_capacity - srl.buffer_count);
        // count += chunk;
        // auto destination = (unsigned char*) data + count;
        // auto source = srl.buffer + srl.buffer_count;
        // memcpy(destination, source, chunk);
    // }
    // assert(count == size);
    // srl.buffer_count += size;
}

void read_(Serializer& srl, void* data, size_t size) {
    // Buffer has not the whole data.
    if(srl.buffer_count + size >= srl.buffer_capacity) {   
        size_t offset = srl.buffer_capacity - srl.buffer_count;
        memcpy((unsigned char*)data, srl.buffer + srl.buffer_count, offset);

        // If the rest of data is still too big, dump to file and load new page.
        if(size - offset >= srl.buffer_capacity) {
            fread((unsigned char*)data + offset, size - offset, 1, srl.file);
            fread(srl.buffer, srl.buffer_capacity, 1, srl.file);
            srl.buffer_count = 0;
            return;
        }

        // Load new page.
        fread(srl.buffer, srl.buffer_capacity, 1, srl.file);
        memcpy((unsigned char*)data + offset, srl.buffer, size - offset);
        srl.buffer_count = size - offset;
    }
    else {
        memcpy(data, srl.buffer + srl.buffer_count, size);
        srl.buffer_count += size;
    }
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

