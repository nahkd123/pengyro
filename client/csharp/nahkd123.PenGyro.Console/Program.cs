using nahkd123.PenGyro;
using nahkd123.PenGyro.Platform;

var platform = PenGyro.GetPlatform(PlatformType.Linux);
var ids = platform.GetAllModules();
foreach (var id in ids) Console.WriteLine($"Found module {id}");

var module = platform.Open(ids.Select(v => (ModuleIdentifier?)v).FirstOrDefault() ?? throw new Exception("No modules found"));
var rotation = 0.0;

module.Data += (sender, data) =>
{
    rotation += data.GyroscopeY * data.Delta.TotalSeconds;

    Console.WriteLine($"[{module.Id}]");
    Console.WriteLine($"  Sensor time:   {data.Timestamp}us");
    Console.WriteLine($"  Delta time:    +{data.Delta}");
    Console.WriteLine($"  Accelerometer: {data.AccelerometerX} / {data.AccelerometerY} / {data.AccelerometerZ}");
    Console.WriteLine($"  Gyroscope:     {data.GyroscopeX} / {data.GyroscopeY} / {data.GyroscopeZ}");
    Console.WriteLine($"  Rotation:      {rotation}deg");
};
module.Start();
new Thread(() => Console.ReadLine()).Start();