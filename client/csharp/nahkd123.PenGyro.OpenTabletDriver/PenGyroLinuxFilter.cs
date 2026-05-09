using nahkd123.PenGyro.Platform;
using OpenTabletDriver.Plugin;
using OpenTabletDriver.Plugin.Attributes;

namespace nahkd123.PenGyro.OpenTabletDriver;

[PluginName("PenGyro Rotation Injector (Linux)"), SupportedPlatform(PluginPlatform.Linux)]
public class PenGyroLinuxFilter : PenGyroBaseFilter
{
    public override IPlatform GetPlatform() => PenGyro.GetPlatform(PlatformType.Linux);
}