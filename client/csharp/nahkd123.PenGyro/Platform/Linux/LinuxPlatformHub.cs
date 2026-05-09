
namespace nahkd123.PenGyro.Platform.Linux;

internal class LinuxPlatformHub : IPlatform
{
    public PlatformType Type => PlatformType.Linux;
    private readonly BlueZPlatform blePlatform = new();
    private IEnumerable<IPlatform> AllPlatforms => [blePlatform];

    public IEnumerable<ModuleIdentifier> GetAllModules() => AllPlatforms.SelectMany(p => p.GetAllModules());

    public IModule Open(ModuleIdentifier id) => id.Transport switch
    {
        TransportType.BLE => blePlatform.Open(id),
        _ => throw new Exception($"Unsupported transport: {id.Transport}")
    };

    public void Dispose()
    {
        blePlatform.Dispose();
    }
}