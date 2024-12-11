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
        options.GeneratorKind = GeneratorKind.CSharp;
        options.GenerateDefaultValuesForArguments = true;
        options.GenerateSequentialLayout = true;
        options.Verbose = true;
        options.UsePropertyDetectionHeuristics = true;

        var outputDir = Path.Combine(repoRoot, "OCIOSharp");
        options.OutputDir = outputDir;

        var module = options.AddModule("OpenColorIO");  

        var installDir = Path.Combine(repoRoot, "OpenColorIO", "install");
        var includeDir = Path.Combine(installDir, "include");

        // check if the include directory exists    
        if (!Directory.Exists(includeDir))
        {
            throw new DirectoryNotFoundException($"The include directory {includeDir} does not exist.");
        }

        module.IncludeDirs.Add(includeDir);

        var headerFile = Path.Combine(includeDir, "OpenColorIO", "OpenColorIO.h");

        // check if the header file exists
        if (!File.Exists(headerFile))
        {
            throw new FileNotFoundException($"The header file {headerFile} does not exist.");
        }

        module.Headers.Add(headerFile);
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

