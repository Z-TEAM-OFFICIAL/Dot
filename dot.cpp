#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

// ZEGA Corporate Branding
#define ZEGA_GREEN "\033[38;2;88;240;27m"
#define ZEGA_RED   "\033[91m"
#define RESET      "\033[0m"

void ShowVersion() {
    std::cout << ZEGA_GREEN << "ZEGA Dot Engine [Version 1.0.42-Production]" << RESET << std::endl;
    std::cout << "Owner: ZEGA Corporation (Founder Edition)" << std::endl;
}

void InitProject(std::string name) {
    std::string root = "C:\\Program Files\\Dot";
    std::string api_dll = root + "\\api\\dot.dll";
    std::string target = fs::current_path().string() + "\\" + name;

    if (fs::exists(target)) {
        std::cerr << ZEGA_RED << "FATAL: <Project directory already exists: " << name << ">" << RESET << std::endl;
        exit(1);
    }

    fs::create_directories(target + "\\src");
    fs::create_directories(target + "\\build");

    // Copy Local Engine (dot.dll) from the Trusted Installer location
    if (!CopyFileA(api_dll.c_str(), (target + "\\dot.dll").c_str(), FALSE)) {
        std::cerr << ZEGA_RED << "FATAL: <System API missing. Please run setupdot.exe as Admin.>" << RESET << std::endl;
        exit(1);
    }

    std::ofstream manifest(target + "\\zega.json");
    manifest << "{\n  \"project\": \"" << name << "\",\n  \"version\": \"1.0.0\",\n  \"engine\": \"dot.dll\"\n}";
    manifest.close();

    std::ofstream src(target + "\\src\\main.dot");
    src << "import system.\nset x 10.\nprint \"ZEGA System Online\".\nloop 5.\n  print x.\nstop.\n.";
    src.close();

    std::cout << ZEGA_GREEN << "PROJECT CREATED: " << name << RESET << std::endl;
    std::cout << "Location: " << target << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "ZEGA Dot CLI\nUsage: dot [file.dot] [flags] OR dot --newproj [name]" << std::endl;
        return 0;
    }

    std::string cmd = argv[1];

    // 1. Version Check (Added to fix the blank output issue)
    if (cmd == "--version" || cmd == "-v") {
        ShowVersion();
        return 0;
    }

    // 2. Project Scaffolding
    if (cmd == "--newproj" && argc == 3) {
        InitProject(argv[2]);
        return 0;
    }

    // 3. Engine Execution Logic
    // Try to load the System Core first, then fallback to the local Project DLL
    HINSTANCE hLib = LoadLibraryA("C:\\Program Files\\Dot\\bin\\dlls\\zega_core.dll");
    if (!hLib) {
        hLib = LoadLibraryA("dot.dll"); 
    }

    if (hLib) {
        typedef int (*ZegaExec)(const char*, const char*);
        auto Run = (ZegaExec)GetProcAddress(hLib, "ExecuteZega");
        
        if (!Run) {
            std::cerr << ZEGA_RED << "FATAL: <Entry point 'ExecuteZega' not found in DLL.>" << RESET << std::endl;
            FreeLibrary(hLib);
            return 1;
        }

        const char* opt = (argc > 2) ? argv[2] : "-o1";
        int result = Run(argv[1], opt);
        
        FreeLibrary(hLib);
        return result;
    }
    
    std::cerr << ZEGA_RED << "FATAL: <ZEGA Engine (dot.dll/zega_core.dll) not found.>" << RESET << std::endl;
    return 1;
}