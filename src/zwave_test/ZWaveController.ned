package zwavenetwork.zwave_test;

simple ZWaveController
{
    parameters:
        double frequency @unit(MHz);
        double range @unit(m);
        double frequencyLow @unit(MHz);
        double frequencyHigh @unit(MHz);
        @display("i=device/antennatower");
    gates:
        inout gate[5];
}