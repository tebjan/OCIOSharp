using CppSharp;
namespace OCIOSharp;
class Program
{
    static void Main(string[] args)
    {
        var thisExePath = System.Reflection.Assembly.GetExecutingAssembly().Location;
        // we go up from the bin\framework\config folder to the root of the repo
        var repoRootDir = Path.GetFullPath(System.IO.Path.Combine(thisExePath, @"..\..\..\.."));
        var ocioBindings = new OCIOBindings(repoRootDir);
        ConsoleDriver.Run(ocioBindings);
        var s = ocioBindings.ToString();
    }
}