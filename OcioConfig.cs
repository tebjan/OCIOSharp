using System.Collections.Generic;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;

namespace OCIOSharp;

public class OcioConfig
{
    [YamlMember(Alias = "ocio_profile_version")]
    public string OcioProfileVersion { get; set; }

    private string configFolder;

    [YamlMember(Alias = "search_path")]
    public string SearchPath { get; set; }

    [YamlMember(Alias = "strictparsing")]
    public bool StrictParsing { get; set; }

    [YamlMember(Alias = "luma")]
    public List<double> Luma { get; set; } // List of three double values representing luma coefficients

    [YamlMember(Alias = "description")]
    public string Description { get; set; }

    [YamlMember(Alias = "roles")]
    public Dictionary<string, string> Roles { get; set; }

    [YamlMember(Alias = "displays")]
    public Dictionary<string, List<View>> Displays { get; set; }

    [YamlMember(Alias = "active_displays")]
    public List<string> ActiveDisplays { get; set; }

    [YamlMember(Alias = "active_views")]
    public List<string> ActiveViews { get; set; }

    [YamlMember(Alias = "colorspaces")]
    public List<ColorSpace> Colorspaces { get; set; }

    public IReadOnlyDictionary<string, ColorSpace> ColorspaceDictionary;
    public IReadOnlyDictionary<string, string> ViewDictionary;

    private void BuildColorspaceDictionary()
    {
        var colorSpaceDictionary = new Dictionary<string, ColorSpace>();

        foreach (var colorSpace in Colorspaces)
        {
            colorSpaceDictionary.Add(colorSpace.Name, colorSpace);
        }

        ColorspaceDictionary = colorSpaceDictionary;

        var viewDictionary = new Dictionary<string, string>();

        foreach (var display in Displays)
        {
            foreach (var view in display.Value)
            {
                viewDictionary.Add(view.Name, view.Colorspace);
            }
        }

        ViewDictionary = viewDictionary;
    }

    public ColorSpace GetColorSpaceOfDisplay(string displayName)
    {
        var colorSpaceName = ViewDictionary[displayName];
        return ColorspaceDictionary[colorSpaceName];
    }

    public string GetLUTFilename(FileTransform fileTransform, out bool is3D)
    {
        var lutName = fileTransform.Src;
        var lutFolder = Path.Combine(configFolder, SearchPath);

        is3D = false;

        // Ensure the LUT folder exists
        if (!Directory.Exists(lutFolder))
        {
            return null;
        }

        // Search for all files in the LUT directory and subdirectories
        var files = Directory.GetFiles(lutFolder, lutName, SearchOption.AllDirectories);

        // Find the file with the matching name (ignoring the extension)
        foreach (var file in files)
        {
            if (Path.GetFileName(file) == lutName)
            {
                // Check if the LUT is 3D
                is3D = Is3DLUT(file);
                return file; // Return the full path to the file
            }
        }

        return null; // No match found
    }

    // Helper method to check if the file is 3D LUT
    private static bool Is3DLUT(string filePath)
    {
        var extension = Path.GetExtension(filePath).ToLower();
        if (extension == ".spi3d")
            return true;

        return false;
    }

    public static OcioConfig FromFile(string path)
    {
        // Read the OCIO configuration file
        var yaml = File.ReadAllText(path);

        var deserializer = new DeserializerBuilder()
            .WithNamingConvention(UnderscoredNamingConvention.Instance)
            .WithTagMapping("View", typeof(View))
            .WithTagMapping("ColorSpace", typeof(ColorSpace))
            .WithTagMapping("FileTransform", typeof(FileTransform))
            .WithTagMapping("GroupTransform", typeof(GroupTransform))
            .WithTagMapping("LogTransform", typeof(LogTransform))
            .WithTagMapping("ExponentTransform", typeof(ExponentTransform))
            .WithTagMapping("MatrixTransform", typeof(MatrixTransform))
            .WithTagMapping("ColorSpaceTransform", typeof(ColorSpaceTransform))
            .Build();

        var config = deserializer.Deserialize<OcioConfig>(yaml);

        config.BuildColorspaceDictionary();
        config.configFolder = Path.GetDirectoryName(path);

        return config;
    }
}

public class View
{
    [YamlMember(Alias = "name")]
    public string Name { get; set; }

    [YamlMember(Alias = "colorspace")]
    public string Colorspace { get; set; }
}

public class ColorSpace
{
    [YamlMember(Alias = "name")]
    public string Name { get; set; }

    [YamlMember(Alias = "family")]
    public string Family { get; set; }

    [YamlMember(Alias = "equalitygroup")]
    public string EqualityGroup { get; set; }

    [YamlMember(Alias = "bitdepth")]
    public string BitDepth { get; set; }

    [YamlMember(Alias = "description")]
    public string Description { get; set; }

    [YamlMember(Alias = "isdata")]
    public bool IsData { get; set; }

    [YamlMember(Alias = "allocation")]
    public string Allocation { get; set; }

    [YamlMember(Alias = "allocationvars")]
    public List<float> AllocationVars { get; set; }

    [YamlMember(Alias = "to_reference")]
    public Transform ToReference { get; set; }

    [YamlMember(Alias = "from_reference")]
    public Transform FromReference { get; set; }
}

public abstract class Transform
{
    [YamlMember(Alias = "type")]
    public string Type { get; set; }

    [YamlMember(Alias = "direction")]
    public DirectionType? Direction { get; set; }
}

public class FileTransform : Transform
{
    [YamlMember(Alias = "src")]
    public string Src { get; set; }

    [YamlMember(Alias = "interpolation")]
    public InterpolationType Interpolation { get; set; }
}

public class GroupTransform : Transform
{
    [YamlMember(Alias = "children")]
    public List<Transform> Children { get; set; }
}

public class LogTransform : Transform
{
    [YamlMember(Alias = "base")]
    public float Base { get; set; }
}

public class ExponentTransform : Transform
{
    [YamlMember(Alias = "value")]
    public List<float> Value { get; set; }
}

public class MatrixTransform : Transform
{
    [YamlMember(Alias = "matrix")]
    public List<float> Matrix { get; set; }

    [YamlMember(Alias = "offset")]
    public List<float> Offset { get; set; }
}

public class ColorSpaceTransform : Transform
{
    [YamlMember(Alias = "src")]
    public string Src { get; set; }

    [YamlMember(Alias = "dst")]
    public string Dst { get; set; }

    [YamlMember(Alias = "data_bypass")]
    public bool? DataBypass { get; set; }
}

public enum InterpolationType
{
    [YamlMember(Alias = "linear")]
    Linear,

    [YamlMember(Alias = "tetrahedral")]
    Tetrahedral
}

public enum DirectionType
{
    [YamlMember(Alias = "forward")]
    Forward,

    [YamlMember(Alias = "inverse")]
    Inverse
}
