// ----------------------------------------------------------------------------
// <auto-generated>
// This is autogenerated code by CppSharp.
// Do not edit this file or all your changes will be lost after re-generation.
// </auto-generated>
// ----------------------------------------------------------------------------
#pragma once

#include "CppSharp.h"
#include <OpenColorAppHelpers.h>

namespace OpenColorIO
{
    namespace OpenColorIO_v2_4
    {
        enum class SearchReferenceSpaceType;
        enum class TransformDirection;
        ref class ColorSpaceMenuHelper;
        ref class ColorSpaceMenuParameters;
        ref class LegacyViewingPipeline;
        ref class MixingColorSpaceManager;
        ref class MixingSlider;
    }
}

namespace OpenColorIO
{
    namespace OpenColorIO_v2_4
    {
        /// <summary>Parameters controlling which color spaces appear in menus.</summary>
        /// <remarks>
        /// <para>The ColorSpaceMenuHelper class is intended to be used by applications to get the list of items</para>
        /// <para>to show in color space menus.</para>
        /// <para>The ColorSpaceMenuParameters class is used to configure the behavior as needed for any given</para>
        /// <para>menu.  Here is the algorithm used to produce a list of &quot;items&quot; (or strings) that will appear in</para>
        /// <para>a menu:</para>
        /// <para>1)  Use setRole to identify a role that controls a given menu.  If the config has this role,</para>
        /// <para>then only that color space is returned.  The name is set to the color space name, the UIName</para>
        /// <para>is presented as &quot;&lt;role name&gt; (&lt;color space name&gt;)&quot;. It may be useful for the application to</para>
        /// <para>then grey-out the menu or otherwise indicate to the user that the value for this menu is not</para>
        /// <para>user selectable since it was pre-determined by the config.  If the config does not have that</para>
        /// <para>role, the algorithm continues to the remaining steps.</para>
        /// <para>2)  The IncludeColorSpaces, SearchReferenceSpaceType, and IncludeNamedTransforms parameters are</para>
        /// <para>used to identify a set of items from the config that are potential candidates for use in the</para>
        /// <para>menu, as follows:</para>
        /// <para>- IncludeColorSpaces: Set to true to include color spaces in the menu.</para>
        /// <para>- SearchReferenceSpaceType: Use this to control whether the menu should include all color</para>
        /// <para>spaces, only display color spaces, or only non-display color spaces.</para>
        /// <para>- IncludeNamedTransforms: Set to true to include named transforms in the menu.</para>
        /// <para>3)  The set of items from step 2 is then filtered in step 3 using the following parameters:</para>
        /// <para>- AppCategories: A list of strings specified by the application based on the purpose of</para>
        /// <para>the menu.  For example, if the menu is used to select a color space for importing an</para>
        /// <para>image, the application might specify the 'file-io' category, whereas if it is to select</para>
        /// <para>a working color space, it might specify the 'working-space' category.  Application</para>
        /// <para>developers should document what strings they are using for each menu so that config</para>
        /// <para>authors know what categories to use in their configs.  Alternatively, an application</para>
        /// <para>could let advanced users customize the string to use for a given menu in the</para>
        /// <para>application.</para>
        /// <para>- Encodings: A list of strings used to further refine the items selected from the</para>
        /// <para>AppCategories.  For example, an application might specify 'working-space' as the</para>
        /// <para>category and then specify 'scene-linear' as the encoding to only use items that have</para>
        /// <para>both of those properties (e.g., only select scene-linear working color spaces).</para>
        /// <para>- UserCategories: A list of strings specified by the end-user of the application.  OCIO</para>
        /// <para>will check for these strings in an environment variable, or they may be passed in from</para>
        /// <para>the application.</para>
        /// <para>Basically the intent is for the filtering to return the intersection of the app categories,</para>
        /// <para>encoding, and user categories.  However, some fall-backs are in place to ensure that the</para>
        /// <para>filtering does not remove all menu items.  Here is the detailed description:</para>
        /// <para>3a) The items from step 2 are filtered to generate a list of appItems containing only the ones</para>
        /// <para>that contain at least one of the AppCategories strings in their &quot;categories&quot; property and</para>
        /// <para>one of the encodings in their &quot;encoding&quot; property.   If this list is empty, an attempt is</para>
        /// <para>made to generate a non-empty appItems list by only filtering by AppCategories.  If that is</para>
        /// <para>empty, an attempt is made to only filter by Encodings.</para>
        /// <para>3b) The items from step 2 are filtered to generate a list of userItems containing only the ones</para>
        /// <para>that have at least one of the UserCategories strings in their &quot;categories&quot; property.</para>
        /// <para>3c) If both appItems and userItems are non-empty, a list of resultItems will be generated as</para>
        /// <para>the intersection of those two lists.</para>
        /// <para>3d) If the resultItems list is empty, the appList will be expanded by only filtering by</para>
        /// <para>AppCategories and not encodings.  The resultItems will be formed again as the intersection</para>
        /// <para>of the appItems and userItems.</para>
        /// <para>3e) If the resultItems is still empty, it will be set to just the appItems from step 3a.</para>
        /// <para>3f) If the resultItems is still empty, it will be set to just the userItems.</para>
        /// <para>3g) If the resultItems is still empty, the items are not filtered and all items from step 2 are</para>
        /// <para>returned.  The rationale is that if step 2 has produced any items, it is not acceptable for</para>
        /// <para>step 3 to remove all of them.  An application usually expects to have a non-zero number of</para>
        /// <para>items to display in the menu.  However, if step 2 produces no items (e.g. the application</para>
        /// <para>requests only named transforms and the config has no named transform), then no items will</para>
        /// <para>be returned.</para>
        /// <para>4)  If IncludeRoles is true, the items from step 3 are extended by including an item for each</para>
        /// <para>role.  The name is set to the role name, the UIName is presented as &quot;name&gt; (</para>
        /// <para>space name&gt;)&quot;, and the family is set to &quot;Roles&quot;.</para>
        /// <para>5)  If AddColorSpace has been used to add any additional items, these are appended to the final</para>
        /// <para>list.</para>
        /// </remarks>
        public ref class ColorSpaceMenuParameters : ICppInstance
        {
        public:

            [::System::Runtime::InteropServices::UnmanagedFunctionPointer(::System::Runtime::InteropServices::CallingConvention::Cdecl)] 
            delegate OpenColorIO::OpenColorIO_v2_4::SearchReferenceSpaceType Func_OpenColorIO_OpenColorIO_v2_4_SearchReferenceSpaceType___IntPtr(::System::IntPtr __instance);

            [::System::Runtime::InteropServices::UnmanagedFunctionPointer(::System::Runtime::InteropServices::CallingConvention::Cdecl)] 
            delegate void Action___IntPtr_OpenColorIO_OpenColorIO_v2_4_SearchReferenceSpaceType(::System::IntPtr __instance, OpenColorIO::OpenColorIO_v2_4::SearchReferenceSpaceType arg1);

            property class ::OpenColorIO_v2_4::ColorSpaceMenuParameters* NativePtr;
            property ::System::IntPtr __Instance
            {
                virtual ::System::IntPtr get();
                virtual void set(::System::IntPtr instance);
            }

            ColorSpaceMenuParameters(class ::OpenColorIO_v2_4::ColorSpaceMenuParameters* native);
            ColorSpaceMenuParameters(class ::OpenColorIO_v2_4::ColorSpaceMenuParameters* native, bool ownNativeInstance);
            static ColorSpaceMenuParameters^ __CreateInstance(::System::IntPtr native);
            static ColorSpaceMenuParameters^ __CreateInstance(::System::IntPtr native, bool __ownsNativeInstance);
            ColorSpaceMenuParameters();

            ColorSpaceMenuParameters(OpenColorIO::OpenColorIO_v2_4::ColorSpaceMenuParameters^ _0);

            ~ColorSpaceMenuParameters();

            property ::System::String^ Role
            {
                ::System::String^ get();
                void set(::System::String^);
            }

            property bool IncludeColorSpaces
            {
                bool get();
                void set(bool);
            }

            /// <summary>
            /// <para>Can be used to restrict the search using the ReferenceSpaceType of the color spaces.</para>
            /// <para>It has no effect on roles and named transforms.</para>
            /// </summary>
            property OpenColorIO::OpenColorIO_v2_4::SearchReferenceSpaceType SearchReferenceSpaceType
            {
                OpenColorIO::OpenColorIO_v2_4::SearchReferenceSpaceType get();
                void set(OpenColorIO::OpenColorIO_v2_4::SearchReferenceSpaceType);
            }

            property bool IncludeNamedTransforms
            {
                bool get();
                void set(bool);
            }

            property ::System::String^ AppCategories
            {
                ::System::String^ get();
                void set(::System::String^);
            }

            property ::System::String^ Encodings
            {
                ::System::String^ get();
                void set(::System::String^);
            }

            property ::System::String^ UserCategories
            {
                ::System::String^ get();
                void set(::System::String^);
            }

            property bool IncludeRoles
            {
                bool get();
                void set(bool);
            }

            property unsigned long long NumAddedColorSpaces
            {
                unsigned long long get();
            }

            /// <summary>Add an additional color space (or named transform) to the menu.</summary>
            /// <remarks>
            /// <para>Note that an additional color space could be:</para>
            /// <para>* an inactive color space,</para>
            /// <para>* an active color space not having at least one of the selected categories,</para>
            /// <para>* a newly created color space.</para>
            /// <para>Will throw when creating the menu if color space is not part of the config. Nothing is done</para>
            /// <para>if it is already part of the menu.</para>
            /// <para>It's ok to call this multiple times with the same color space, it will only be added to the</para>
            /// <para>menu once.  If a role name is passed in, the name in the menu will be the color space name</para>
            /// <para>the role points to.</para>
            /// </remarks>
            virtual void AddColorSpace(::System::String^ name);

            virtual ::System::String^ GetAddedColorSpace(unsigned long long index);

            virtual void ClearAddedColorSpaces();

        protected:

            bool __ownsNativeInstance;
        };

