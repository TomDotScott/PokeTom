using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using Sharpmake;

[Generate]
public class PokeClone : Project
{
    public PokeClone()
    {
        Name = "PokeClone";

        SourceRootPath = "src";

        AddTargets(
            new Target(
                Platform.win64,
                DevEnv.vs2022,
                Optimization.Debug |
                Optimization.Release |
                Optimization.Retail
            )
        );

        SourceFiles.Add(Path.Combine(Constants.LIBS_PATH, "renenyffenegger", "base64.cpp"));
    }

    // Probably a bit overkill to have a class for this but it does the job
    class SFML_Dependency
    {
        public readonly string LibraryName;
        public readonly bool IsSFMLLibrary;

        public SFML_Dependency(string libraryName, bool isSFMLLibrary = false)
        {
            LibraryName = libraryName;

            IsSFMLLibrary = isSFMLLibrary;
        }
    }

    private void AddSfmlDependency(SFML_Dependency dependency, Configuration conf, Target target)
    {
        string libraryName = dependency.LibraryName;

        // Static debug SFML libraries end in -s-d.lib... Apart from sfml-main apparently... Yay...
        if (libraryName != "sfml-main")
        {
            if (dependency.IsSFMLLibrary)
            {
                libraryName += "-s";

                if (target.Optimization == Optimization.Debug)
                {
                    libraryName += "-d";
                }
            }
        }

        libraryName += ".lib";
        conf.LibraryFiles.Add(libraryName);
    }

    [Configure]
    public void ConfigureAll(Configuration conf, Target target)
    {
        conf.ProjectPath = $"build_{target.DevEnv}";

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);
        conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);

        conf.AddPublicDependency<LuaProject>(target);

        // SFML Paths
        conf.IncludePaths.Add(Path.Combine(Constants.LIBS_PATH, "SFML", "include"));
        conf.LibraryPaths.Add(Path.Combine(Constants.LIBS_PATH, "SFML", "lib"));

        // This sucks. Maybe a better data structure would work but I am fed up with Sharpmake
        var sfmlDependencies = new Dictionary<SFML_Dependency, List<SFML_Dependency>>
        {
            { new("sfml-main", true), new List<SFML_Dependency>() },
            { new("sfml-graphics", true), new List<SFML_Dependency>{ new("sfml-window", true), new("sfml-system", true), new("opengl32"), new("freetype") } },
            { new("sfml-window", true), new List<SFML_Dependency>{ new("sfml-system", true), new("winmm"), new("gdi32") } },
            { new("sfml-audio", true), new List<SFML_Dependency>{ new("sfml-system", true), new("flac"), new("vorbisenc"), new("vorbisfile"), new("vorbis"), new("ogg") } },
            { new("sfml-network", true), new List<SFML_Dependency>{ new("sfml-system", true), new("ws2_32") } },
            { new("sfml-system", true), new List<SFML_Dependency>{ new("winmm") } }
        };

        foreach (var pair in sfmlDependencies)
        {
            AddSfmlDependency(pair.Key, conf, target);

            foreach (var dependency in pair.Value)
            {
                AddSfmlDependency(dependency, conf, target);
            }
        }

        // Zlib must be linked statically to avoid a zlib.dll runtime dependency.
        // Do NOT switch to zlib.lib (dynamic import lib) as this will cause 0xc000007b
        // on machines where zlib.dll is not present or is the wrong architecture.
        conf.IncludePaths.Add(Path.Combine(Constants.LIBS_PATH, "ZLib", "include"));
        conf.LibraryPaths.Add(Path.Combine(Constants.LIBS_PATH, "ZLib", "lib"));
        conf.LibraryFiles.Add("zlibstatic.lib");

        conf.IncludePaths.Add(Path.Combine(Constants.LIBS_PATH, "nlohmann"));

        conf.IncludePaths.Add(Path.Combine(Constants.LIBS_PATH, "renenyffenegger"));

        conf.IncludePaths.Add(Path.Combine(Constants.LIBS_PATH, "hoxml"));

        conf.IncludePaths.Add(Path.Combine(Constants.LIBS_PATH, "magic_enum"));

        conf.IncludePaths.Add(Path.Combine(Constants.LIBS_PATH, "lua", "lua-5.4.8", "src"));
        conf.LibraryPaths.Add(Path.Combine(Constants.LIBS_PATH, "lua", $"build_{target.DevEnv}", "output", target.Platform.ToString(), target.Optimization.ToString()));

        conf.IncludePaths.Add(Path.Combine(Constants.LIBS_PATH, "sol2", "single", "single", "include"));

        if (target.Optimization == Optimization.Debug)
        {
            conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);

            conf.Options.Add(Sharpmake.Options.Vc.Linker.SubSystem.Console);

            conf.Defines.Add("BUILD_DEBUG");
        }
        else
        {
            conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);

            if (target.Optimization == Optimization.Release)
            {
                conf.Options.Add(Sharpmake.Options.Vc.Linker.SubSystem.Console);

                conf.Defines.Add("BUILD_RELEASE");
            }
            else if (target.Optimization == Optimization.Retail)
            {
                conf.Options.Add(Sharpmake.Options.Vc.Linker.SubSystem.Windows);
                conf.Defines.Add("BUILD_MASTER");
            }
        }

        conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);

        // **Important for static SFML**
        conf.Defines.Add("SFML_STATIC");

        conf.VcxprojUserFile = new Configuration.VcxprojUserFileSettings
        {
            LocalDebuggerWorkingDirectory = Path.Combine(Constants.PROJECT_ROOT, "data")
        };
    }
}

[Generate]
public class PokeCloneSolution : Solution
{
    public PokeCloneSolution()
    {
        Name = "SFML_Pokemon_Clone";

        AddTargets(
            new Target(
                Platform.win64,
                DevEnv.vs2022,
                Optimization.Debug |
                Optimization.Release |
                Optimization.Retail
            )
        );
    }

    [Configure]
    public void ConfigureAll(Configuration conf, Target target)
    {
        conf.SolutionPath = "[solution.SharpmakeCsPath]";

        conf.AddProject<LuaProject>(target);
        conf.AddProject<PokeClone>(target);
    }
}
