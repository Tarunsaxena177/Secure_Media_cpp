#include "packet.h"
#include <cstring>
#include <stdexcept>

// Simple binary serialization format:
// [sender_id_len (4)] [sender_id] [packet_id (4)] [timestamp (8)] 
// [token_len (4)] [token] [hash_len (4)] [hash] [payload_len (4)] [payload]

std::vector<unsigned char> MediaPacket::serialize() const {
    std::vector<unsigned char> buffer;

    auto add_string = [&](const std::string& s) {
        uint32_t len = s.size();
        unsigned char bytes[4];
        std::memcpy(bytes, &len, 4);
        for(int i=0; i<4; ++i) buffer.push_back(bytes[i]);
        for(char c : s) buffer.push_back(static_cast<unsigned char>(c));
    };

    add_string(sender_id);

    unsigned char p_id_bytes[4];
    std::memcpy(p_id_bytes, &packet_id, 4);
    for(int i=0; i<4; ++i) buffer.push_back(p_id_bytes[i]);

    unsigned char ts_bytes[8];
    std::memcpy(ts_bytes, &timestamp, 8);
    for(int i=0; i<8; ++i) buffer.push_back(ts_bytes[i]);

    add_string(authentication_token);
    add_string(payload_hash);

    uint32_t p_len = encrypted_payload.size();
    unsigned char p_len_bytes[4];
    std::memcpy(p_len_bytes, &p_len, 4);
    for(int i=0; i<4; ++i) buffer.push_back(p_len_bytes[i]);
    for(unsigned char b : encrypted_payload) buffer.push_back(b);

    return buffer;
}

MediaPacket MediaPacket::deserialize(const std::vector<unsigned char>& data) {
    MediaPacket p;
    size_t offset = 0;

    auto read_string = [&](std::string& s) {
        if (offset + 4 > data.size()) throw std::runtime_error("Buffer too small");
        uint32_t len;
        std::memcpy(&len, &data[offset], 4);
        offset += 4;
        if (offset + len > data.size()) throw std::runtime_error("Buffer too small for string");
        s.assign(reinterpret_cast<const char*>(&data[offset]), len);
        offset += len;
    };

    read_string(p.sender_id);

    if (offset + 4 > data.size()) throw std::runtime_error("Buffer too small for packet_id");
    std::memcpy(&p.packet_id, &data[offset], 4);
    offset += 4;

    if (offset + 8 > data.size()) throw std::runtime_error("Buffer too small for timestamp");
    std::memcpy(&p.timestamp, &data[offset], 8);
    offset += 8;

    read_string(p.authentication_token);
    read_string(p.payload_hash);

    if (offset + 4 > data.size()) throw std::runtime_error("Buffer too small for payload_len");
    uint32_t p_len;
    std::memcpy(&p_len, &data[offset], 4);
    offset += 4;
    if (offset + p_len > data.size()) throw std::runtime_error("Buffer too small for payload");
    p.encrypted_payload.assign(data.begin() + offset, data.begin() + offset + p_len);
    
    return p;
}
