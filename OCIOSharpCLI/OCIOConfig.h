// OcioGpuProcessorWrapper.h
#pragma once

#include <OpenColorIO/OpenColorIO.h>
#include <OpenColorIO/OpenColorAppHelpers.h>
#include <set>
#include <string>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace OCIO = OCIO_NAMESPACE;

namespace OCIOSharpCLI {

    public value struct Display
    {
        String^ Name;
        array<String^>^ Views;
    };

    public value struct View
    {
        String^ Name;
        String^ Colorspace;
    };

    public enum class UniformType
    {
        Double = OCIO::UNIFORM_DOUBLE,
        Bool = OCIO::UNIFORM_BOOL,
        Float3 = OCIO::UNIFORM_FLOAT3,
        VectorFloat = OCIO::UNIFORM_VECTOR_FLOAT,
        VectorInt = OCIO::UNIFORM_VECTOR_INT,
        Unknown = OCIO::UNIFORM_UNKNOWN
    };

    public enum class InterpolationType
    {
        Unknown = OCIO::INTERP_UNKNOWN,
        Nearest = OCIO::INTERP_NEAREST,
        Linear = OCIO::INTERP_LINEAR,
        Tetrahedral = OCIO::INTERP_TETRAHEDRAL,
        Cubic = OCIO::INTERP_CUBIC,
        Default = OCIO::INTERP_DEFAULT,
        Best = OCIO::INTERP_BEST
    };

    public enum class TextureType
    {
		Unknown,
        Luma = OCIO::GpuShaderCreator::TextureType::TEXTURE_RED_CHANNEL,
		RGB = OCIO::GpuShaderCreator::TextureType::TEXTURE_RGB_CHANNEL,
    };

    public enum class TextureDimensions
    {
        Unknown,
		OneD = OCIO::GpuShaderCreator::TextureDimensions::TEXTURE_1D,
		TwoD = OCIO::GpuShaderCreator::TextureDimensions::TEXTURE_2D,
    };

    /// <summary>
    /// Linear output color spaces for view transform only processing.
    /// Use these when your pipeline applies gamma encoding separately (e.g., DX11 sRGB backbuffer).
    /// </summary>
    public enum class LinearOutputSpace
    {
        LinearRec709,      // sRGB/Rec.709 primaries, linear (for SDR with DX11 sRGB)
        LinearRec2020,     // Rec.2020 primaries, linear (for HDR scRGB)
        LinearP3D65,       // P3-D65 primaries, linear
        CIE_XYZ_D65        // Raw XYZ D65 output (user applies own matrix)
    };

    public value struct UniformInfo
    {
        String^ Name;
        UniformType Type;
        double DoubleValue;
        bool BoolValue;
        array<float>^ FloatValues;
        array<int>^ IntValues;
    };

    public value struct TextureInfo
    {
        String^ Name;
        String^ SamplerName;
        unsigned Width;
        unsigned Height;
        TextureType Channel;
        TextureDimensions Dimensions;
        InterpolationType Interpolation;
		array<float>^ Values;
    };

    public value struct Texture3DInfo
    {
        String^ Name;
        String^ SamplerName;
        unsigned EdgeLength;
        InterpolationType Interpolation;
		array<float>^ Values;
    };

