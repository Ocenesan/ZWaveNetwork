package zwavenetwork.zwave_test;
import ned.IdealChannel;

network ZWaveNetwork
{
    parameters:
        int numNodes = default(20);
        double frequency @unit(MHz) = default(908.42MHz);
        double range @unit(m) = default(30m);  // Approximate range for ZWave
        double playgroundSizeX @unit(m) = default(100m);
        double playgroundSizeY @unit(m) = default(100m);
        double frequencyLow @unit(MHz) = default(908.40MHz);
        double frequencyHigh @unit(MHz) = default(908.42MHz);
        @display("bgb=src/floorplan.png, $playgroundSizeX, $playgroundSizeY");
    
    submodules:
        controller: ZWaveController {
            @display("p=50,50;i=device/antennatower");
        }
        node[numNodes]: ZWaveNode {
            parameters:
                @display("is=s");
        }
 
    connections allowunconnected:
        for i=0..9 {
            controller.gate++ <--> IdealChannel <--> node[i].gate++;
        }
        
        node[0].gate++ <--> IdealChannel <--> node[1].gate++;
        node[1].gate++ <--> IdealChannel <--> node[2].gate++;
        node[2].gate++ <--> IdealChannel <--> node[3].gate++;
        node[3].gate++ <--> IdealChannel <--> node[4].gate++;
        node[4].gate++ <--> IdealChannel <--> node[5].gate++;
        node[5].gate++ <--> IdealChannel <--> node[6].gate++;
        node[6].gate++ <--> IdealChannel <--> node[7].gate++;
        node[7].gate++ <--> IdealChannel <--> node[8].gate++;
        node[8].gate++ <--> IdealChannel <--> node[9].gate++;
        node[9].gate++ <--> IdealChannel <--> node[0].gate++;
}
