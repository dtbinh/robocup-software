
struct ControlMessage {
    uint8_t unique_id; // robot being addressed.  0 == nobody
    int16_t vel_x;
    int16_t vel_y;
    int16_t vel_w;
    uint16_t kick_power;
    unsigned int chip:1;    // 1 == chip, 0 == kick
    unsigned int kick_mode:2;   // 0 == do nothing, 1 == on beam break, 2 = immediate
};

struct RobotReplyMessage {
    uint8_t unique_id;
    unsigned int shell:4;
    // TODO: voltage levels
};

struct Packet {
    unsigned int type:4;

    ControlMessage control[6];
};

class ProtocolEntity {
public:
    ProtocolEntity(CommModule* comm, CC1201* radio);
    virtual ~ProtocolEntity() { stop(); }

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void rxHandler(const rtp::packet* pkt) = 0;

protected:
    CommModule* commModule() { return _commModule; }
    CC1201* radio() { return _radio; }

private:
    CommModule* _commModule;
    CC1201* _radio;
};



// TODO: if we haven't heard from base station in a while, timeout and "disconnect"
class ProtocolClient : public ProtocolEntity {
public:
    ProtocolClient(uint8_t unique_id, CommModule* comm, CC1201* radio)
        : ProtocolEntity(comm, radio);

    void start() {
        if (_started) return;

        commModule()->RxHandler(rtp::port::CONTROL, this, _rxHandler);
    }

    void stop() {
        commModule()->closePort(rtp::port::CONTROL);

        _replyTimer.stop();
        _started = false;
        _connected = false;
    }

    bool isConnected() const { return _connected; }

private:
    void _sendReply(void* arg) {
        std::unique_ptr pkt(new RobotReplyMessage); // TODO: should be a packet
        commModule()->send(pkt);
    }

    void _rxHandler(const rtp::packet* pkt) {
        int index = -1;
        int openSlots = 0;
        const Packet* ctrlPkt = (const Packet*)pkt;
        for (int i = 0; i < 6; i++) {
            if (ctrlPkt->control[i].unique_id == _uniqueId) {
                index = i;
                _controlMessage = ctrlPkt->control[i];
                break;
            }

            if (ctrlPkt->control[i].unique_id == 0) openSlots++;
        }

        if (index != -1) {
            // TODO: schedule reply

            if (!_connected) LOG(INF1, "Connected to radio base station");
            _connected = true;
        } else {
            // this radio packed did not address us.
            _connected = false;

            if (openSlots > 0) {
                // TODO: choose random reply slot
                // TODO: schedule reply
            }
        }
    }

    RtosTimer _replyTimer;

    bool _started = false;
    bool _connected = false;

    ControlMessage _controlMessage;
};


class ProtocolServer : public ProtocolEntity {
public:
    
private:
    struct ClientInfo {
        uint32_t lastPacketTime;
    };

    std::map<int, ClientInfo> _clients;
};


// OTA Firmware update
// Parameter tuning
// Serial Console?
// RTP