    public ref class OCIOConfig
    {
    private:
        OCIO::ConfigRcPtr* config;  // Mutable config for adding linear displays
        OCIO::ConstProcessorRcPtr* processor;
        OCIO::ConstGPUProcessorRcPtr* gpuProcessor;
        OCIO::GpuShaderDescRcPtr* shaderDesc;

    public:
        OCIOConfig()
            : config(nullptr), processor(nullptr), gpuProcessor(nullptr), shaderDesc(nullptr)
        {
        }

        ~OCIOConfig()
        {
            this->!OCIOConfig();
        }

        !OCIOConfig()
        {
            delete config;
            delete processor;
            delete gpuProcessor;
            delete shaderDesc;
        }

        void LoadConfig(String^ configPath)
        {
            auto configPathPtr = Marshal::StringToHGlobalAnsi(configPath);
            try
            {
                auto constConfig = OCIO::Config::CreateFromFile(static_cast<const char*>(configPathPtr.ToPointer()));
                config = new OCIO::ConfigRcPtr(constConfig->createEditableCopy());

                // Add linear displays for DX11 sRGB workflows
                AddLinearDisplaysInternal();
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
            finally
            {
                Marshal::FreeHGlobal(configPathPtr);
            }
        }

        /// <summary>
        /// Load a built-in OCIO config by name.
        /// Automatically adds synthetic linear displays for SDR/HDR pipelines.
        /// Examples: "ocio://studio-config-latest", "ocio://cg-config-latest",
        /// "ocio://studio-config-v4.0.0_aces-v2.0_ocio-v2.5"
        /// </summary>
        void LoadBuiltinConfig(String^ configName)
        {
            auto configNamePtr = Marshal::StringToHGlobalAnsi(configName);
            try
            {
                auto constConfig = OCIO::Config::CreateFromBuiltinConfig(static_cast<const char*>(configNamePtr.ToPointer()));
                config = new OCIO::ConfigRcPtr(constConfig->createEditableCopy());

                // Automatically add linear displays for DX11 sRGB workflows
                AddLinearDisplaysInternal();
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
            finally
            {
                Marshal::FreeHGlobal(configNamePtr);
            }
        }

    private:
        /// <summary>
        /// Internal: Add a single linear display colorspace and display to the config.
        /// Copies views from ALL existing displays so HDR views are also available.
        /// </summary>
        void AddLinearDisplay(const char* csName, const char* displayName, const double* matrix)
        {
            // Create the display colorspace with from_display_reference transform
            auto cs = OCIO::ColorSpace::Create(OCIO::REFERENCE_SPACE_DISPLAY);
            cs->setName(csName);
            cs->setFamily("Display");
            cs->setEncoding("display-linear");
            cs->setDescription("Linear output in target primaries (no gamma encoding)");

            // Create matrix transform from CIE XYZ D65 to target primaries
            auto matrixTransform = OCIO::MatrixTransform::Create();
            matrixTransform->setMatrix(matrix);
            cs->setTransform(matrixTransform, OCIO::COLORSPACE_DIR_FROM_REFERENCE);

            (*config)->addColorSpace(cs);

            // Add Raw view (direct colorspace, no view transform)
            (*config)->addDisplayView(displayName, "Raw", "Raw", "");

            // Collect unique view transforms from ALL displays
            std::set<std::string> addedViews;
            int numDisplays = (*config)->getNumDisplaysAll();

            for (int d = 0; d < numDisplays; ++d)
            {
                const char* existingDisplay = (*config)->getDisplayAll(d);
                int numViews = (*config)->getNumViews(existingDisplay);

                for (int v = 0; v < numViews; ++v)
                {
                    const char* viewName = (*config)->getView(existingDisplay, v);
                    const char* vtName = (*config)->getDisplayViewTransformName(existingDisplay, viewName);

                    // Only add views that have a view transform and haven't been added yet
                    if (vtName && *vtName && addedViews.find(viewName) == addedViews.end())
                    {
                        (*config)->addDisplayView(displayName, viewName, vtName, csName, "", "", "");
                        addedViews.insert(viewName);
                    }
                }
            }
        }

        /// <summary>
        /// Internal: Add synthetic linear display colorspaces and displays to the config.
        /// These allow applying view transforms (tone mapping) with linear output,
        /// useful for pipelines where gamma is applied separately (e.g., DX11 sRGB backbuffer).
        /// </summary>
        void AddLinearDisplaysInternal()
        {
            if (config == nullptr)
                throw gcnew Exception("Configuration is not loaded.");

            try
            {
                // Exact OCIO matrices (from build_conversion_matrix_from_XYZ_D65 in ColorMatrixHelpers.cpp)
                // These match the values used by OCIO's builtin display transforms.

                // XYZ D65 -> Linear Rec.709/sRGB (from OCIO Rec.709 primaries)
                const double rec709[16] = {
                     3.240969941905, -1.537383177570, -0.498610760293, 0,
                    -0.969243636281,  1.875967501508,  0.041555057407, 0,
                     0.055630079697, -0.203976958889,  1.056971514243, 0,
                     0, 0, 0, 1
                };

                // XYZ D65 -> Linear Rec.2020
                const double rec2020[16] = {
                     1.716651187971, -0.355670783776, -0.253366281374, 0,
                    -0.666684351832,  1.616481236635,  0.015768545814, 0,
                     0.017639857445, -0.042770613258,  0.942103121235, 0,
                     0, 0, 0, 1
                };

                // XYZ D65 -> Linear P3-D65
                const double p3d65[16] = {
                     2.493496911941, -0.931383617919, -0.402710784451, 0,
                    -0.829488969562,  1.762664060318,  0.023624685842, 0,
                     0.035845830244, -0.076172389268,  0.956884524008, 0,
                     0, 0, 0, 1
                };

                // XYZ D65 -> Linear AdobeRGB (1998)
                const double adobeRgb[16] = {
                     2.041587903811, -0.565006974335, -0.344731350824, 0,
                    -0.969243636281,  1.875967501508,  0.041555057407, 0,
                     0.013444280632, -0.118362392231,  1.015148047721, 0,
                     0, 0, 0, 1
                };

                AddLinearDisplay("Linear Rec.709 (Display)", "Linear Rec.709 - Display", rec709);
                AddLinearDisplay("Linear Rec.2020 (Display)", "Linear Rec.2020 - Display", rec2020);
                AddLinearDisplay("Linear P3-D65 (Display)", "Linear P3-D65 - Display", p3d65);
                AddLinearDisplay("Linear AdobeRGB (Display)", "Linear AdobeRGB - Display", adobeRgb);
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
        }

    public:
        /// <summary>
        /// Get list of available built-in config names.
        /// </summary>
        static array<String^>^ GetBuiltinConfigNames()
        {
            try
            {
                const auto& registry = OCIO::BuiltinConfigRegistry::Get();
                size_t numConfigs = registry.getNumBuiltinConfigs();
                array<String^>^ names = gcnew array<String^>((int)numConfigs);

                for (size_t i = 0; i < numConfigs; ++i)
                {
                    names[(int)i] = gcnew String(registry.getBuiltinConfigName(i));
                }
                return names;
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
        }

        /// <summary>
        /// Merge another config into this config.
        /// New colorspaces from inputConfig are added to this config.
        /// </summary>
        void MergeConfig(OCIOConfig^ inputConfig)
        {
            if (config == nullptr || inputConfig->config == nullptr)
                throw gcnew Exception("Both configs must be loaded before merging.");

            try
            {
                auto params = OCIO::ConfigMergingParameters::Create();
                // Use PREFER_INPUT strategy - new colorspaces from input take priority
                params->setDefaultStrategy(OCIO::ConfigMergingParameters::STRATEGY_PREFER_INPUT);

                auto mergedConfig = OCIO::ConfigMergingHelpers::MergeConfigs(
                    params,
                    *config,                    // base config
                    *(inputConfig->config)      // input config to merge
                );

                // Replace current config with merged result (already editable from merge)
                delete config;
                config = new OCIO::ConfigRcPtr(mergedConfig);
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
        }

        array<Display>^ GetDisplays()
        {
            if (config == nullptr)
            {
                throw gcnew Exception("Configuration is not loaded.");
            }

			int numDisplays = (*config)->getNumDisplaysAll();
            array<Display>^ displays = gcnew array<Display>(numDisplays);

            for (int i = 0; i < numDisplays; ++i)
            {
                const char* displayName = (*config)->getDisplayAll(i);
                int numViews = (*config)->getNumViews(displayName);
                array<String^>^ views = gcnew array<String^>(numViews);

                for (int j = 0; j < numViews; ++j)
                {
                    views[j] = gcnew String((*config)->getView(displayName, j));
                }

                Display display;
                display.Name = gcnew String(displayName);
                display.Views = views;
                displays[i] = display;
            }
            return displays;
        }

        array<String^>^ GetColorSpaces()
        {
            if (config == nullptr)
            {
                throw gcnew Exception("Configuration is not loaded.");
            }

            //search ref tpye

			int numColorSpaces = (*config)->getNumColorSpaces(OpenColorIO_v2_5::SEARCH_REFERENCE_SPACE_ALL, OpenColorIO_v2_5::COLORSPACE_ACTIVE);
            array<String^>^ spaces = gcnew array<String^>(numColorSpaces);

            for (int i = 0; i < numColorSpaces; ++i)
            {
				const char* colorSpaceName = (*config)->getColorSpaceNameByIndex(i);
                spaces[i] = gcnew String(colorSpaceName);
            }
            return spaces;
        }

        array<String^>^ GetRoles()
        {
            if (config == nullptr)
                throw gcnew Exception("Configuration is not loaded.");

            int numRoles = (*config)->getNumRoles();
            array<String^>^ roles = gcnew array<String^>(numRoles);

            for (int i = 0; i < numRoles; ++i)
            {
                roles[i] = gcnew String((*config)->getRoleName(i));
            }

            return roles;
        }

        String^ GetRoleColorSpace(String^ role)
        {
            if (config == nullptr)
                throw gcnew Exception("Configuration is not loaded.");

            auto rolePtr = Marshal::StringToHGlobalAnsi(role);
            try
            {
                const char* cs = (*config)->getRoleColorSpace(static_cast<const char*>(rolePtr.ToPointer()));
                if (!cs || !*cs)
                    return nullptr;

                return gcnew String(cs);
            }
            finally
            {
                Marshal::FreeHGlobal(rolePtr);
            }
        }

        array<String^>^ GetLooks()
        {
            if (config == nullptr)
                throw gcnew Exception("Configuration is not loaded.");

            int numLooks = (*config)->getNumLooks();
            array<String^>^ looks = gcnew array<String^>(numLooks);

            for (int i = 0; i < numLooks; ++i)
            {
                looks[i] = gcnew String((*config)->getLookNameByIndex(i));
            }
            return looks;
        }

        void CreateProcessor(String^ inputColorSpace, String^ display, String^ view, bool inverse)
        {
            if (config == nullptr)
                throw gcnew Exception("Configuration is not loaded.");

            auto inputColorSpacePtr = Marshal::StringToHGlobalAnsi(inputColorSpace);
            auto displayPtr = Marshal::StringToHGlobalAnsi(display);
            auto viewPtr = Marshal::StringToHGlobalAnsi(view);

            try
            {
                const auto dir = inverse ? OCIO::TRANSFORM_DIR_INVERSE : OCIO::TRANSFORM_DIR_FORWARD;

                processor = new OCIO::ConstProcessorRcPtr(
                    (*config)->getProcessor(
                        static_cast<const char*>(inputColorSpacePtr.ToPointer()),
                        static_cast<const char*>(displayPtr.ToPointer()),
                        static_cast<const char*>(viewPtr.ToPointer()),
                        dir));

                gpuProcessor = new OCIO::ConstGPUProcessorRcPtr(processor->get()->getDefaultGPUProcessor());
                shaderDesc = new OCIO::GpuShaderDescRcPtr(OCIO::GpuShaderDesc::CreateShaderDesc());
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
            finally
            {
                Marshal::FreeHGlobal(inputColorSpacePtr);
                Marshal::FreeHGlobal(displayPtr);
                Marshal::FreeHGlobal(viewPtr);
            }
        }

        // ColorSpace->ColorSpace has no direction overload in OCIO.
        // Inverse is implemented by swapping src/dst.
        void CreateProcessor(String^ inputColorSpace, String^ outputColorSpace, bool inverse)
        {
            if (config == nullptr)
                throw gcnew Exception("Configuration is not loaded.");

            auto inputPtr = Marshal::StringToHGlobalAnsi(inputColorSpace);
            auto outputPtr = Marshal::StringToHGlobalAnsi(outputColorSpace);

            try
            {
                const char* src = static_cast<const char*>(inputPtr.ToPointer());
                const char* dst = static_cast<const char*>(outputPtr.ToPointer());

                if (inverse)
                {
                    // swap
                    auto tmp = src;
                    src = dst;
                    dst = tmp;
                }

                processor = new OCIO::ConstProcessorRcPtr((*config)->getProcessor(src, dst));
                gpuProcessor = new OCIO::ConstGPUProcessorRcPtr(processor->get()->getDefaultGPUProcessor());
                shaderDesc = new OCIO::GpuShaderDescRcPtr(OCIO::GpuShaderDesc::CreateShaderDesc());
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
            finally
            {
                Marshal::FreeHGlobal(inputPtr);
                Marshal::FreeHGlobal(outputPtr);
            }
        }

        /// <summary>
        /// Create a processor that applies only the view transform (tone mapping) with linear output.
        /// This is useful for pipelines that apply gamma encoding separately (e.g., DX11 sRGB backbuffer).
        /// The output is linear in the specified color primaries.
        /// </summary>
        void CreateViewTransformProcessor(String^ inputColorSpace, String^ display, String^ view, LinearOutputSpace outputSpace)
        {
            if (config == nullptr)
                throw gcnew Exception("Configuration is not loaded.");

            auto inputPtr = Marshal::StringToHGlobalAnsi(inputColorSpace);
            auto displayPtr = Marshal::StringToHGlobalAnsi(display);
            auto viewPtr = Marshal::StringToHGlobalAnsi(view);

            try
            {
                // Get view transform name for this display/view
                const char* vtName = (*config)->getDisplayViewTransformName(
                    static_cast<const char*>(displayPtr.ToPointer()),
                    static_cast<const char*>(viewPtr.ToPointer()));

                if (!vtName || !*vtName)
                    throw gcnew Exception("View transform not found for display/view combination.");

                auto group = OCIO::GroupTransform::Create();

                // 1. Input colorspace -> ACES2065-1 (scene reference)
                auto toReference = OCIO::ColorSpaceTransform::Create();
                toReference->setSrc(static_cast<const char*>(inputPtr.ToPointer()));
                toReference->setDst("ACES2065-1");
                group->appendTransform(toReference);

                // 2. Apply view transform (tone mapping) -> CIE XYZ-D65
                auto vt = (*config)->getViewTransform(vtName);
                if (!vt)
                    throw gcnew Exception("View transform not found.");
                auto vtTransform = vt->getTransform(OCIO::VIEWTRANSFORM_DIR_FROM_REFERENCE);
                if (vtTransform)
                    group->appendTransform(vtTransform->createEditableCopy());

                // 3. CIE XYZ-D65 -> Target linear primaries (matrix only, no gamma)
                if (outputSpace != LinearOutputSpace::CIE_XYZ_D65)
                {
                    auto matrix = OCIO::MatrixTransform::Create();
                    double m[16];

                    switch (outputSpace)
                    {
                    case LinearOutputSpace::LinearRec709:
                        // XYZ D65 -> Linear Rec.709/sRGB (exact OCIO values)
                        m[0] =  3.240969941905; m[1] = -1.537383177570; m[2] = -0.498610760293; m[3] = 0;
                        m[4] = -0.969243636281; m[5] =  1.875967501508; m[6] =  0.041555057407; m[7] = 0;
                        m[8] =  0.055630079697; m[9] = -0.203976958889; m[10] = 1.056971514243; m[11] = 0;
                        m[12] = 0;              m[13] = 0;              m[14] = 0;               m[15] = 1;
                        break;
                    case LinearOutputSpace::LinearRec2020:
                        // XYZ D65 -> Linear Rec.2020 (exact OCIO values)
                        m[0] =  1.716651187971; m[1] = -0.355670783776; m[2] = -0.253366281374; m[3] = 0;
                        m[4] = -0.666684351832; m[5] =  1.616481236635; m[6] =  0.015768545814; m[7] = 0;
                        m[8] =  0.017639857445; m[9] = -0.042770613258; m[10] = 0.942103121235; m[11] = 0;
                        m[12] = 0;              m[13] = 0;              m[14] = 0;               m[15] = 1;
                        break;
                    case LinearOutputSpace::LinearP3D65:
                        // XYZ D65 -> Linear P3-D65 (exact OCIO values)
                        m[0] =  2.493496911941; m[1] = -0.931383617919; m[2] = -0.402710784451; m[3] = 0;
                        m[4] = -0.829488969562; m[5] =  1.762664060318; m[6] =  0.023624685842; m[7] = 0;
                        m[8] =  0.035845830244; m[9] = -0.076172389268; m[10] = 0.956884524008; m[11] = 0;
                        m[12] = 0;              m[13] = 0;              m[14] = 0;               m[15] = 1;
                        break;
                    default:
                        break;
                    }
                    matrix->setMatrix(m);
                    group->appendTransform(matrix);
                }

                processor = new OCIO::ConstProcessorRcPtr((*config)->getProcessor(group));
                gpuProcessor = new OCIO::ConstGPUProcessorRcPtr(processor->get()->getDefaultGPUProcessor());
                shaderDesc = new OCIO::GpuShaderDescRcPtr(OCIO::GpuShaderDesc::CreateShaderDesc());
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
            finally
            {
                Marshal::FreeHGlobal(inputPtr);
                Marshal::FreeHGlobal(displayPtr);
                Marshal::FreeHGlobal(viewPtr);
            }
        }

        /// <summary>
        /// Create a processor using GroupTransform: optional LookTransform + DisplayViewTransform.
        /// Produces a single GPU shader for the entire chain.
        /// </summary>
        void CreateDisplayViewProcessor(String^ inputColorSpace, String^ display, String^ view, String^ look, bool inverse)
        {
            if (config == nullptr)
                throw gcnew Exception("Configuration is not loaded.");

            auto inputPtr = Marshal::StringToHGlobalAnsi(inputColorSpace);
            auto displayPtr = Marshal::StringToHGlobalAnsi(display);
            auto viewPtr = Marshal::StringToHGlobalAnsi(view);
            auto lookPtr = look != nullptr ? Marshal::StringToHGlobalAnsi(look) : IntPtr::Zero;

            try
            {
                const auto dir = inverse ? OCIO::TRANSFORM_DIR_INVERSE : OCIO::TRANSFORM_DIR_FORWARD;
                bool hasLook = look != nullptr && look->Length > 0;

                auto group = OCIO::GroupTransform::Create();
                group->setDirection(dir);

                if (hasLook)
                {
                    auto lt = OCIO::LookTransform::Create();
                    lt->setSrc(static_cast<const char*>(inputPtr.ToPointer()));
                    lt->setDst(static_cast<const char*>(inputPtr.ToPointer()));
                    lt->setLooks(static_cast<const char*>(lookPtr.ToPointer()));
                    group->appendTransform(lt);
                }

                auto dvt = OCIO::DisplayViewTransform::Create();
                dvt->setSrc(static_cast<const char*>(inputPtr.ToPointer()));
                dvt->setDisplay(static_cast<const char*>(displayPtr.ToPointer()));
                dvt->setView(static_cast<const char*>(viewPtr.ToPointer()));
                if (hasLook) dvt->setLooksBypass(true);
                group->appendTransform(dvt);

                processor = new OCIO::ConstProcessorRcPtr((*config)->getProcessor(group));
                gpuProcessor = new OCIO::ConstGPUProcessorRcPtr(processor->get()->getDefaultGPUProcessor());
                shaderDesc = new OCIO::GpuShaderDescRcPtr(OCIO::GpuShaderDesc::CreateShaderDesc());
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
            finally
            {
                Marshal::FreeHGlobal(inputPtr);
                Marshal::FreeHGlobal(displayPtr);
                Marshal::FreeHGlobal(viewPtr);
                if (lookPtr != IntPtr::Zero) Marshal::FreeHGlobal(lookPtr);
            }
        }

        String^ GetHLSLShader()
        {
            try
            {
                shaderDesc->get()->setLanguage(OCIO::GPU_LANGUAGE_HLSL_DX11);
                shaderDesc->get()->setFunctionName("OCIODisplay");
                shaderDesc->get()->setResourcePrefix("ocio_");

                // Extract GPU shader information
                gpuProcessor->get()->extractGpuShaderInfo(*shaderDesc);

                // Retrieve the shader code
                return gcnew String(shaderDesc->get()->getShaderText());
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
        }

        array<UniformInfo>^ GetUniforms()
        {
            try
            {
                unsigned numUniforms = shaderDesc->get()->getNumUniforms();
                array<UniformInfo>^ uniforms = gcnew array<UniformInfo>(numUniforms);

                for (unsigned i = 0; i < numUniforms; ++i)
                {
                    OCIO::GpuShaderDesc::UniformData data;
                    const char* name = shaderDesc->get()->getUniform(i, data);

                    UniformInfo uniformInfo;
                    uniformInfo.Name = gcnew String(name);
                    uniformInfo.Type = static_cast<UniformType>(data.m_type);

                    switch (data.m_type)
                    {
                    case OCIO::UNIFORM_DOUBLE:
                        uniformInfo.DoubleValue = data.m_getDouble();
                        break;
                    case OCIO::UNIFORM_FLOAT3:
                    {
                        uniformInfo.FloatValues = gcnew array<float>(3);
                        auto float3 = data.m_getFloat3();
                        for (int j = 0; j < 3; ++j)
                        {
                            uniformInfo.FloatValues[j] = float3[j];
                        }
                        break;
                    }
                    case OCIO::UNIFORM_VECTOR_FLOAT:
                    {
                        int size = data.m_vectorFloat.m_getSize();
                        uniformInfo.FloatValues = gcnew array<float>(size);
                        const float* values = data.m_vectorFloat.m_getVector();
                        for (int j = 0; j < size; ++j)
                        {
                            uniformInfo.FloatValues[j] = values[j];
                        }
                        break;
                    }
                    case OCIO::UNIFORM_VECTOR_INT:
                    {
                        int size = data.m_vectorInt.m_getSize();
                        uniformInfo.IntValues = gcnew array<int>(size);
                        const int* values = data.m_vectorInt.m_getVector();
                        for (int j = 0; j < size; ++j)
                        {
                            uniformInfo.IntValues[j] = values[j];
                        }
                        break;
                    }
                    default:
                        break;
                    }

                    uniforms[i] = uniformInfo;
                }
                return uniforms;
            }
            catch (OCIO::Exception &e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
        }

        array<float>^ CopyTextureValues(const float* sourceValues, int totalTexels, int channels)
        {
            if (channels == 1)
            {
                // Allocate space for single channel data
                array<float>^ textureValues = gcnew array<float>(totalTexels);
                for (int i = 0; i < totalTexels; ++i)
                {
                    textureValues[i] = sourceValues[i];
                }
                return textureValues;
            }
            else if (channels == 3)
            {
                // Allocate space for RGBA
                int rgbaElementCount = totalTexels * 4;
                array<float>^ textureValues = gcnew array<float>(rgbaElementCount);

                for (int i = 0; i < totalTexels; ++i)
                {
                    textureValues[i * 4 + 0] = sourceValues[i * 3 + 0]; // R
                    textureValues[i * 4 + 1] = sourceValues[i * 3 + 1]; // G
                    textureValues[i * 4 + 2] = sourceValues[i * 3 + 2]; // B
                    textureValues[i * 4 + 3] = 1.0f; // A default to 1.0
                }

                return textureValues;
            }

            throw gcnew Exception("Unsupported channel configuration");
        }

        array<TextureInfo>^ GetTextures()
        {
            try
            {
                unsigned numTextures = shaderDesc->get()->getNumTextures();
                array<TextureInfo>^ textures = gcnew array<TextureInfo>(numTextures);

                for (unsigned i = 0; i < numTextures; ++i)
                {
                    const char* textureName;
                    const char* samplerName;
                    unsigned width, height;
                    OCIO::GpuShaderCreator::TextureType channel;
                    OCIO::GpuShaderCreator::TextureDimensions dimensions;
                    OCIO::Interpolation interpolation;

                    shaderDesc->get()->getTexture(i, textureName, samplerName, width, height, channel, dimensions, interpolation);

                    TextureInfo textureInfo;
                    textureInfo.Name = gcnew String(textureName);
                    textureInfo.SamplerName = gcnew String(samplerName);
                    textureInfo.Width = width;
                    textureInfo.Height = height;
                    textureInfo.Channel = static_cast<TextureType>(channel);
                    textureInfo.Dimensions = static_cast<TextureDimensions>(dimensions);
                    textureInfo.Interpolation = static_cast<InterpolationType>(interpolation);

                    int channels = (channel == OCIO::GpuShaderCreator::TextureType::TEXTURE_RGB_CHANNEL) ? 3 : 1;
                    int totalTexels = width * height;

                    const float* values = nullptr;
                    shaderDesc->get()->getTextureValues(i, values);

                    if (values != nullptr)
                    {
                        textureInfo.Values = CopyTextureValues(values, totalTexels, channels);
                    }

                    textures[i] = textureInfo;
                }

                return textures;
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
        }

        array<Texture3DInfo>^ Get3DTextures()
        {
            try
            {
                unsigned num3DTextures = shaderDesc->get()->getNum3DTextures();
                array<Texture3DInfo>^ textures = gcnew array<Texture3DInfo>(num3DTextures);

                for (unsigned i = 0; i < num3DTextures; ++i)
                {
                    const char* textureName;
                    const char* samplerName;
                    unsigned edgelen;
                    OCIO::Interpolation interpolation;

                    shaderDesc->get()->get3DTexture(i, textureName, samplerName, edgelen, interpolation);

                    Texture3DInfo textureInfo;
                    textureInfo.Name = gcnew String(textureName);
                    textureInfo.SamplerName = gcnew String(samplerName);
                    textureInfo.EdgeLength = edgelen;
                    textureInfo.Interpolation = static_cast<InterpolationType>(interpolation);

                    int totalTexels = edgelen * edgelen * edgelen;

                    const float* values = nullptr;
                    shaderDesc->get()->get3DTextureValues(i, values);

                    if (values != nullptr)
                    {
                        textureInfo.Values = CopyTextureValues(values, totalTexels, 3);
                    }

                    textures[i] = textureInfo;
                }

                return textures;
            }
            catch (OCIO::Exception& e)
            {
                throw gcnew Exception(gcnew String(e.what()));
            }
        }

        /// <summary>
        /// Apply OCIO transform to float pixel array (CPU processing).
        /// </summary>
        /// <param name="pixelData">RGBA float array (length = width * height * 4)</param>
        /// <param name="width">Image width in pixels</param>
        /// <param name="height">Image height in pixels</param>
        void ApplyCPUTransform(array<float>^ pixelData, int width, int height)
        {
            if (processor == nullptr)
                throw gcnew Exception("No processor created. Call CreateProcessor first.");

            pin_ptr<float> pinnedData = &pixelData[0];

            // Get CPU processor (F32 in/out, default optimizations)
            auto cpuProc = processor->get()->getOptimizedCPUProcessor(
                OCIO::BIT_DEPTH_F32,
                OCIO::BIT_DEPTH_F32,
                OCIO::OPTIMIZATION_DEFAULT
            );

            // Create packed image descriptor (RGBA interleaved)
            OCIO::PackedImageDesc imgDesc(
                pinnedData,
                width,
                height,
                OCIO::CHANNEL_ORDERING_RGBA,
                OCIO::BIT_DEPTH_F32,
                sizeof(float),          // chanStride
                4 * sizeof(float),      // xStride
                OCIO::AutoStride        // yStride
            );

            // Apply transform in-place
            cpuProc->apply(imgDesc);
        }

        /// <summary>
        /// Apply OCIO transform with separate source and destination buffers (CPU).
        /// </summary>
        void ApplyCPUTransformSeparate(
            array<float>^ srcPixelData,
            array<float>^ dstPixelData,
            int width,
            int height)
        {
            if (processor == nullptr)
                throw gcnew Exception("No processor created.");

            if (srcPixelData->Length != dstPixelData->Length)
                throw gcnew Exception("Source and destination arrays must be same size.");

            pin_ptr<float> pinnedSrc = &srcPixelData[0];
            pin_ptr<float> pinnedDst = &dstPixelData[0];

            auto cpuProc = processor->get()->getOptimizedCPUProcessor(
                OCIO::BIT_DEPTH_F32,
                OCIO::BIT_DEPTH_F32,
                OCIO::OPTIMIZATION_DEFAULT
            );

            OCIO::PackedImageDesc srcDesc(
                pinnedSrc, width, height,
                OCIO::CHANNEL_ORDERING_RGBA,
                OCIO::BIT_DEPTH_F32,
                sizeof(float), 4 * sizeof(float), OCIO::AutoStride
            );

            OCIO::PackedImageDesc dstDesc(
                pinnedDst, width, height,
                OCIO::CHANNEL_ORDERING_RGBA,
                OCIO::BIT_DEPTH_F32,
                sizeof(float), 4 * sizeof(float), OCIO::AutoStride
            );

            cpuProc->apply(srcDesc, dstDesc);
        }

        /// <summary>
        /// Apply OCIO transform to a single RGBA pixel (CPU).
        /// </summary>
        /// <param name="pixel">4-element float array [R,G,B,A]</param>
        void ApplyCPUTransformPixel(array<float>^ pixel)
        {
            if (processor == nullptr)
                throw gcnew Exception("No processor created.");

            if (pixel->Length != 4)
                throw gcnew Exception("Pixel must have 4 elements (RGBA).");

            pin_ptr<float> pinnedPixel = &pixel[0];

            auto cpuProc = processor->get()->getOptimizedCPUProcessor(
                OCIO::BIT_DEPTH_F32,
                OCIO::BIT_DEPTH_F32,
                OCIO::OPTIMIZATION_DEFAULT
            );

            cpuProc->applyRGBA(pinnedPixel);
        }
    };
}
