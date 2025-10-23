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

        arguments.Generate<PokeCloneSolution>();
    }
}