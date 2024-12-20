using CppSharp;
using CppSharp.AST;
using CppSharp.Generators;
using CppSharp.Types;

public class OCIOBindings : ILibrary
{
    private string repoRoot;

    public OCIOBindings(string repoRootDir)
    {
        repoRoot = repoRootDir;
    }

    public void Setup(Driver driver)
    {
        var options = driver.Options;
        options.GeneratorKind = GeneratorKind.CLI;
        options.GenerateDefaultValuesForArguments = true;
        options.GenerateSequentialLayout = true;
        options.Verbose = true;
        options.UsePropertyDetectionHeuristics = true;

        var outputDir = Path.Combine(repoRoot, "OCIOSharpCLI");
        options.OutputDir = outputDir;

        var module = options.AddModule("OpenColorIO");  

        var installDir = Path.Combine(repoRoot, "OpenColorIO", "install");
        var includeDir = Path.Combine(installDir, "include", "OpenColorIO");

        // check if the include directory exists    
        if (!Directory.Exists(includeDir))
        {
            throw new DirectoryNotFoundException($"The include directory {includeDir} does not exist.");
        }

        module.IncludeDirs.Add(includeDir);

        var headerFile = Path.Combine(includeDir, "OpenColorIO.h");

        // check if the header file exists
        if (!File.Exists(headerFile))
        {
            throw new FileNotFoundException($"The header file {headerFile} does not exist.");
        }

        // include all header files
        //module.Headers.Add("OpenColorABI.h");
        //module.Headers.Add("OpenColorAppHelpers.h");
        module.Headers.Add("OpenColorIO.h");
        //module.Headers.Add("OpenColorTransforms.h");
        //module.Headers.Add("OpenColorTypes.h");

        var libDir = Path.Combine(installDir, "bin");
        var libFile = Path.Combine(libDir, "OpenColorIO_2_4.dll");

        // check if the library file exists
        if (!File.Exists(libFile))
        {
            throw new FileNotFoundException($"The library file {libFile} does not exist.");
        }

        module.LibraryDirs.Add(libDir);
        module.Libraries.Add("OpenColorIO_2_4.dll");
    }

    public void Preprocess(Driver driver, ASTContext ctx) { }

    public void Postprocess(Driver driver, ASTContext ctx) { }

    public void SetupPasses(Driver driver) { }
}

[TypeMap("sbyte**", GeneratorKind = GeneratorKind.CSharp)]
public class SBytePtrPtrToString : TypeMap
{
    public override CppSharp.AST.Type CSharpSignatureType(TypePrinterContext ctx)
    {
        return new CILType(typeof(IntPtr));
    }
}

