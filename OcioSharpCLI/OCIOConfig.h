// OcioGpuProcessorWrapper.h
#pragma once

#include <OpenColorIO/OpenColorIO.h>

using namespace System;
using namespace System::Runtime::InteropServices;
namespace OCIO = OCIO_NAMESPACE;

namespace OcioSharpCLI {

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
        OCIO::ConstConfigRcPtr* config;
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
                config = new OCIO::ConstConfigRcPtr(
                    OCIO::Config::CreateFromFile(static_cast<const char*>(configPathPtr.ToPointer())));
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

        void CreateProcessor(String^ inputColorSpace, String^ display, String^ view)
        {
            if (config == nullptr)
            {
                throw gcnew Exception("Configuration is not loaded.");
            }

            auto inputColorSpacePtr = Marshal::StringToHGlobalAnsi(inputColorSpace);
            auto displayPtr = Marshal::StringToHGlobalAnsi(display);
            auto viewPtr = Marshal::StringToHGlobalAnsi(view);

            try
            {
                processor = new OCIO::ConstProcessorRcPtr(
                    (*config)->getProcessor(
                        static_cast<const char*>(inputColorSpacePtr.ToPointer()),
                        static_cast<const char*>(displayPtr.ToPointer()),
                        static_cast<const char*>(viewPtr.ToPointer()),
                        OCIO::TRANSFORM_DIR_FORWARD));

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
    };
}
