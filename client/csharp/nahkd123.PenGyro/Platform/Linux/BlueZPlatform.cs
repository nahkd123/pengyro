
using Linux.Bluetooth;
using Linux.Bluetooth.Extensions;

namespace nahkd123.PenGyro.Platform.Linux;

internal class BlueZPlatform : IPlatform
{
    public PlatformType Type => PlatformType.Linux;

    public BlueZPlatform()
    {
    }

    public IEnumerable<ModuleIdentifier> GetAllModules()
    {
        List<ModuleIdentifier> ids = [];

        foreach (var adapter in BlueZManager.GetAdaptersAsync().Result)
        {
            foreach (var device in adapter.GetDevicesAsync().Result)
            {
                if (device.GetServiceAsync(PenGyro.SERVICE_UUID).Result != null)
                {
                    ids.Add(new ModuleIdentifier
                    {
                        Transport = TransportType.BLE,
                        AdapterId = adapter.Name,
                        ModuleId = device.GetAddressAsync().Result
                    });
                }

                device.Dispose();
            }

            adapter.Dispose();
        }

        return ids;
    }

    public IModule Open(ModuleIdentifier id)
    {
        var adapter = BlueZManager.GetAdapterAsync(id.AdapterId, true).Result ?? throw new Exception($"No adapter with ID {id.AdapterId}");
        var device = adapter.GetDeviceAsync(id.ModuleId).Result ?? throw new Exception($"No module with ID {id.ModuleId}");
        device.ConnectAsync().Wait();
        return new BlueZModule(id, adapter, device);
    }

    public void Dispose()
    {
    }
}