
using System.Runtime.CompilerServices;
using Linux.Bluetooth;
using Linux.Bluetooth.Extensions;

namespace nahkd123.PenGyro.Platform.Linux;

internal class BlueZModule : IModule
{
    private readonly Adapter adapter;
    private readonly Device device;
    private readonly IGattService1 service;
    private readonly GattCharacteristic configAttr;
    private readonly GattCharacteristic dataAttr;
    private bool reading = false;
    private ModuleConfig lastConfig;

    public ModuleIdentifier Id { get; }
    public ModuleConstants Constants { get; }

    private ModuleConfig Config
    {
        get
        {
            if (reading)
            {
                return lastConfig;
            }
            else
            {
                var config = Unsafe.ReadUnaligned<ModuleConfig>(ref configAttr.ReadValueAsync(TimeSpan.FromSeconds(1)).Result[0]);
                lastConfig = config;
                return config;
            }
        }
        set
        {
            lastConfig = value;
            var raw = new byte[Unsafe.SizeOf<ModuleConfig>()];
            Unsafe.WriteUnaligned(ref raw[0], value);
            configAttr.WriteValueAsync(raw, new Dictionary<string, object>()).Wait();
        }
    }

    public ushort DataRate
    {
        get => Config.DataRate;
        set
        {
            var newConfig = Config;
            newConfig.DataRate = value;
            Config = newConfig;
        }
    }

    public ushort AccelerometerRange
    {
        get => Config.AccelerometerRange;
        set
        {
            var newConfig = Config;
            newConfig.AccelerometerRange = value;
            Config = newConfig;
        }
    }

    public ushort GyroscopeRange
    {
        get => Config.GyroscopeRange;
        set
        {
            var newConfig = Config;
            newConfig.GyroscopeRange = value;
            Config = newConfig;
        }
    }

    public event EventHandler<ModuleRawData>? Raw;
    public event EventHandler<ModuleData>? Data;

    public BlueZModule(ModuleIdentifier id, Adapter adapter, Device device)
    {
        Id = id;

        this.adapter = adapter;
        this.device = device;
        service = device.GetServiceAsync(PenGyro.SERVICE_UUID).Result ?? throw new Exception("Device does not have PenGyro service");
        configAttr = service.GetCharacteristicAsync(PenGyro.CONFIG_UUID).Result ?? throw new Exception("Client is outdated (unable to find config attribute)");
        lastConfig = Unsafe.ReadUnaligned<ModuleConfig>(ref configAttr.ReadValueAsync(TimeSpan.FromSeconds(1)).Result[0]);
        dataAttr = service.GetCharacteristicAsync(PenGyro.DATA_UUID).Result ?? throw new Exception("Client is outdated (unable to find data attribute)");
        dataAttr.Value += OnData;
        using var constsAttr = service.GetCharacteristicAsync(PenGyro.CONSTANTS_UUID).Result ?? throw new Exception("Client is outdated (unable to find constants attribute)");
        Constants = Unsafe.ReadUnaligned<ModuleConstants>(ref constsAttr.ReadValueAsync(TimeSpan.FromSeconds(1)).Result[0]);

        Raw += (sender, raw) =>
        {
            Data?.Invoke(sender, new ModuleData
            {
                Raw = raw,
                TimeStep = Constants.TimeStep,
                AccelerometerRange = lastConfig.AccelerometerRange,
                GyroscopeRange = lastConfig.GyroscopeRange
            });
        };
    }

    public void SendCommand(ModuleCommand command)
    {
        if (reading) throw new Exception($"Cannot send command inside {nameof(IModule)}.{nameof(IModule.Raw)} event handler");
        while (Config.Command != ModuleCommand.Idle) Thread.Sleep(20);
        var newConfig = Config;
        newConfig.Command = command;
        Config = newConfig;
        while (Config.Command != command) Thread.Sleep(20);
    }

    public void Start()
    {
        dataAttr.StartNotifyAsync().Wait();
    }

    public void Stop()
    {
        dataAttr.StopNotifyAsync().Wait();
    }

    private Task OnData(GattCharacteristic sender, GattCharacteristicValueEventArgs args)
    {
        var raw = Unsafe.ReadUnaligned<ModuleRawData>(ref args.Value[0]);
        reading = true;
        Raw?.Invoke(this, raw);
        reading = false;
        return Task.CompletedTask;
    }

    public void Dispose()
    {
        configAttr.Dispose();
        dataAttr.Value -= OnData;
        dataAttr.Dispose();
        device.Dispose();
        adapter.Dispose();
    }
}