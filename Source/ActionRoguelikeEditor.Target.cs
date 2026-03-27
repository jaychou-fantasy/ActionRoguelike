// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ActionRoguelikeEditorTarget : TargetRules
{
    public ActionRoguelikeEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;

        //使用 UE 5.7 的新默认构建规则
        DefaultBuildSettings = BuildSettingsVersion.V6;

        //使用新的 Include 顺序
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

        ExtraModuleNames.AddRange(new string[] { "ActionRoguelike" });
    }
}
