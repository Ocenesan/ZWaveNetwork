#include <omnetpp.h>
#include <cmath>
#include <vector>
#include <string>

using namespace omnetpp;

namespace zwavenetwork {

class ZWaveController : public cSimpleModule
{
protected:
    double frequencyLow;
    double frequencyHigh;
    int numGates;
    bool sendCommands;
    int commandNode;
    std::string commandType;
    std::vector<int> connectedNodes;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    bool isValidGateIndex(int index) const;
    void sendCommand(int nodeId, const std::string& cmd);
    void processIncomingMessage(cMessage *msg, int nodeId);

};

Define_Module(ZWaveController);

void ZWaveController::initialize()
{
    frequencyLow = par("frequencyLow").doubleValue();
    frequencyHigh = par("frequencyHigh").doubleValue();
    sendCommands = par("sendCommands").boolValue();
    commandNode = par("commandNode").intValue();
    commandType = par("commandType").stdstringValue();
    numGates = gateSize("gate");

    for (int i = 0; i < numGates; ++i) {
        if (gate("gate$o", i)->isConnected()) {
            connectedNodes.push_back(i);
        }
    }

    EV << "ZWave Controller initialized with FSK frequencies "
           << frequencyLow << " MHz and " << frequencyHigh << " MHz\n";
    EV << "Number of connected gates: " << numGates << "\n";
    EV << "Connected nodes: " << connectedNodes.size() << "\n";

    if (sendCommands && !connectedNodes.empty()) {
        scheduleAt(simTime() + 2, new cMessage("sendCommand"));
    }  else {
         // Schedule the first message (if not sending commands)
        cMessage *startMsg = new cMessage("start");
        scheduleAt(simTime() + uniform(0, 1), startMsg);
    }
}

bool ZWaveController::isValidGateIndex(int index) const
{
    return (index >= 0 && index < numGates && gate("gate$o", index)->isConnected());
}

void ZWaveController::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (strcmp(msg->getName(), "sendCommand") == 0) {  // Handle commands
            if (commandNode >= 0 && commandNode < connectedNodes.size()) {
                int gateIndex = connectedNodes[commandNode];
                sendCommand(gateIndex, commandType);
                EV << "Controller sent command '" << commandType
                   << "' to Node " << commandNode << " via gate " << gateIndex << "\n";
            }
            else {
                EV << "Invalid commandNode index\n";
            }
            scheduleAt(simTime() + 10, msg); // Reschedule command
        } else { // Handle FSK data messages

            int numMessagesToSend = 3;
            for (int i = 0; i < numMessagesToSend; i++) {
                int randomNode;
                int attempts = 0;
                int maxAttempts = 5;

                do {
                    randomNode = intuniform(0, numGates - 1);
                    attempts++;
                } while (!isValidGateIndex(randomNode) && attempts < maxAttempts);

                if (attempts >= maxAttempts) {
                    EV << "Could not find a valid gate after " << maxAttempts << " attempts\n";
                    continue;
                }

                int bitToSend = intuniform(0, 1);
                double chosenFrequency = (bitToSend == 0) ? frequencyLow : frequencyHigh;

                cMessage *dataMsg = new cMessage("data");
                dataMsg->addPar("frequency") = chosenFrequency;
                dataMsg->addPar("bit") = bitToSend;

                try {
                    send(dataMsg, "gate$o", randomNode);
                    EV << "Controller sent bit " << bitToSend << " to Node " << randomNode
                       << " with frequency " << chosenFrequency << " MHz\n";
                }
                catch (const cRuntimeError& e) {
                    EV << "Error sending message: " << e.what() << "\n";
                    delete dataMsg;
                }
            }
             cMessage *nextMsg = new cMessage("start"); // Reschedule FSK data
             scheduleAt(simTime() + exponential(1.0), nextMsg);
        }

    } else {
        int arrivalGate = msg->getArrivalGate()->getIndex();
        processIncomingMessage(msg, arrivalGate); // Process incoming messages
        delete msg;
    }
}


void ZWaveController::sendCommand(int nodeId, const std::string& cmd)
{
    if (nodeId >= 0 && nodeId < gateSize("gate")) {
        cMessage *cmdMsg = new cMessage(cmd.c_str());
        try {  // Add error handling
            send(cmdMsg, "gate$o", nodeId);
            EV << "Controller sending command '" << cmd << "' to Gate " << nodeId << "\n";
        } catch (cRuntimeError& e) {
            EV << "Error sending command: " << e.what() << "\n";
            delete cmdMsg;
        }
    }
}

void ZWaveController::processIncomingMessage(cMessage *msg, int nodeId) {
    int senderNodeId = nodeId;
    std::string messageName = msg->getName();

    if (messageName == "AlarmTriggered") {
        EV << "Processing Alarm Trigger from Node " << senderNodeId << "\n";
    } else if (messageName == "MotionDetected") {
        EV << "Processing Motion Detection from Node " << senderNodeId << "\n";
        sendCommand(0, "ON"); // Example action
    } else if (messageName == "LampOn" || messageName == "LampOff" ||
               messageName == "DoorLocked" || messageName == "DoorUnlocked") {
        EV << "Controller received acknowledgment: " << messageName
           << " from Node " << nodeId << "\n";
    } else {
        EV << "Controller received unexpected message: " << messageName << "\n";
    }

    // Only process frequency and bit for FSK data messages
    if (msg->hasPar("frequency")) {
        double receivedFrequency = msg->par("frequency").doubleValue();
        int receivedBit = (receivedFrequency == frequencyLow) ? 0 : 1;
        EV << "Received Frequency: " << receivedFrequency << " MHz, Bit: " << receivedBit << "\n";
    }
}

} // namespace zwavenetwork
