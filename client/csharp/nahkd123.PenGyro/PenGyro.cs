using nahkd123.PenGyro.Platform;
using nahkd123.PenGyro.Platform.Linux;

namespace nahkd123.PenGyro;

public static class PenGyro
{
    public const string SERVICE_UUID = "217a5545-9217-403b-a2af-af04cf7fad88";
    public const string CONFIG_UUID = "ee8cf7e0-a370-4fb2-9d16-d1e31ac66051";
    public const string CONSTANTS_UUID = "f1563870-f4e5-41ba-8165-7954f2513905";
    public const string DATA_UUID = "d312a6d2-2375-48fb-a333-c413144bc6c8";
    public const string ROTATION_UUID = "270b1d88-ac32-4658-99d3-babd43a2db93";

    public static IPlatform GetPlatform(PlatformType type) => type switch
    {
        PlatformType.Linux => new LinuxPlatformHub(),
        _ => throw new Exception($"Unsupported platform: {type}")
    };
}