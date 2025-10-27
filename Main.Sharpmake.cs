using System.Diagnostics;
using System.IO;
using System.Runtime.CompilerServices;
using Sharpmake;

[module: Sharpmake.Include("Constants.cs")]
[module: Sharpmake.Include("Pokeclone.Sharpmake.cs")]

public static class Main
{
    private static string GetThisFilePath([CallerFilePath] string path = null)
    {
        return path;
    }


    [Sharpmake.Main]
    public static void SharpmakeMain(Sharpmake.Arguments arguments)
    {
        // Find the folder of this Csharp file
        Constants.PROJECT_ROOT = Path.GetDirectoryName(GetThisFilePath());

        Debug.Assert(Constants.PROJECT_ROOT != null, "Constants.PROJECT_ROOT != null");
        Constants.LIBS_PATH = Path.Combine(Constants.PROJECT_ROOT, "Lib");

        arguments.Generate<PokeCloneSolution>();
    }
}