using System.IO;
using Flax.Build;
using Flax.Build.NativeCpp;

public class DLSS : GameModule
{
    /// <summary>
    /// Conditionally imports module into the build depending on the target platform support.
    /// </summary>
    /// <param name="options">The build options.</param>
    /// <param name="dependencies">Output dependencies list to import to.</param>
    /// <returns>True if module was imported, otherwise false.</returns>
    public static bool ConditionalImport(BuildOptions options, System.Collections.Generic.List<string> dependencies)
    {
        bool result = false;
        switch (options.Platform.Target)
        {
        case TargetPlatform.Windows:
            switch (options.Architecture)
            {
            case TargetArchitecture.x64:
                result = true;
                break;
            }
            break;
        case TargetPlatform.Linux:
            switch (options.Architecture)
            {
            case TargetArchitecture.x64:
                result = true;
                break;
            }
            break;
        }
        if (result)
            dependencies.Add("DLSS");
        return result;
    }
    
    /// <inheritdoc />
    public override void Setup(BuildOptions options)
    {
        base.Setup(options);

        BuildNativeCode = true;

        // Link against DLSS library
        options.PrivateIncludePaths.Add(Path.Combine(FolderPath, "../ThirdParty/DLSS/include"));
        switch (options.Platform.Target)
        {
        case TargetPlatform.Windows:
            switch (options.Architecture)
            {
                case TargetArchitecture.x64:
                {
                    var libPath = Flax.Build.Utilities.RemovePathRelativeParts(Path.Combine(FolderPath, "../ThirdParty/DLSS/lib/Windows_x86_64"));
                    if (options.CompileEnv.UseDebugCRT)
                        options.LinkEnv.InputFiles.Add(Path.Combine(libPath, "x86_64/nvsdk_ngx_d_dbg.lib"));
                    else
                        options.LinkEnv.InputFiles.Add(Path.Combine(libPath, "x86_64/nvsdk_ngx_d.lib"));
                    if (options.Configuration == TargetConfiguration.Release)
                        options.DependencyFiles.Add(Path.Combine(libPath, "rel\\nvngx_dlss.dll"));
                    else
                        options.DependencyFiles.Add(Path.Combine(libPath, "dev\\nvngx_dlss.dll"));
                    break;
                }
                default:
                    throw new InvalidArchitectureException(options.Architecture);
            }

            break;
        case TargetPlatform.Linux:
            switch (options.Architecture)
            {
                case TargetArchitecture.x64:
                {
                    var libPath = Path.Combine(FolderPath, "../ThirdParty/DLSS/lib/Linux_x86_64");
                    options.LinkEnv.InputFiles.Add(Path.Combine(libPath, "libnvsdk_ngx.a"));
                    if (options.Configuration == TargetConfiguration.Release)
                        options.DependencyFiles.Add(Path.Combine(libPath, "rel/libnvidia-ngx-dlss.so.2.4.0"));
                    else
                        options.DependencyFiles.Add(Path.Combine(libPath, "dev/libnvidia-ngx-dlss.so.2.4.0"));
                    break;
                }
                default:
                    throw new InvalidArchitectureException(options.Architecture);
            }

            break;
        default:
            throw new InvalidPlatformException(options.Platform.Target);
        }

        // Use optionally Vulkan support (Vulkan SDK is required)
        if (VulkanSdk.Instance.IsValid)
        {
            options.PrivateDefinitions.Add("GRAPHICS_API_VULKAN");
            options.PrivateDependencies.Add("volk");
        }
    }
}
