newoption
{
    trigger = "sdl_backend",
    value = "BACKEND",
    description = "SDL backend to use",
    allowed = {
        { "auto", "Auto-detect best backend" },
        { "opengl", "OpenGL backend" },
        { "vulkan", "Vulkan backend" },
        { "d3d11", "Direct3D 11 backend" },
        { "d3d12", "Direct3D 12 backend" }
    },
    default = "auto"
}

function download_progress(total, current)
    local ratio = current / total
    ratio = math.min(math.max(ratio, 0), 1)
    local percent = math.floor(ratio * 100)
    print("Download progress (" .. percent .. "%/100%)")
end

function check_sdl3()
    os.chdir("external")
    if(os.isdir("SDL3") == false) then
        if(not os.isfile("SDL3-devel-VC.zip")) then
            print("SDL3 not found, downloading from GitHub")
            local result_str, response_code = http.download(
                "https://github.com/libsdl-org/SDL/releases/download/release-3.2.22/SDL3-devel-3.2.22-VC.zip", 
                "SDL3-devel-VC.zip", 
                {
                    progress = download_progress,
                    headers = { "From: Premake", "Referer: Premake" }
                }
            )
        end
        print("Unzipping to " .. os.getcwd())
        zip.extract("SDL3-devel-VC.zip", os.getcwd())
        -- Rename the extracted folder to SDL3 for consistency
        if os.isdir("SDL3-3.2.22") then
            os.rename("SDL3-3.2.22", "SDL3")
        end
        os.remove("SDL3-devel-VC.zip")
    end
    os.chdir("../")
end

function build_externals()
    print("calling externals")
    check_sdl3()
end

function platform_defines()
    filter {"configurations:Debug"}
        defines{"DEBUG", "_DEBUG"}
        symbols "On"

    filter {"configurations:Release"}
        defines{"NDEBUG", "RELEASE"}
        optimize "On"

    filter {"system:windows"}
        defines {"_WIN32", "_WINDOWS"}
        systemversion "latest"

    filter {"system:linux"}
        defines {"_GNU_SOURCE"}
        
    filter{}
end

-- Configuration
downloadSDL3 = true
sdl3_dir = "external/SDL3"

workspaceName = 'HOL'
baseName = path.getbasename(path.getdirectory(os.getcwd()))

if (baseName ~= 'HOL') then
    workspaceName = baseName
end

if (os.isdir('build_files') == false) then
    os.mkdir('build_files')
end

if (os.isdir('external') == false) then
    os.mkdir('external')
end

workspace (workspaceName)
    location "../"
    configurations { "Debug", "Release" }
    platforms { "x64", "x86" }

    defaultplatform ("x64")

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter { "platforms:x64" }
        architecture "x86_64"

    filter { "platforms:x86" }
        architecture "x86"

    filter {}

    targetdir "bin/%{cfg.buildcfg}/"

if (downloadSDL3) then
    build_externals()
end

    startproject(workspaceName)

    project (workspaceName)
        kind "ConsoleApp"
        location "build_files/"
        targetdir "../bin/%{cfg.buildcfg}"

        filter "action:vs*"
            debugdir "$(SolutionDir)"

        filter{}

        vpaths 
        {
            ["Header Files/*"] = { "../include/**.h", "../src/**.h"},
            ["Source Files/*"] = {"../src/**.c"},
        }
        
        files {"../src/**.c", "../src/**.h", "../include/**.h"}

        includedirs { "../src" }
        includedirs { "../include" }
        includedirs { sdl3_dir .. "/include" }

        cdialect "C17"
        platform_defines()

        filter "action:vs*"
            defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
            links {"SDL3.lib"}
            characterset ("Unicode")

        filter "system:windows"
            defines{"_WIN32"}
            links {"winmm", "gdi32", "opengl32"}
            
            -- SDL3 x64 específic
            filter { "system:windows", "platforms:x64" }
                libdirs { sdl3_dir .. "/lib/x64" }
                postbuildcommands {
                    -- Path correcte: des de build/build_files/ cap a build/external/SDL3/lib/x64/
                    "{COPY} \"$(SolutionDir)build\\external\\SDL3\\lib\\x64\\SDL3.dll\" \"$(SolutionDir)bin\\%{cfg.buildcfg}\\\""
                }
                
            -- SDL3 x86 específic
            filter { "system:windows", "platforms:x86" }
                libdirs { sdl3_dir .. "/lib/x86" }
                postbuildcommands {
                    -- Path correcte: des de build/build_files/ cap a build/external/SDL3/lib/x86/
                    "{COPY} \"$(SolutionDir)build\\external\\SDL3\\lib\\x86\\SDL3.dll\" \"$(SolutionDir)bin\\%{cfg.buildcfg}\\\""
                }

        filter "system:linux"
            links {"pthread", "m", "dl", "rt", "X11"}

        filter{}