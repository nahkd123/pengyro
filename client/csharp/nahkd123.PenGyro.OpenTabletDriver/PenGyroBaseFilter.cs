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
    private double bias;
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

    [Property("Interpolation curve"), DefaultPropertyValue(1), ToolTip("The exponent part (y) of x^y curve for interpolation")]
    public double InterpolationCurve { get; set; } = 1;

    [Property("Rotation bias (deg/report)"), DefaultPropertyValue(0), ToolTip("The amount of rotation bias to add on each report")]
    public double RotationBias { get; set; } = 0;

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
            module.Rotation += OnRotation;
            module.StartRotation();
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
                var frac = Math.Pow(now / InterpolateDuration, InterpolationCurve);
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

    private void OnRotation(object? sender, double nextRotation)
    {
        bias = Wrap(bias + RotationBias);
        lastRotation = InterpolatedRotation;
        rotation = Wrap((ReverseRotation ? -nextRotation : nextRotation) + bias);
        if (rotation < 0) rotation += 360.0;
        injectRotation = true;
        stopwatch.Restart();
    }

    private static double Wrap(double v) => ((v % 360.0) + 360.0) % 360.0;

    public void Dispose()
    {
        module?.StopRotation();
        module?.Dispose();
        Log.Write("PenGyro", "Disposed rotation inject filter");
    }
}
