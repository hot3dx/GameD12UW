Visual Studio 2019 - 16.3.9, Latest Upgrades, OS Build 1903, Tools 142
Upgraded by Hot3dx - Jeff Kubitz for x64
The build will run in Debug x64 Native Only
The DirectXTK12 lib is not from the XDK so I have included it
Make Certain the build order is DirectXTK12 first!!!!
This is the DeviceResources Universal Windows Platform version

There are two  projects!!!!!
1) GameD12UW is dependent on:
2) DirectXTK12 Solution and project files in the DXTK12 folder!!!


In order to build and run the projects:
1) Set the build dependencies so that GameD12Uw is dependent on 
2) the DirectXTK12

The #pragma comment(lib, L"C:..\..\.. ... \GameD12UW\DXTK12\..\ must be correctly set before the prject will run
When you build, the path will show before DirectXTK12.lib!!!!!
make certain that path is in the #pramga comment(lib, "..\..\..\..\..\ here\DirectXTK12.lib);

// The path must be correct in order for it all to work
// Add a reference by right clicking the project. The DXTK12 should showup, check the box 
// Hit Okay, else: click the browse button find where you put the DirectXTK12 project
// Check the box. The paths in pch.h will need to be changed to match the DirectXTK12 location

The build order should be set to DirectxTK12

There may be problems with the Segoeui.ttf, segoui.spritefont, ADPCMdroid.xwb, tinymeshmesh.sdkmesh files not automatically going to the correct directories! they shoould work where they are at but...put them in the right directories and voila!!!

Have Fun!!!

Escape Button Should also shut'er off!

A template can be easily made from this code of two projects

1) For Visual Studio 2019 delete  the file GameD12UW.vstemplate
2) Click the DirectXTK12 project
    a) Right click Projects
    b) Choose Export Template
    c) Use the __Template.ico and GamD12UW.png files in the Assets folder for Icon and Preview Image
    d) Hit Okay
    e) The DirectXTK12.zip will show up in Visual Studio 2019/My Exported Templaates/ DirectXTK12.zip
   
3) Click the GameD12UW project
    a) Right click Projects
    b) Choose Export Template
    c) Use the __Template.ico and GamD12UW.png files in the Assets folder for Icon and Preview Image
    d) Hit Okay
    e) The GameD12UW.zip will show up in Visual Studio 2019/My Exported Templaates/GameD12UW.zip
    
4) Both projects will show as templates when creating a new project ad the DirectXTK12 first!



# Direct3D Game VS project templates
This repo contains simplified Visual C++ project templates. They are primarily intended for developer education, samples, and tutorials.

See [this blog post](https://walbourn.github.io/direct3d-win32-game-visual-studio-template/) and [this one](https://walbourn.github.io/direct3d-game-visual-studio-templates-redux/).

Documentation is available on [GitHub](https://github.com/walbourn/directx-vs-templates/wiki).

There are two versions of each template. The DR version adds a ``DeviceResources`` ([DX11](https://github.com/Microsoft/DirectXTK/wiki/DeviceResources) or [DX12](https://github.com/Microsoft/DirectXTK12/wiki/DeviceResources)) abstraction to isolate the device and swapchain code into it's own helper class.

For the UWP templates, there are C++/WinRT variants that use [C++ Windows Runtime language projections](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/) rather than the C++/CX language extensions (``/ZW``).

# VS 2015
``VSIX\Direct3DUWPGame.vsix`` installs the Direct3D UWP Game templates and the Direct3D Win32 Game templates for VS 2015

The package requires the [Visual C++](https://devblogs.microsoft.com/cppblog/setup-changes-in-visual-studio-2015-affecting-c-developers/) and [Windows Tools](https://devblogs.microsoft.com/cppblog/developing-for-windows-10-with-visual-c-2015/) optional features are installed, and requires Windows 10 Anniversary Update SDK (14393) or later.

> For C++/WinRT projects, use cppwinrt NuGet package [2017.2.28](https://www.nuget.org/packages/cppwinrt/2017.2.28.4) with the Windows 10 Anniversary Update SDK (14393).

# VS 2017
The ``VSIX\Direct3DUWPGame.vsix`` also works for VS 2017. Use of the Visual Studio 2017 15.9 update is recommended.

The package requires the *Universal Windows Platform development* workload (``Microsoft.VisualStudio.Workload.Universal``) with the *C++ Universal Windows Platform tools* (``Microsoft.VisualStudio.ComponentGroup.UWP.VC``). The Win32 templates require the *Desktop development with C++* workload (``Microsoft.VisualStudio.Workload.NativeDesktop``). It is recommended you make use of the Windows 10 October 2018 Update SDK (17763).

# VS 2019
The ``VSIX\Direct3DUWPGame.vsix`` also works for VS 2019.

The package requires the *Universal Windows Platform development* workload (``Microsoft.VisualStudio.Workload.Universal``) with the *C++ Universal Windows Platform tools* (``Microsoft.VisualStudio.ComponentGroup.UWP.VC``). The Win32 templates require the *Desktop development with C++* workload (``Microsoft.VisualStudio.Workload.NativeDesktop``). It is recommended you make use of the Windows 10 October 2018 Update SDK (17763) or later.

# Rebuilding the VSIX
Building the VSIX project requires VS 2017 with the *Visual Studio extension development* (``Microsoft.VisualStudio.Workload.VisualStudioExtension``) and *.NET desktop development* (``Microsoft.VisualStudio.Workload.ManagedDesktop``) workloads.

# Notices
All content and source code for this package are subject to the terms of the [MIT License](http://opensource.org/licenses/MIT).

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

Donations accepted:
BTC 32HVJHEXh9bMLVkmoSE1d389q2Q1YvfrNE
