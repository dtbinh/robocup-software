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
 * @brief Port enumerations for different communication protocols.
 */
enum port {
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
};

struct header_data {
    enum Type { Control, Tuning, FirmwareUpdate, Misc };

    header_data(uint8_t p = SINK) : address(0), port(p), type(Control){};

    uint8_t address;
    unsigned int port : 4;
    Type type : 4;

    /// "packed" size (in bytes) of header data
    size_t size() const { return 2; }

    void pack(std::vector<uint8_t>* buf) const {
        buf->push_back(address);
        buf->push_back(port << 4 | type);
    }

    void unpack(const std::vector<uint8_t>& buf) {
        address = buf[1];
        port = buf[2] >> 4;
        type = (Type)(buf[2] & 0x0F);
    }
};

/**
 * @brief Real-Time packet definition
 */
class packet {
public:
    rtp::header_data header;
    std::vector<uint8_t> payload;

    packet(){};
    packet(const std::string& s, uint8_t p = SINK) : header(p) {
        for (char c : s) payload.push_back(c);
        payload.push_back('\0');
    }

    template <class T>
    packet(const std::vector<T>& v, uint8_t p = SINK)
        : header(p) {
        for (T val : v) payload.push_back(val);
    }

    size_t size() const { return payload.size() + header.size(); }

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
        for (size_t i = header.size() + 1; i < v.size(); i++) {
            payload.push_back(v[i]);
        }
    }

    void pack(std::vector<uint8_t>* buffer) const {
        // first byte is total size (excluding the size byte)
        const uint8_t total_size = payload.size() + header.size();
        buffer->reserve(total_size + 1);

        buffer->push_back(total_size);

        header.pack(buffer);

        // payload
        buffer->insert(buffer->end(), payload.begin(), payload.end());
    }
};
}
