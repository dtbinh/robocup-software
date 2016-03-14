#pragma once

#include <cstdint>
#include <vector>
#include <string>

#define BASE_STATION_ADDR (1)
#define LOOPBACK_ADDR (127)

namespace rtp {

/// Max packet size.  This is limited by the CC1201 buffer size.
static const unsigned int MAX_DATA_SZ = 120;

/**
 * Subclass this class to enable looping easily looping over the bytes in a
 * class/struct.
 * For example:
 * struct MyStruct : public ByteIterable<MyStruct> {
 *     int intValue;
 *     char charValue;
 * }
 *
 * MyStruct s;
 * for (uint8_t byte : s) printf("0x%X ", byte);
 */
template <class SUBCLASS>
class ByteIterable {
public:
    typedef uint8_t* iterator;
    typedef const uint8_t* const_iterator;
    iterator begin() { return (uint8_t*)this; }
    const_iterator begin() const { return (const uint8_t*)this; }
    iterator end() { return (uint8_t*)this + sizeof(SUBCLASS); }
    const_iterator end() const {
        return (const uint8_t*)this + sizeof(SUBCLASS);
    }

    // Attempts to create packet classes that exceed the max amount the cc1201
    // can send will fail to compile
    static_assert(sizeof(SUBCLASS) <= MAX_DATA_SZ,
                  "Too big to fit in a single cc1201 packet");
};

/**
 * @brief Port enumerations for different communication protocols.
 */
typedef enum {
    SINK = 0,
    LINK,
    CONTROL,
    // SETPOINT,
    // GSTROBE,
    DISCOVER,
    LOGGER,
    TCP,
    LEGACY,
    PING
} Port;

struct header_data {
    header_data(Port p = SINK) : address(0), port(p){};
    unsigned address : 8;
    Port port : 4;
} __attribute__((packed));

/**
 * @brief Real-Time packet definition
 */
class packet final : public ByteIterable<packet> {
public:
    rtp::header_data header;
    std::vector<uint8_t> payload;

    packet(){};
    packet(const std::string& s, Port p = SINK) : header(p) {
        for (char c : s) payload.push_back(c);
        payload.push_back('\0');
    }

    template <class T>
    packet(const std::vector<T>& v, uint8_t p = SINK)
        : header(p) {
        for (T val : v) payload.push_back(val);
    }

    int port() const { return static_cast<int>(header.port); }
    template <class T>
    void port(T p) {
        header.port = static_cast<unsigned int>(p);
    }

    int address() { return header.address; }
    void address(int a) { header.address = static_cast<unsigned int>(a); }

    template <class T>
    void recv(const std::vector<T>& v) {
        // note: header ignores the first byte since it's the size byte
        header.unpack(v);

        


        // Everything after the header is payload data
        payload.clear();
        for (size_t i = sizeof(header) + 1; i < v.size(); i++) {
            payload.push_back(v[i]);
        }
    }

    void pack(std::vector<uint8_t>* buffer) const {
        // first byte is total size (excluding the size byte)
        const uint8_t total_size = payload.size() + header.size();
        buffer->reserve(total_size + 1);

        buffer->push_back(total_size);

        for (size_t i = 0; i < sizeof(header); i++) {
            buffer->push_back(((uint8_t*)&header)[i]);
        }

        // payload
        buffer->insert(buffer->end(), payload.begin(), payload.end());
    }
};
}

struct ControlMessage {
    int vel_x : 16;
    int vel_y : 16;
    int vel_w : 16;

    int kick_power : 16;
    unsigned chip : 1;  // 1 == chip, 0 == kick
    unsigned
        kick_mode : 2;  // 0 == do nothing, 1 == on beam break, 2 = immediate

} __attribute__((packed));
