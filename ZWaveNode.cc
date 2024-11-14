#include <omnetpp.h>
#include <cmath>
#include <string>

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
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(ZWaveNode);

void ZWaveNode::initialize()
{
    frequency = par("frequency").doubleValue();
    range = par("range").doubleValue();
    frequencyLow = par("frequencyLow").doubleValue();
    frequencyHigh = par("frequencyHigh").doubleValue();
    std::string deviceType = par("deviceType").stringValue();
    //EV << "ZWave Node " << getIndex() << " initialized with frequency " << frequency << " MHz and range " << range << " m\n";
    EV << "ZWave Node " << getIndex() << " (" << deviceType << ") initialized with frequencies "
       << frequencyLow << " MHz and " << frequencyHigh << " MHz\n";
    getDisplayString().setTagArg("t", 0, deviceType.c_str());
}

void ZWaveNode::handleMessage(cMessage *msg)
{
    // Process the incoming message
    std::string deviceType = par("deviceType").stringValue();
    double receivedFrequency = msg->par("frequency").doubleValue();
    int receivedBit = (receivedFrequency == frequencyLow) ? 0 : 1;

    //EV << "Node " << getIndex() << " received bit " << receivedBit << " with frequency "
       //<< receivedFrequency << " MHz\n";

    if (deviceType == "Lamp") {
        EV << "Lamp received bit " << receivedBit << "\n";
        // Simulate light on/off state change here
    } else if (deviceType == "Thermostat") {
        EV << "Thermostat adjusting temperature based on received bit " << receivedBit << "\n";
        // Simulate thermostat adjustment here
    } else if (deviceType == "Door Lock") {
        EV << "Door Lock received bit " << receivedBit << "\n";
        // Simulate lock/unlock here
    }

    // Forward the message to another random node, if desired
    int numGates = gateSize("gate");
    int randomGate = intuniform(0, numGates - 1);

    cMessage *fwdMsg = msg->dup();
    send(fwdMsg, "gate$o", randomGate);
    EV << "Node " << getIndex() << " forwarded message to gate " << randomGate << "\n";

    delete msg;
}

}
