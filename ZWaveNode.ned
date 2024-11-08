package zwavenetwork.zwave_test;
package zwavenetwork;

simple ZWaveNode
{
    parameters:
        double frequency @unit(MHz);
        double range @unit(m);
        double frequencyLow @unit(MHz);
        double frequencyHigh @unit(MHz);
        @display("i=device/wifilaptop");
    gates:
        inout gate[];
}
