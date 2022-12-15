// This code was auto-generated. Do not modify it.

#include "Engine/Scripting/BinaryModule.h"
#include "DLSS.Gen.h"

StaticallyLinkedBinaryModuleInitializer StaticallyLinkedBinaryModuleDLSS(GetBinaryModuleDLSS);

extern "C" BinaryModule* GetBinaryModuleDLSS()
{
    static NativeBinaryModule module("DLSS", MAssemblyOptions());
    return &module;
}
