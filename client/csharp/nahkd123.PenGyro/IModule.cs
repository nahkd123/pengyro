namespace nahkd123.PenGyro;

public interface IModule : IDisposable
{
    ModuleIdentifier Id { get; }
    ModuleConstants Constants { get; }
    ushort DataRate { get; set; }
    ushort AccelerometerRange { get; set; }
    ushort GyroscopeRange { get; set; }

    event EventHandler<ModuleRawData> Raw;
    event EventHandler<ModuleData> Data;

    void SendCommand(ModuleCommand command);
    void Start();
    void Stop();
}