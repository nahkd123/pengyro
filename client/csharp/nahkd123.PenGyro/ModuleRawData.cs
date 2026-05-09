using System.Runtime.InteropServices;

namespace nahkd123.PenGyro;

[StructLayout(LayoutKind.Sequential)]
public struct ModuleRawData
{
    /// <summary>
    /// The timestamp of when this data has been captured, measured in number of ticks.
    /// </summary>
    public uint Timestamp;

    /// <summary>
    /// The delta between previous and this captured data, measured in number of ticks.
    /// </summary>
    public uint Delta;

    public short AccelerometerX;
    public short AccelerometerY;
    public short AccelerometerZ;
    public short GyroscopeX;
    public short GyroscopeY;
    public short GyroscopeZ;
}