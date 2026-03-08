workspace "smg"
	architecture "x86_64"
	startproject "smg"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

project "smg"
    kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
    multiprocessorcompile "on"

    targetdir ("%{wks.location}/build/%{cfg.buildcfg}")
	objdir ("%{wks.location}/build-int/%{cfg.buildcfg}/%{prj.name}")

	pchheader "main.h"
	pchsource "src/make_pch.cpp"

    includedirs {
        "external/include",
        "%{wks.location}/tracy",
    }

    files {
        "src/**.h",
        "src/**.cpp",
        "%{wks.location}/tracy/TracyClient.cpp"
    }

    defines {
		"_CRT_SECURE_NO_WARNINGS",
	}

    filter "files:tracy/TracyClient.cpp"
	    enablepch "off"

    filter "system:windows"
		systemversion "latest"

        defines { "RENDER_D3D11" }

        buildoptions { "/Zc:strictStrings-" }

        links {
            "winmm.lib",
            "d3d11.lib",
            "dxgi.lib",
            "freetype.lib",
        }

    filter "configurations:Debug"
		defines { "BUILD_DEBUG", "TRACY_ENABLE" }
		runtime "Debug"
		symbols "on"

        libdirs { "external/lib/Debug" }
        
	filter "configurations:Release"
		defines { "BUILD_RELEASE", "TRACY_ENABLE" }
		runtime "Release"
		optimize "on"

        libdirs { "external/lib/Release" }

	filter "configurations:Dist"
		defines "BUILD_DIST"
		runtime "Release"
		optimize "on"

        libdirs { "external/lib/Release" }
