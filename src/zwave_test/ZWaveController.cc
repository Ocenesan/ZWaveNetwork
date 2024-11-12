#include <omnetpp.h>
#include <cmath>

using namespace omnetpp;

namespace zwavenetwork {

class ZWaveController : public cSimpleModule
{
protected:
    double frequencyLow;
    double frequencyHigh;
    int numGates;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    bool isValidGateIndex(int index) const;
};

Define_Module(ZWaveController);

void ZWaveController::initialize()
{
    frequencyLow = par("frequencyLow").doubleValue();
    frequencyHigh = par("frequencyHigh").doubleValue();
    numGates = gateSize("gate");
    EV << "ZWave Controller initialized with FSK frequencies "
           << frequencyLow << " MHz and " << frequencyHigh << " MHz\n";
    EV << "Number of connected gates: " << numGates << "\n";

    if (numGates == 0) {
        EV << "Warning: Controller has no connected gates!\n";
        return;
    }

    // Schedule the first message
    cMessage *startMsg = new cMessage("start");
    scheduleAt(simTime() + uniform(0, 1), startMsg);
}

bool ZWaveController::isValidGateIndex(int index) const
{
    return (index >= 0 && index < numGates && gate("gate$o", index)->isConnected());
}


void ZWaveController::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // Schedule multiple messages to different nodes
        int numMessagesToSend = 3;  // Number of simultaneous messages
        for (int i = 0; i < numMessagesToSend; i++) {
            // Randomly select a node to send the message to
            int randomNode;
            int attempts = 0;
            int maxAttempts = 5;  // Prevent infinite loops

            do {
                randomNode = intuniform(0, numGates - 1);
                attempts++;
            } while (!isValidGateIndex(randomNode) && attempts < maxAttempts);

            if (attempts >= maxAttempts) {
                EV << "Could not find a valid gate to send message after " << maxAttempts << " attempts\n";
                continue;
            }

            // Generate a random bit and frequency
            int bitToSend = intuniform(0, 1);
            double chosenFrequency = (bitToSend == 0) ? frequencyLow : frequencyHigh;

            // Create and send the message
            cMessage *dataMsg = new cMessage("data");
            dataMsg->addPar("frequency") = chosenFrequency; // Add frequency parameter
            dataMsg->addPar("bit") = bitToSend;  // Add bit parameter

            try {
                // Attempt to send the message to the selected gate (node)
                send(dataMsg, "gate$o", randomNode);
                EV << "Controller sent bit " << bitToSend << " to Node " << randomNode
                   << " with frequency " << chosenFrequency << " MHz\n";
            }
            catch (const cRuntimeError& e) {
                EV << "Error sending message: " << e.what() << "\n";
                delete dataMsg;  // Clean up if send fails
            }
        }

        // Schedule the next set of messages
        cMessage *nextMsg = new cMessage("start");
        scheduleAt(simTime() + exponential(1.0), nextMsg);
    }
    else {
        // Message received from a node
        int arrivalGate = msg->getArrivalGate()->getIndex();
        EV << "Controller received: " << msg->getName()
           << " from gate " << arrivalGate << "\n";

        // Access the parameters if needed
        double receivedFrequency = msg->par("frequency");
        int receivedBit = msg->par("bit");
        EV << "Received Frequency: " << receivedFrequency << " MHz, Bit: " << receivedBit << "\n";

        delete msg;
    }
}

}
