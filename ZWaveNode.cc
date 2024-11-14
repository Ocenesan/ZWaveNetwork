#include <omnetpp.h>
#include <cmath>
#include <string>
#include <vector>


using namespace omnetpp;

namespace zwavenetwork {

class ZWaveNode : public cSimpleModule
{
protected:
    double frequency;
    double range;
    double frequencyLow;
    double frequencyHigh;
    std::string deviceType;
    cMessage *selfMsg; // For scheduled events

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void sendToController(const std::string& eventName);
    void sendStatusUpdate(const std::string& status);
    void forwardMessage(cMessage *msg, int incomingGateIndex);
};

Define_Module(ZWaveNode);

void ZWaveNode::initialize()
{
    frequency = par("frequency").doubleValue();
    range = par("range").doubleValue();
    frequencyLow = par("frequencyLow").doubleValue();
    frequencyHigh = par("frequencyHigh").doubleValue();
    deviceType = par("deviceType").stdstringValue();

    // Schedule motion detection for sensor nodes
    if (deviceType == "sensor") {
        selfMsg = new cMessage("motionDetected");
        scheduleAt(simTime() + par("motionDetectionInterval").doubleValue(), selfMsg);
    }

    EV << "ZWave Node " << getIndex() << " (" << deviceType << ") initialized with frequencies "
       << frequencyLow << " MHz and " << frequencyHigh << " MHz\n";
    getDisplayString().setTagArg("t", 0, deviceType.c_str());
}

void ZWaveNode::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
         if (deviceType == "sensor" && strcmp(msg->getName(), "motionDetected") == 0) {
            EV << "Sensor Node " << getIndex() << " detected motion!\n";
            sendToController("MotionDetected");
            scheduleAt(simTime() + par("motionDetectionInterval").doubleValue(), msg);
        }
    } else {
        int incomingGateIndex = msg->getArrivalGate()->getIndex();
        std::string msgName = msg->getName();

        EV << "Node " << getIndex() << " (" << deviceType << ") received message: "
           << msgName << " from gate " << incomingGateIndex << "\n";

        if (msgName == "data") { // Process FSK messages
            double receivedFrequency = msg->par("frequency").doubleValue();
            int receivedBit = (receivedFrequency == frequencyLow) ? 0 : 1;

            if (deviceType == "Lamp") {
                EV << "Lamp received bit " << receivedBit << "\n";
                 if (receivedBit == 1) {
                       sendStatusUpdate("LampOn");
                   } else {
                       sendStatusUpdate("LampOff");
                   }
            } else if (deviceType == "Thermostat") {
                EV << "Thermostat adjusting temperature based on received bit " << receivedBit << "\n";
                if (receivedBit == 1) {
                    sendStatusUpdate("RaiseTemp");
                } else {
                    sendStatusUpdate("ReduceTempt");
                }
            } else if (deviceType == "Door Lock") {
                EV << "Door Lock received bit " << receivedBit << "\n";
                sendStatusUpdate(receivedBit == 1 ? "DoorUnlocked" : "DoorLocked");
            }
            delete msg;  // Delete the FSK message after processing it

        } else {
            forwardMessage(msg, incomingGateIndex); // Forward other messages
            return; // Important: Don't delete the message here, it's handled in forwardMessage

        }
    }
}



void ZWaveNode::sendToController(const std::string& eventName)
{
    cMessage *eventMsg = new cMessage(eventName.c_str());
    send(eventMsg, "gate$o", 0); // Send to controller (gate 0)
    EV << "Node " << getIndex() << " sending '" << eventName << "' to controller.\n";
}

void ZWaveNode::sendStatusUpdate(const std::string& status)
{
    sendToController(status); // Send status update back to the controller
}

void ZWaveNode::forwardMessage(cMessage *msg, int incomingGateIndex)
{
    int numGates = gateSize("gate");
    for (int i = 0; i < numGates; ++i) {
        if (i != incomingGateIndex) {
            cMessage *forwardedMsg = msg->dup();
            send(forwardedMsg, "gate$o", i);
            EV << "Node " << getIndex() << " forwarding message '" << forwardedMsg->getName()
               << "' to gate " << i << "\n";
        }
    }
    delete msg;
}

} // namespace zwavenetwork
