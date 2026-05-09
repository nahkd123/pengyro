using System.Runtime.InteropServices;

namespace nahkd123.PenGyro;

[StructLayout(LayoutKind.Sequential)]
public struct ModuleConfig
{
    public ModuleCommand Command;
    public ushort DataRate;
    public ushort AccelerometerRange;
    public ushort GyroscopeRange;
}