#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <map>
#include "../common/packet.h"
#include "../common/crypto.h"

int main() {
    const int PORT = 8080;
    const std::string EXPECTED_TOKEN = "SECURE_TOKEN_2026";
    
    std::vector<unsigned char> key = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 
                                     0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32};
    std::vector<unsigned char> iv = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18, 0x29, 0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F, 0x90};

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Receiver listening on port " << PORT << "..." << std::endl;

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    int last_packet_id = 0;

    while (true) {
        uint32_t packet_size = 0;
        int valread = read(new_socket, &packet_size, sizeof(packet_size));
        if (valread <= 0) break;

        std::vector<unsigned char> buffer(packet_size);
        int total_read = 0;
        while(total_read < packet_size) {
            int n = read(new_socket, buffer.data() + total_read, packet_size - total_read);
            if(n <= 0) break;
            total_read += n;
        }

        try {
            MediaPacket packet = MediaPacket::deserialize(buffer);
            std::cout << "\nReceiver received packet " << packet.packet_id << " from " << packet.sender_id << std::endl;

            // Reliability Check
            if (packet.packet_id <= last_packet_id) {
                std::cout << "[Warning] Duplicate or out-of-order packet detected! (Expected > " << last_packet_id << ")" << std::endl;
            }
            last_packet_id = packet.packet_id;

            // 1. Validate Auth Token
            if (packet.authentication_token != EXPECTED_TOKEN) {
                std::cout << "[Error] Authentication failed!" << std::endl;
                continue;
            }

            // 2. Decrypt Payload
            std::cout << "Decrypting payload..." << std::endl;
            std::string decrypted_payload = Crypto::decrypt(packet.encrypted_payload, key, iv);

            // 3. Verify Integrity (SHA-256)
            std::string calculated_hash = Crypto::sha256(decrypted_payload);
            if (calculated_hash == packet.payload_hash) {
                std::cout << "Integrity verified (SHA-256 match)" << std::endl;
                std::cout << "Frame processed: " << decrypted_payload << std::endl;
            } else {
                std::cout << "[Error] Integrity check failed! Hash mismatch." << std::endl;
            }

        } catch (const std::exception& e) {
            std::cerr << "Error processing packet: " << e.what() << std::endl;
        }
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
