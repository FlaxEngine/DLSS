#if FLAX_EDITOR

using System;
using FlaxEngine;
using FlaxEditor;
using FlaxEditor.Content;

namespace NVIDIA
{
    /// <summary>
    /// DLSS plugin for Editor.
    /// </summary>
    public sealed class DLSSEditor : EditorPlugin
    {
        private AssetProxy _assetProxy;

        /// <summary>
        /// Initializes a new instance of the <see cref="DLSSEditor"/> class.
        /// </summary>
        public DLSSEditor()
        {
            _description = new PluginDescription
            {
                Name = "DLSS",
                Category = "Rendering",
                Description = "DLSS is a revolutionary breakthrough in AI-powered graphics upscaling technology that massively boosts performance.",
                Author = "NVIDIA",
                RepositoryUrl = "https://github.com/FlaxEngine/DLSS",
                Version = new Version(2, 4),
            };
        }

        /// <inheritdoc />
        public override Type GamePluginType => typeof(DLSS);

        /// <inheritdoc />
        public override void InitializeEditor()
        {
            base.InitializeEditor();

            _assetProxy = new CustomSettingsProxy(typeof(DLSSSettings), "DLSS");
            Editor.ContentDatabase.Proxy.Add(_assetProxy);
        }

        /// <inheritdoc />
        public override void Deinitialize()
        {
            Editor.ContentDatabase.Proxy.Remove(_assetProxy);
            _assetProxy = null;

            base.Deinitialize();
        }
    }
}

#endif
