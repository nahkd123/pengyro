using System.Runtime.InteropServices;

namespace nahkd123.PenGyro;

[StructLayout(LayoutKind.Sequential)]
public struct ModuleConstants
{
    /// <summary>
    /// The maximum (inclusive) value for absolute time.
    /// </summary>
    public uint MaxTicks;

    /// <summary>
    /// The duration of time for each tick, measured in microseconds (us).
    /// </summary>
    public uint TimeStep;
}