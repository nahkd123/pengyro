using nahkd123.PenGyro;
using nahkd123.PenGyro.Platform;

var platform = PenGyro.GetPlatform(PlatformType.Linux);
var ids = platform.GetAllModules();
foreach (var id in ids) Console.WriteLine($"Found module {id}");

var module = platform.Open(ids.Select(v => (ModuleIdentifier?)v).FirstOrDefault() ?? throw new Exception("No modules found"));
module.GyroscopeRange = 1000;
module.Rotation += (_, rotation) => Console.WriteLine($"{module.Id} => Rotation is {rotation:F2}");
module.StartRotation();
new Thread(() => Console.ReadLine()).Start();