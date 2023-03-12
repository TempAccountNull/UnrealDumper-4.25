#include <Windows.h>
#include <fmt/core.h>
#include "dumper.h"
#include "engine.h"
#include "memory.h"
#include "utils.h"
#include "wrappers.h"

Dumper::~Dumper()
{
  if (Image) VirtualFree(Image, 0, MEM_RELEASE);
}

STATUS Dumper::Init(int argc, char* argv[])
{
    uint32_t pid = 0;

    fs::path processName;
    wchar_t processPath[MAX_PATH]{};

    for (int i = 1; i < argc; i++)
    {
        char* arg = argv[i];
        uint16 arg16 = *reinterpret_cast<uint16*>(arg);
        if (arg16 == 'h-')
        {
            printf("'-p' - dump only names and objects\n'-w' - wait for input (it gives me time to inject mods)\n'-f packageNameHere' - specifies package where we should look for pointers in paddings (can take a lot of time)");
            return STATUS::FAILED;
        }
        else if (arg16 == 'p-')
        {
            Full = false;
        }
        else if (arg16 == 'w-')
        {
            Wait = true;
        }
        else if (arg16 == 'f-')
        {
            i++;
            if (i < argc) { PackageName = argv[i]; }
            else { return STATUS::FAILED; }
        }
        else if (!strcmp(arg, "--spacing"))
        {
            Spacing = true;
        }
    }

    if (Wait)
    {
        system("pause");
    }

    //See if the game in question is an Unreal game by it's class window.
    HWND hWnd = FindWindowA("UnrealWindow", nullptr);
    if (!hWnd)
    {
        printf("Cannot read! Unreal window does not exist!\n");
        return STATUS::WINDOW_NOT_FOUND;
    }

    //Grab process ID
    GetWindowThreadProcessId(hWnd, reinterpret_cast<DWORD*>(&pid));
    if (!pid)
    {
        printf("Cannot read! PID requires a change in rebuild!\n");
        return STATUS::PROCESS_NOT_FOUND;
    }

    //Attempt to read Process ID
    if (!ReaderInit(pid))
    {
        printf("[ReaderInit]: Cannot read! Reader Error\n");
        return STATUS::READER_ERROR;
    }

    //Attempt to Grab the process Path
    if (!GetProccessPath(pid, processPath, MAX_PATH))
    {
        printf("[PROC ERR]: Process name was not found! FAILING . . .\n");
	    return STATUS::CANNOT_GET_PROCNAME;
    }

    //If we have a process name
    processName = fs::path(processPath).filename();
    printf("Found UE4 game: %ls\n", processName.c_str());

    fs::path root = fs::path(argv[0]);
    root.remove_filename();

    //Create the folder Games/GameName directory
    fs::path game = processName.stem();
    Directory = root / "Games" / game;
    fs::create_directories(Directory);

    //Check the Process image size.
    uint64 size = GetImageSize();
    if (!size)
    {
        printf("[MEM ERR]: The Module was not found! FAILING . . .\n");
        return STATUS::MODULE_NOT_FOUND;
    }

    //Read the base memory
    Image = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!Read(reinterpret_cast<void*>(Base), Image, size))
    {
        printf("[MEM ERR]: Cannot read the base memory!\n");
        return STATUS::CANNOT_READ;
    }

    return EngineInit(game.string(), Image);
}

STATUS Dumper::Dump() {
    /*
     * Names dumping.
     * We go through each block, except last, that is not fully filled.
     * In each block we calculate next entry depending on previous entry size.
     */
    File file(Directory / "NamesDump.txt", "w");
    if (!file)
    {
        return STATUS::FILE_NOT_OPEN;
    }

    size_t size = NULL;
    NamePoolData.Dump([&file, &size](std::string_view name, uint32_t id) { 
            fmt::print(file, "[{:0>6}] {}\n", id, name.data()); 
            size++; 
        }); 
    fmt::print("Names: {}\n", size);

    // Why we need to iterate all objects twice? We dumping objects and filling packages simultaneously.
    std::unordered_map<uint8*, std::vector<UE_UObject>> packages;
    {
        File file(Directory / "ObjectsDump.txt", "w");
        if (!file) { return STATUS::FILE_NOT_OPEN; }
        size_t size = NULL;


        std::function<void(UE_UObject)> callback;
        if (Full)
        {
            callback = [&file, &size, &packages](UE_UObject object) {

                bool isFunction = object.IsA<UE_UFunction>();
                if (isFunction)
                {
                    fmt::print(file, "[{:0>6}] <{}> <{}> {} {:x}\n", object.GetIndex(), object.GetAddress(), Read<void*>(object.GetAddress()), object.GetFullName(), object.Cast<UE_UFunction>().GetFunc() - Base);
                }
                else
                {
                    fmt::print(file, "[{:0>6}] <{}> <{}> {}\n", object.GetIndex(), object.GetAddress(), Read<void*>(object.GetAddress()), object.GetFullName());
                }

                size++;
                if (isFunction || object.IsA<UE_UStruct>() || object.IsA<UE_UEnum>())
                {
                    UE_UObject packageObj = object.GetPackageObject();
                    packages[packageObj].push_back(object);
                }
            };
        }
        else
        {
            callback = [&file, &size](UE_UObject object)
            {
                fmt::print(file, "[{:0>6}] <{}> <{}> {}\n", object.GetIndex(), object.GetAddress(), Read<void*>(object.GetAddress()), object.GetFullName());
                size++;
            };
        }

        ObjObjects.Dump(callback);

        fmt::print("Objects: {}\n", size);
    }

    if (!Full)
    {
        printf("[INFO]: Packages are not Full!\n");
        return STATUS::SUCCESS;
    }

    //{
    //    // Clearing all packages with small amount of objects (comment this if
    //    you need all packages to be dumped) size_t size = packages.size();
    //    size_t erased = std::erase_if(packages, [](std::pair<byte* const,
    //    std::vector<UE_UObject>>& package) { return package.second.size() < 2;
    //    });

    //    fmt::print("Wiped {} out of {}\n", erased, size);
    //}

    // Checking if we have any package after clearing.
    if (!packages.size())
    {
        return STATUS::ZERO_PACKAGES;
    }

    fmt::print("Packages: {}\n", packages.size());


    std::filesystem::path path = Directory / "DUMP";
    fs::create_directories(path);

    int i = 1, saved = NULL;
    std::string unsaved{};

    bool lock = true;
    if (PackageName) lock = false;

    for (UE_UPackage package : packages)
    {
        fmt::print("\rProcessing: {}/{}", i++, packages.size());

        if (!lock && package.GetObject().GetName() == PackageName)
        {
            package.FindPointers = true;
            lock = true;
        }

        package.Process();
        if (package.Save(path, Spacing))
        {
            saved++;
        }
        else
        {
            unsaved += (package.GetObject().GetName() + ", ");
        }
    }

    fmt::print("\nSaved packages: {}\n", saved);

    if (unsaved.size())
    {
        unsaved.erase(unsaved.size() - 2);
        fmt::print("Unsaved empty packages: [ {} ]\n", unsaved);
    }
    return STATUS::SUCCESS;
}
