namespace nahkd123.PenGyro.Platform;

public interface IPlatform : IDisposable
{
    PlatformType Type { get; }

    /// <summary>
    /// Get the identifiers of all paired modules.
    /// </summary>
    IEnumerable<ModuleIdentifier> GetAllModules();

    /// <summary>
    /// Open module from given identifier.
    /// </summary>
    IModule Open(ModuleIdentifier id);
}