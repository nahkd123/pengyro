using nahkd123.PenGyro.Platform;
using OpenTabletDriver.Plugin;
using OpenTabletDriver.Plugin.Attributes;
using OpenTabletDriver.Plugin.DependencyInjection;
using OpenTabletDriver.Plugin.Output;
using OpenTabletDriver.Plugin.Platform.Pointer;
using OpenTabletDriver.Plugin.Tablet;
using OpenTabletDriver.Plugin.Timing;

namespace nahkd123.PenGyro.OpenTabletDriver;

public abstract class PenGyroBaseFilter : IPositionedPipelineElement<IDeviceReport>, IDisposable
{
    private IModule? module;
    private double lastRotation;
    private double rotation;
    private bool injectRotation = false;
    private readonly HPETDeltaStopwatch stopwatch = new(true);

    public PipelinePosition Position => PipelinePosition.PreTransform;
    public event Action<IDeviceReport>? Emit;

    [Resolved]
    public IPressureHandler? PressureHandler { get; set; }
    protected IRotationHandler? RotationHandler => PressureHandler is IRotationHandler handler ? handler : null;

    [Property("Reverse rotation"), DefaultPropertyValue(false), ToolTip("Whether to reverse rotating direction")]
    public bool ReverseRotation { get; set; } = false;

    [Property("Interpolate duration (s)"), DefaultPropertyValue(0), ToolTip("The amount of time it takes to interpolate the value (use 0 to disable)")]
    public double InterpolateDurationSec { get; set; } = 0;
    protected TimeSpan InterpolateDuration => TimeSpan.FromSeconds(InterpolateDurationSec);

    [TabletReference]
    public required TabletReference Tablet { get; set; }

    [OnDependencyLoad]
    public void Init()
    {
        var platform = GetPlatform();
        var id = platform.GetAllModules().Cast<ModuleIdentifier?>().First();

        if (id == null)
        {
            Log.Write("PenGyro", "No modules connected, the filter will not have effect!", LogLevel.Warning);
            return;
        }

        try
        {
            module = platform.Open(id.Value);
            module.Data += OnData;
            module.Start();
            Log.Write("PenGyro", $"Connected to {module.Id}");
        }
        catch (Exception e)
        {
            Log.Exception(e, LogLevel.Warning);
        }
    }

    public abstract IPlatform GetPlatform();

    public double InterpolatedRotation
    {
        get
        {
            var now = stopwatch.Elapsed;

            if (now < InterpolateDuration)
            {
                var frac = now / InterpolateDuration;
                var delta = (rotation - lastRotation) % 360;
                if (delta > 180) delta -= 360;
                if (delta < -180) delta += 360;
                return ((lastRotation + delta * frac) % 360 + 360) % 360;
            }

            return rotation;
        }
    }

    public void Consume(IDeviceReport value)
    {
        if (value is ITabletReport && injectRotation)
        {
            if (value is IRotationReport rotationReport)
            {
                var maxRotation = Tablet.Properties.Specifications.Pen.MaxRotation!;
                rotationReport.Rotation = (uint)(InterpolatedRotation * maxRotation / 360.0);
            }
            else
            {
                RotationHandler?.SetRotation((float)(InterpolatedRotation / 360.0));
            }
        }

        Emit?.Invoke(value);
    }

    private void OnData(object? sender, ModuleData data)
    {
        double rotated = data.GyroscopeY * data.Delta.TotalSeconds;
        lastRotation = InterpolatedRotation;
        rotation = (rotation + (ReverseRotation ? -rotated : rotated)) % 360.0;
        if (rotation < 0) rotation += 360.0;
        injectRotation = true;
        stopwatch.Restart();
    }

    public void Dispose()
    {
        module?.Stop();
        module?.Dispose();
        Log.Write("PenGyro", "Disposed rotation inject filter");
    }
}
