# Deep Learning Super Sampling (DLSS) for Flax Engine

[DLSS](https://www.nvidia.com/en-us/geforce/technologies/dlss/) is a revolutionary breakthrough in AI-powered graphics upscaling technology that massively boosts performance. This repository contains a plugin project for [Flax Engine](https://flaxengine.com/) games with DLSS.

Minimum supported Flax version: `1.5`.

## Installation

1. Clone repo into `<game-project>\Plugins\DLSS`

2. Add reference to DLSS project in your game by modyfying your game `<game-project>.flaxproj` as follows:


```
...
"References": [
    {
        "Name": "$(EnginePath)/Flax.flaxproj"
    },
    {
        "Name": "$(ProjectPath)/Plugins/DLSS/DLSS.flaxproj"
    }
]
```

3. Setup plugin

Now, open Flax Editor and add new `DLSS` settings (via *Content* window **New -> Settings**, select Type to `DLSSSettings`). The you can provide *AppId* or *ProjectId* for NVIDIA DLSS. Also, you might want to get [official DLSS SDK](https://developer.nvidia.com/rtx/dlss/get-started) and update DLL files in this repo.

4. Test it out!

Finally you can use DLSS for image upscaling. DLSS extension will be visible in Plugins window (under Rendering category). It implements `CustomUpscale` postFx to increase visual quality when using low-res rendering. To test it simply start the game and adjust the **Rendering Percentage** property in *Graphics Quality Window*. Use plugin API to adjust DLSS quality and setup proper `RenderingPercentage` of the game rendering based on optimal settings from NVIDIA.

## Supported Platforms

Platforms:
* Windows x64
* Linux x64 (untested)

Graphics APIs:
* DirectX 11
* DirectX 12
* Vulkan (untested)

## API

In order to access DLSS API in C++/C# scripting import it in your code module (modify `Game.Build.cs` file):

```cs
public override void Setup(BuildOptions options)
{
    base.Setup(options);
    
    // Adds DLSS module to PrivateDependencies if supported on current platform (eg. Windows/Linux x64)
    DLSS.ConditionalImport(options, options.PrivateDependencies);
}
```

DLSS is implemented as a `GamePlugin` and can be accessed as follows to configure effect:

```cs
// Get DLSS plugin
var dlss = PluginManager.GetPlugin<DLSS>();

// Check if DLSS is not supported
if (dlss.Support != DLSSSupport.Supported)
    return;

// Adjust quality and sharpness
dlss.Quality = DLSSQuality.UltraQuality;
dlss.Sharpness = 0.1f;

// Use automatic settings based on output resolution and quality
// Sets main render task RenderingPercentage and DLSS Sharpness
dlss.ApplyRecommendedSettings(DLSSQuality.UltraPerformance);

// Enable/disable effect
dlss.PostFx.Enabled = true;
```

## License

See official [NVIDIA DLSS License](https://github.com/NVIDIA/DLSS/blob/main/LICENSE.txt).

## Trademarks

NVIDIA and the NVIDIA logo are trademarks and/or registered trademarks of NVIDIA Corporation in the
U.S. and other countries. Other company and product names may be trademarks of the respective
companies with which they are associated.
