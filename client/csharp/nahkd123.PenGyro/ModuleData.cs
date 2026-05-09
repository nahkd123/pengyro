namespace nahkd123.PenGyro;

public struct ModuleData
{
    public ModuleRawData Raw;
    public uint TimeStep;
    public ushort AccelerometerRange;
    public ushort GyroscopeRange;

    public readonly ulong Timestamp => (ulong)Raw.Timestamp * TimeStep;
    public readonly TimeSpan Delta => TimeSpan.FromMicroseconds((long)Raw.Delta * TimeStep);
    public readonly double AccelerometerX => Raw.AccelerometerX * (long)AccelerometerRange / 32767.0;
    public readonly double AccelerometerY => Raw.AccelerometerY * (long)AccelerometerRange / 32767.0;
    public readonly double AccelerometerZ => Raw.AccelerometerZ * (long)AccelerometerRange / 32767.0;
    public readonly double GyroscopeX => Raw.GyroscopeY * (long)GyroscopeRange / 32767.0;
    public readonly double GyroscopeY => Raw.GyroscopeY * (long)GyroscopeRange / 32767.0;
    public readonly double GyroscopeZ => Raw.GyroscopeY * (long)GyroscopeRange / 32767.0;
}