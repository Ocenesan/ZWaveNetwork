#include <omnetpp.h>
#include <cmath>

using namespace omnetpp;

namespace zwavenetwork {

class ZWaveNode : public cSimpleModule
{
protected:
    double frequency;
    double range;
    double frequencyLow;
    double frequencyHigh;
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
    //EV << "ZWave Node " << getIndex() << " initialized with frequency " << frequency << " MHz and range " << range << " m\n";
    EV << "ZWave Node " << getIndex() << " initialized with FSK frequencies "
           << frequencyLow << " MHz and " << frequencyHigh << " MHz\n";
}

void ZWaveNode::handleMessage(cMessage *msg)
{
    // Process the incoming message
    double receivedFrequency = msg->par("frequency").doubleValue();
    int receivedBit = (receivedFrequency == frequencyLow) ? 0 : 1;

    EV << "Node " << getIndex() << " received bit " << receivedBit << " with frequency "
       << receivedFrequency << " MHz\n";

    // Forward the message to another random node, if desired
    int numGates = gateSize("gate");
    int randomGate = intuniform(0, numGates - 1);

    cMessage *fwdMsg = msg->dup();
    send(fwdMsg, "gate$o", randomGate);
    EV << "Node " << getIndex() << " forwarded message to gate " << randomGate << "\n";

    delete msg;
}

}
