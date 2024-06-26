package vendor.xiaomi.hardware.stylus;

@VintfStability
interface IStylusControl {
    int getPenStatus();
    boolean PenControl(in boolean enable);
}