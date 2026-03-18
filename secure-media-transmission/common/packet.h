#ifndef PACKET_H
#define PACKET_H

#include <string>
#include <vector>
#include <cstdint>

struct MediaPacket {
    std::string sender_id;
    int32_t packet_id;
    int64_t timestamp;
    std::string authentication_token;
    std::string payload_hash;
    std::vector<unsigned char> encrypted_payload;

    // Helper to serialize/deserialize for TCP transmission
    std::vector<unsigned char> serialize() const;
    static MediaPacket deserialize(const std::vector<unsigned char>& data);
};

#endif
