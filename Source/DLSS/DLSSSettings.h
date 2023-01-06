#pragma once

#include "Engine/Core/Config/Settings.h"
#include "Engine/Scripting/ScriptingObject.h"

/// <summary>
/// The settings for NVIDIA DLSS plugin.
/// </summary>
API_CLASS(Namespace="NVIDIA") class DLSS_API DLSSSettings : public SettingsBase
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(DLSSSettings);
    DECLARE_SETTINGS_GETTER(DLSSSettings);

public:
    // App ID to pass for DLSS to identify the app. 0 if unused.
    API_FIELD(Attributes="EditorOrder(0)")
    uint32 AppId = 0;

    // Project ID to pass for DLSS to identify the project. Empty if unused.
    API_FIELD(Attributes="EditorOrder(10)")
    StringAnsi ProjectId;

    // If checked, DLSS initialization will be delayed until actually used.
    API_FIELD(Attributes="EditorOrder(100)")
    bool LazyInit = true;
};