        /// <summary>Helper class to create menus for the content of a config.</summary>
        /// <remarks>
        /// <para>Menu can list color spaces, roles, named transforms. Each entry has a name, a UI name, a</para>
        /// <para>description, and a family. Family can also be accessed as hierarchy levels; levels are created</para>
        /// <para>by splitting the family using the 'family separator'. Hierarchy levels are meant to be used as</para>
        /// <para>sub-menus.</para>
        /// <para>The UI name is what is intended to be put in application menus seen by the end-user.   However,</para>
        /// <para>please note that the UI name is not guaranteed to remain stable between releases and so if</para>
        /// <para>applications need to save something it should be the 'name' rather than the 'UI name'.</para>
        /// <para>Currently, the only difference between the 'name' and 'UI name' is for roles.</para>
        /// <para>The overall ordering of items is: color spaces, named transforms, roles, and additional color</para>
        /// <para>spaces.  The display color spaces will either come before or after the other color spaces based</para>
        /// <para>on where that block of spaces appears in the config.  The order of items returned by the menu</para>
        /// <para>helper preserves the order of items in the config itself for each type of elements, thus</para>
        /// <para>preserving the intent of the config author.  For example, if you call getName at idx</para>
        /// <para>and idx+1, the name returned at idx+1 will be from farther down in the config than the one at</para>
        /// <para>idx as long as both are of the same type.  (An application may ask for only the items in one</para>
        /// <para>of those blocks if it wants to handle them separately.)  If the application makes use of</para>
        /// <para>hierarchical menus, that will obviously impose a different order on what the user sees in the</para>
        /// <para>menu.  Though even with  hierarchical menus, applications should try to preserve config ordering</para>
        /// <para>(which is equivalent to index ordering) for items within the same sub-menu.</para>
        /// </remarks>
        public ref class ColorSpaceMenuHelper : ICppInstance
        {
        public:

            [::System::Runtime::InteropServices::UnmanagedFunctionPointer(::System::Runtime::InteropServices::CallingConvention::Cdecl)] 
            delegate unsigned long long Func_ulong___IntPtr_string8(::System::IntPtr __instance, ::System::String^ arg1);

            [::System::Runtime::InteropServices::UnmanagedFunctionPointer(::System::Runtime::InteropServices::CallingConvention::Cdecl)] 
            delegate unsigned long long Func_ulong___IntPtr_ulong(::System::IntPtr __instance, unsigned long long arg1);

            [::System::Runtime::InteropServices::UnmanagedFunctionPointer(::System::Runtime::InteropServices::CallingConvention::Cdecl)] 
            delegate ::System::String^ Func___IntPtr___IntPtr_ulong_ulong(::System::IntPtr __instance, unsigned long long arg1, unsigned long long arg2);

            property class ::OpenColorIO_v2_4::ColorSpaceMenuHelper* NativePtr;
            property ::System::IntPtr __Instance
            {
                virtual ::System::IntPtr get();
                virtual void set(::System::IntPtr instance);
            }

            ColorSpaceMenuHelper(class ::OpenColorIO_v2_4::ColorSpaceMenuHelper* native);
            ColorSpaceMenuHelper(class ::OpenColorIO_v2_4::ColorSpaceMenuHelper* native, bool ownNativeInstance);
            static ColorSpaceMenuHelper^ __CreateInstance(::System::IntPtr native);
            static ColorSpaceMenuHelper^ __CreateInstance(::System::IntPtr native, bool __ownsNativeInstance);
            ~ColorSpaceMenuHelper();

            /// <summary>Access to the color spaces (or roles).</summary>
            property unsigned long long NumColorSpaces
            {
                unsigned long long get();
            }

            /// <summary>
            /// <para>Get the color space (or role) name used in the config for this menu item.  Will be empty</para>
            /// <para>if the index is out of range.</para>
            /// </summary>
            virtual ::System::String^ GetName(unsigned long long idx);

            /// <summary>
            /// <para>Get the name to use in the menu UI.  This might be different from the config name, for</para>
            /// <para>example in the case of roles.  Will be empty if the index is out of range.</para>
            /// </summary>
            virtual ::System::String^ GetUIName(unsigned long long idx);

            /// <summary>
            /// <para>Get the index of the element of a given name. Return (size_t)-1 name if NULL or empty, or if</para>
            /// <para>no element with that name is found.</para>
            /// </summary>
            virtual unsigned long long GetIndexFromName(::System::String^ name);

            virtual unsigned long long GetIndexFromUIName(::System::String^ name);

            virtual ::System::String^ GetDescription(unsigned long long idx);

            virtual ::System::String^ GetFamily(unsigned long long idx);

            /// <summary>
            /// <para>Hierarchy levels are created from the family string. It is split into levels using the</para>
            /// <para>'family separator'.</para>
            /// </summary>
            virtual unsigned long long GetNumHierarchyLevels(unsigned long long idx);

            virtual ::System::String^ GetHierarchyLevel(unsigned long long idx, unsigned long long i);

            /// <summary>Get the color space name from the UI name.</summary>
            virtual ::System::String^ GetNameFromUIName(::System::String^ uiName);

            /// <summary>Get the color space UI name from the name.</summary>
            virtual ::System::String^ GetUINameFromName(::System::String^ name);

        protected:

            bool __ownsNativeInstance;
        };

