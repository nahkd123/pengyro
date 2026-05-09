namespace nahkd123.PenGyro;

public struct ModuleIdentifier
{
    public TransportType Transport;
    public string AdapterId;
    public string ModuleId;

    public override readonly string ToString() => $"{Transport}/{AdapterId}/{ModuleId}";
}