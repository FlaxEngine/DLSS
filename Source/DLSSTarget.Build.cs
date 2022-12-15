using Flax.Build;

public class DLSSTarget : GameProjectTarget
{
    /// <inheritdoc />
    public override void Init()
    {
        base.Init();

        Platforms = new[]
        {
            TargetPlatform.Windows,
            TargetPlatform.Linux,
        };
        Architectures = new[]
        {
            TargetArchitecture.x64,
        };
        Modules.Add("DLSS");
    }
}