        /// <summary>
        /// <para>Whereas the DisplayViewTransform simply applies a specific view from an OCIO display, the</para>
        /// <para>LegacyViewingPipeline provides an example of a complete viewing pipeline of the sort that could</para>
        /// <para>be used to implement a viewport in a typical application.  It therefore adds, around the</para>
        /// <para>DisplayViewTransform, various optional color correction steps and RGBA channel view swizzling.</para>
        /// <para>The direction of the DisplayViewTranform is used as the direction of the pipeline.</para>
        /// <para>Note: The LegacyViewingPipeline class provides the same functionality as the OCIO v1</para>
        /// <para>DisplayTransform.</para>
        /// </summary>
        /// <remarks>
        /// <para>Legacy viewing pipeline:</para>
        /// <para>* Start in display transform input color space.</para>
        /// <para>* If linearCC is provided:</para>
        /// <para>* Go to scene_linear colorspace.</para>
        /// <para>* Apply linearCC transform.</para>
        /// <para>* If colorTimingCC is provided:</para>
        /// <para>* Go to color_timing colorspace.</para>
        /// <para>* Apply colorTimingCC transform.</para>
        /// <para>* Apply looks (from display transform or from looks override).</para>
        /// <para>* Go to first look color space.</para>
        /// <para>* Apply first look transform.</para>
        /// <para>* Iterate for all looks.</para>
        /// <para>* Apply channelView transform.</para>
        /// <para>* Apply display transform (without looks).</para>
        /// <para>* Apply displayCC.</para>
        /// <para>Note that looks are applied even if the display transform involves data color spaces.</para>
        /// </remarks>
        public ref class LegacyViewingPipeline : ICppInstance
        {
        public:

            property class ::OpenColorIO_v2_4::LegacyViewingPipeline* NativePtr;
            property ::System::IntPtr __Instance
            {
                virtual ::System::IntPtr get();
                virtual void set(::System::IntPtr instance);
            }

            LegacyViewingPipeline(class ::OpenColorIO_v2_4::LegacyViewingPipeline* native);
            LegacyViewingPipeline(class ::OpenColorIO_v2_4::LegacyViewingPipeline* native, bool ownNativeInstance);
            static LegacyViewingPipeline^ __CreateInstance(::System::IntPtr native);
            static LegacyViewingPipeline^ __CreateInstance(::System::IntPtr native, bool __ownsNativeInstance);
            ~LegacyViewingPipeline();

            property bool LooksOverrideEnabled
            {
                bool get();
                void set(bool);
            }

            property ::System::String^ LooksOverride
            {
                ::System::String^ get();
                void set(::System::String^);
            }

        protected:

            bool __ownsNativeInstance;
        };

