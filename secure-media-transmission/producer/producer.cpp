#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../common/packet.h"
#include "../common/crypto.h"

int main() {
    const std::string SERVER_IP = "127.0.0.1";
    const int PORT = 8080;
    const std::string AUTH_TOKEN = "SECURE_TOKEN_2026";
    
    // Hardcoded AES key and IV for prototype
    std::vector<unsigned char> key = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 
                                     0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32};
    std::vector<unsigned char> iv = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18, 0x29, 0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F, 0x90};

    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    std::cout << "Connecting to receiver at " << SERVER_IP << ":" << PORT << "..." << std::endl;
    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed. Retrying in 2 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    for (int i = 1; i <= 5; ++i) {
        std::string frame_data = "FRAME_" + std::to_string(i);
        std::cout << "Producer sending packet " << i << " (" << frame_data << ")" << std::endl;

        MediaPacket packet;
        packet.sender_id = "PRODUCER_01";
        packet.packet_id = i;
        packet.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        packet.authentication_token = AUTH_TOKEN;
        
        // 1. Calculate Hash of original payload
        packet.payload_hash = Crypto::sha256(frame_data);
        
        // 2. Encrypt Payload
        packet.encrypted_payload = Crypto::encrypt(frame_data, key, iv);

        // 3. Serialize and Send
        std::vector<unsigned char> serialized = packet.serialize();
        uint32_t size = serialized.size();
        send(sock, &size, sizeof(size), 0); // Send size first
        send(sock, serialized.data(), serialized.size(), 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    close(sock);
    return 0;
}
