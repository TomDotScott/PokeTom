
using System.Collections.Generic;
using System.IO;
using Sharpmake;

[Generate]
public class LuaProject : Project
{
    public LuaProject()
    {
        Name = "Lua";

        SourceRootPath = "lua-5.4.8\\src";

        AddTargets(
            new Target(
                Platform.win64,
                DevEnv.vs2022,
                Optimization.Debug |
                Optimization.Release |
                Optimization.Retail
            )
        );

        SourceFilesExclude.Add("lua.c");
        SourceFilesExclude.Add("luac.c");
    }

    [Configure]
    public void ConfigureAll(Configuration conf, Target target)
    {
        conf.ProjectPath = $"build_{target.DevEnv}";

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);

        conf.Output = Configuration.OutputType.Dll;

        conf.Defines.Add("LUA_BUILD_AS_DLL");
    }
}