        /// <summary>
        /// <para>The MixingSlider and MixingColorSpaceManager classes are to help applications implement correct</para>
        /// <para>color pickers.  The term &quot;color mixing&quot; is used here to describe what is done in a typical</para>
        /// <para>application &quot;color picker&quot; user interface.</para>
        /// </summary>
        /// <remarks>
        /// <para>A user may want to mix colors in different color spaces.  The two most common mixing space</para>
        /// <para>options are a scene-linear working space or the display space.</para>
        /// <para>Since scene-linear color spaces are not perceptually uniform, it is necessary to compensate UI</para>
        /// <para>widgets such as sliders.  For example, it is nice if mid-gray falls near the center of mixing</para>
        /// <para>controls rather than way over near the black end.  This may be done by using a mapping from</para>
        /// <para>linear into an approximately perceptually uniform space.</para>
        /// <para>Also note that a color picking/mixing UI may want to present a given color space in several</para>
        /// <para>different encodings.  The most common two encodings for color mixing are RGB and HSV.</para>
        /// <para>Note that these helpers anticipate that a user may want to mix colors using values that extend</para>
        /// <para>outside the typical [0,1] domain.</para>
        /// </remarks>
        public ref class MixingSlider : ICppInstance
        {
        public:

            [::System::Runtime::InteropServices::UnmanagedFunctionPointer(::System::Runtime::InteropServices::CallingConvention::Cdecl)] 
            delegate void Action___IntPtr_float(::System::IntPtr __instance, float arg1);

            [::System::Runtime::InteropServices::UnmanagedFunctionPointer(::System::Runtime::InteropServices::CallingConvention::Cdecl)] 
            delegate float Func_float___IntPtr(::System::IntPtr __instance);

            [::System::Runtime::InteropServices::UnmanagedFunctionPointer(::System::Runtime::InteropServices::CallingConvention::Cdecl)] 
            delegate float Func_float___IntPtr_float(::System::IntPtr __instance, float arg1);

            property class ::OpenColorIO_v2_4::MixingSlider* NativePtr;
            property ::System::IntPtr __Instance
            {
                virtual ::System::IntPtr get();
                virtual void set(::System::IntPtr instance);
            }

            MixingSlider(class ::OpenColorIO_v2_4::MixingSlider* native);
            MixingSlider(class ::OpenColorIO_v2_4::MixingSlider* native, bool ownNativeInstance);
            static MixingSlider^ __CreateInstance(::System::IntPtr native);
            static MixingSlider^ __CreateInstance(::System::IntPtr native, bool __ownsNativeInstance);
            ~MixingSlider();

            /// <summary>Minimum edge of a UI slider for conversion to mixing space.</summary>
            /// <remarks>Set the minimum edge of a UI slider for conversion to mixing space.</remarks>
            property float SliderMinEdge
            {
                float get();
                void set(float);
            }

            /// <summary>Maximum edge of a UI slider for conversion to mixing space.</summary>
            /// <remarks>Set the maximum edge of a UI slider for conversion to mixing space.</remarks>
            property float SliderMaxEdge
            {
                float get();
                void set(float);
            }

            /// <summary>Convert from units in distance along the slider to mixing space units.</summary>
            virtual float SliderToMixing(float sliderUnits);

            /// <summary>Convert from mixing space units to distance along the slider.</summary>
            virtual float MixingToSlider(float mixingUnits);

        protected:

            bool __ownsNativeInstance;
        };

