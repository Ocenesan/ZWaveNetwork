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
        @display("bgb=$playgroundSizeX,$playgroundSizeY");
    submodules:
        controller: ZWaveController {
            @display("p=50,50;i=device/antennatower");
        }
        node[numNodes]: ZWaveNode {
            @display("p=,,m,0.5,0.5;is=s");
        }
    connections allowunconnected:
}