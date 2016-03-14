
/*
packet types:
* control (velocity, kick/chip, dribbler)
* status update from robot
*
* parameter update
* parameter update request from robot
*
* push firmware update from computer
* based on hash of file, only update if needed?
*
* FirmwareUpdateAnnounce
* FirmwareUpdateReply - ack that you heard it, reply with radio



How does parameter sync work?
* Hash to see if up-to-date?
* If out of date, perform a full-sync over reliable channel
* Robot requests parameter updates when it wants them
* Computer can send them at will or by choice
*

Soccer has an
-invalidateParameter() function

Base station has a packet queue for things to send
* Allocate bandwidth to different protocols

Reliable comm:
* a la tcp?
* connection handshake
* choose port?  connection id?
*

Reliable multicast?
* Yes.  But if a robot breaks connection, log it and drop it


Status Update
*
*

Channels?
* set channel spacing in reg config via SmartRF Studio
* DIP switch on robot?
* Option selection in soccer
*/

class RobotParameterUpdater {
public:
    void invalidateParameter(const std::string& key, uint8_t robotID);
    void invalidateParameter(const std::string& key);
};

class ReliableChannel {
    template <typename ARRAY_TYPE>
    void send(const ARRAY_TYPE& buffer, int size);
};

class PacketHeader {
public:
};

template <class MSG_TYPE, int MSG_COUNT, class HDR_CLASS = PacketHeader>
class Packet {
public:
private:
    HDR_CLASS _header;
    std::array<MSG_TYPE, MSG_COUNT> _messages;
};

struct RobotMessage {
    uint8_t unique_id = 0;  // robot being addressed.  0 == nobody
};

struct RobotReplyMessage {
    uint8_t unique_id;
    unsigned int shell : 4;
    // TODO: voltage levels
};

struct Packet {
    unsigned int type : 4;

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

// TODO: if we haven't heard from base station in a while, timeout and
// "disconnect"
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
        std::unique_ptr pkt(new RobotReplyMessage);  // TODO: should be a packet
        commModule()->send(pkt);
    }

    void _rxHandler(const rtp::packet* pkt) {
        // Iterate over packete messages and see if any are addressed to us. If
        // so, we'll reply in the timeslot according to index of the message we
        // got. If not, we'll reply in an unassigned reply slot (if any are
        // available).
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

    // Timer used to delay our reply to the base station according to our
    // assigned timeslot
    RtosTimer _replyTimer;

    // whether or not we're in communication mode (attempting to talk to a base
    // station)
    bool _started = false;

    // whether or not we're connected to the radio base station
    bool _connected = false;

    // the most-recently received control message
    ControlMessage _controlMessage;
};

class BaseStation : public ProtocolEntity {
public:
    /// After this amount of time (ms) of radio silence from a robot, it is
    /// considered disconnected
    static uint32_t ClientTimeout = 200;

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

// Variable-length reply slots