        /// <summary>Used to mix (or pick/choose) colors.</summary>
        public ref class MixingColorSpaceManager : ICppInstance
        {
        public:

            [::System::Runtime::InteropServices::UnmanagedFunctionPointer(::System::Runtime::InteropServices::CallingConvention::Cdecl)] 
            delegate OpenColorIO::OpenColorIO_v2_4::MixingSlider^ Func___IntPtr___IntPtr_float_float(::System::IntPtr __instance, float arg1, float arg2);

            property class ::OpenColorIO_v2_4::MixingColorSpaceManager* NativePtr;
            property ::System::IntPtr __Instance
            {
                virtual ::System::IntPtr get();
                virtual void set(::System::IntPtr instance);
            }

            MixingColorSpaceManager(class ::OpenColorIO_v2_4::MixingColorSpaceManager* native);
            MixingColorSpaceManager(class ::OpenColorIO_v2_4::MixingColorSpaceManager* native, bool ownNativeInstance);
            static MixingColorSpaceManager^ __CreateInstance(::System::IntPtr native);
            static MixingColorSpaceManager^ __CreateInstance(::System::IntPtr native, bool __ownsNativeInstance);
            ~MixingColorSpaceManager();

            /// <summary>Access to the mixing spaces.</summary>
            property unsigned long long NumMixingSpaces
            {
                unsigned long long get();
            }

            property unsigned long long SelectedMixingSpaceIdx
            {
                unsigned long long get();
                void set(unsigned long long);
            }

            property bool IsPerceptuallyUniform
            {
                bool get();
            }

            /// <summary>Access to the mixing encodings.</summary>
            property unsigned long long NumMixingEncodings
            {
                unsigned long long get();
            }

            property unsigned long long SelectedMixingEncodingIdx
            {
                unsigned long long get();
                void set(unsigned long long);
            }

            virtual ::System::String^ GetMixingSpaceUIName(unsigned long long idx);

            virtual void SetSelectedMixingSpace(::System::String^ mixingSpace);

            virtual ::System::String^ GetMixingEncodingName(unsigned long long idx);

            virtual void SetSelectedMixingEncoding(::System::String^ mixingEncoding);

            virtual OpenColorIO::OpenColorIO_v2_4::MixingSlider^ GetSlider();

            virtual OpenColorIO::OpenColorIO_v2_4::MixingSlider^ GetSlider(float sliderMixingMinEdge, float sliderMixingMaxEdge);

        protected:

            bool __ownsNativeInstance;
        };

        namespace ColorSpaceHelpers
        {
        }

        namespace DisplayViewHelpers
        {
        }
    }
}
