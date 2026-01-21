// CMDHelper - Windows Command Prompt Enhancement Utility
// 
// Copyright (c) 2025 AuthorityGate, Inc.
// Licensed under the MIT License. See LICENSE file for details.
// 
// Version: 1.0.0
// Author: Kevin E. Komlosy
// Company: AuthorityGate Inc.
// Repository: https://github.com/AuthorityGate/CMDHelper
// 
// Description:
//   This application enhances the Windows command prompt experience by adding
//   Explorer context menu integration, admin/user/system mode switching with 
//   color-coded terminals, and self-installation capabilities.
// 
// Features:
//   - Right-click context menu: "AuthorityGate CMD (Admin)", "AuthorityGate CMD (User)", 
//     and "AuthorityGate CMD (System)"
//   - Start Menu shortcuts under "AuthorityGate Utilities"
//   - Customizable color schemes via --set-colors
//   - Self-installing on first run
// 
// Modes:
//   - Admin:  Red text on black background (elevated prompt)
//   - User:   Light green text on black background (standard prompt)
//   - System: Yellow text on black background (elevated prompt for system tasks)
// 
// Usage:
//   CMDHelper.exe [options] [directory]
//   Options: --help, --admin, --user, --system, --reinstall, --uninstall, --set-colors
// 
// THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
// See LICENSE file for full terms.

#include <iostream>
#include <windows.h>
#include <string>
#include <shlobj.h> // For SHCreateDirectoryEx
#include <comdef.h> // For _com_error

void StartCmd(const std::string& command, bool asAdmin)
{
    SHELLEXECUTEINFOA sei = { sizeof(sei) };
    sei.lpVerb = asAdmin ? "runas" : "open";
    sei.lpFile = "cmd.exe";
    sei.lpParameters = command.c_str();
    sei.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExA(&sei))
    {
        DWORD error = GetLastError();
        std::cerr << "Failed to start CMD" << (asAdmin ? " as Admin" : " as User") << ". Error: " << error << std::endl;
    }
}

bool IsFirstRun()
{
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\AuthorityGate\\CMDHelper", 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }
    return true;
}

void AddToPath(const std::string& path)
{
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", 0, KEY_READ | KEY_WRITE, &hKey);
    if (result == ERROR_SUCCESS)
    {
        char value[1024];
        DWORD value_length = sizeof(value);
        result = RegQueryValueExA(hKey, "Path", NULL, NULL, (LPBYTE)value, &value_length);
        if (result == ERROR_SUCCESS)
        {
            std::string pathValue = value;
            if (pathValue.find(path) == std::string::npos)
            {
                pathValue += ";" + path;
                RegSetValueExA(hKey, "Path", 0, REG_SZ, (BYTE*)pathValue.c_str(), pathValue.size() + 1);
            }
        }
        RegCloseKey(hKey);
    }
}

void CreateShortcut(const std::string& targetPath, const std::string& arguments, const std::string& shortcutPath, const std::string& description)
{
    HRESULT hres = CoInitialize(NULL);
    if (FAILED(hres))
    {
        std::cerr << "Failed to initialize COM library. Error: " << _com_error(hres).ErrorMessage() << std::endl;
        return;
    }

    IShellLink* pShellLink = NULL;
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pShellLink);
    if (SUCCEEDED(hres))
    {
        // Convert narrow strings to wide strings
        std::wstring wTargetPath(targetPath.begin(), targetPath.end());
        std::wstring wArguments(arguments.begin(), arguments.end());
        std::wstring wDescription(description.begin(), description.end());

        pShellLink->SetPath(wTargetPath.c_str());
        pShellLink->SetArguments(wArguments.c_str());
        pShellLink->SetDescription(wDescription.c_str());

        IPersistFile* pPersistFile = NULL;
        hres = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
        if (SUCCEEDED(hres))
        {
            WCHAR wsz[MAX_PATH];
            MultiByteToWideChar(CP_ACP, 0, shortcutPath.c_str(), -1, wsz, MAX_PATH);
            hres = pPersistFile->Save(wsz, TRUE);
            if (FAILED(hres))
            {
                std::cerr << "Failed to save shortcut. Error: " << _com_error(hres).ErrorMessage() << std::endl;
            }
            pPersistFile->Release();
        }
        else
        {
            std::cerr << "Failed to get IPersistFile interface. Error: " << _com_error(hres).ErrorMessage() << std::endl;
        }
        pShellLink->Release();
    }
    else
    {
        std::cerr << "Failed to create IShellLink instance. Error: " << _com_error(hres).ErrorMessage() << std::endl;
    }
    CoUninitialize();
}

void CreateStartMenuShortcuts()
{
    char szPath[MAX_PATH];
    GetModuleFileNameA(NULL, szPath, MAX_PATH);
    std::string exePath = szPath;
    char startMenuPath[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_PROGRAMS, NULL, 0, startMenuPath);
    std::string folderPath = std::string(startMenuPath) + "\\AuthorityGate Utilities";
    SHCreateDirectoryExA(NULL, folderPath.c_str(), NULL);

    CreateShortcut(exePath, "--admin", folderPath + "\\CMDHelper (Admin).lnk", "Open CMDHelper as Admin - Red");
    CreateShortcut(exePath, "--user", folderPath + "\\CMDHelper (User).lnk", "Open CMDHelper as User - Green");
    CreateShortcut(exePath, "--system", folderPath + "\\CMDHelper (System).lnk", "Open CMDHelper as System - Yellow");
    CreateShortcut(exePath, "--reinstall", folderPath + "\\CMDHelper (Reinstall).lnk", "Reinstall CMDHelper");
    CreateShortcut(exePath, "--uninstall", folderPath + "\\CMDHelper (Uninstall).lnk", "Uninstall CMDHelper");
}

void InstallExecutable()
{
    char szPath[MAX_PATH];
    GetModuleFileNameA(NULL, szPath, MAX_PATH);
    std::string sourcePath = szPath;
    std::string destDir = "C:\\Program Files\\AuthorityGate\\CMDHelper";
    std::string destPath = destDir + "\\CmdHelper.exe";
    std::string iconSourcePath = "Authority_Gate_CMD.ico";
    std::string iconDestPath = destDir + "\\Authority_Gate_CMD.ico";

    // Create the destination directory if it doesn't exist
    SHCreateDirectoryExA(NULL, destDir.c_str(), NULL);

    // Copy the executable to the destination
    if (!CopyFileA(sourcePath.c_str(), destPath.c_str(), FALSE))
    {
        DWORD error = GetLastError();
        std::cerr << "Failed to copy executable to " << destPath << ". Error: " << error << std::endl;
    }

    if (!CopyFileA(iconSourcePath.c_str(), iconDestPath.c_str(), FALSE))
    {
        DWORD error = GetLastError();
        std::cerr << "Failed to copy icon file to " << iconDestPath << ". Error: " << error << std::endl;
    }

    AddToPath(destDir);
    CreateStartMenuShortcuts();
}

void InstallRegistryEntry()
{
    HKEY hKey;
    
    // Add context menu entry for directories (Admin)
    RegCreateKeyExA(HKEY_CLASSES_ROOT, "Directory\\shell\\OpenCmdHereAsAdmin", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"AuthorityGate CMD (Admin)", strlen("AuthorityGate CMD (Admin)") + 1);
    RegSetValueExA(hKey, "Icon", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico") + 1);
    RegCreateKeyExA(hKey, "command", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%1\" --admin", strlen("\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%1\" --admin") + 1);
    RegCloseKey(hKey);

    // Add context menu entry for directories (User)
    RegCreateKeyExA(HKEY_CLASSES_ROOT, "Directory\\shell\\OpenCmdHereAsUser", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"AuthorityGate CMD (User)", strlen("AuthorityGate CMD (User)") + 1);
    RegSetValueExA(hKey, "Icon", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico") + 1);
    RegCreateKeyExA(hKey, "command", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%1\" --user", strlen("\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%1\" --user") + 1);
    RegCloseKey(hKey);

    // Add context menu entry for directories (System)
    RegCreateKeyExA(HKEY_CLASSES_ROOT, "Directory\\shell\\OpenCmdHereAsSystem", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"AuthorityGate CMD (System)", strlen("AuthorityGate CMD (System)") + 1);
    RegSetValueExA(hKey, "Icon", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico") + 1);
    RegCreateKeyExA(hKey, "command", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%1\" --system", strlen("\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%1\" --system") + 1);
    RegCloseKey(hKey);

    // Add context menu entry for background of directories (Admin)
    RegCreateKeyExA(HKEY_CLASSES_ROOT, "Directory\\Background\\shell\\OpenCmdHereAsAdmin", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"AuthorityGate CMD (Admin)", strlen("AuthorityGate CMD (Admin)") + 1);
    RegSetValueExA(hKey, "Icon", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico") + 1);
    RegCreateKeyExA(hKey, "command", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%V\" --admin", strlen("\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%V\" --admin") + 1);
    RegCloseKey(hKey);

    // Add context menu entry for background of directories (User)
    RegCreateKeyExA(HKEY_CLASSES_ROOT, "Directory\\Background\\shell\\OpenCmdHereAsUser", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"AuthorityGate CMD (User)", strlen("AuthorityGate CMD (User)") + 1);
    RegSetValueExA(hKey, "Icon", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico") + 1);
    RegCreateKeyExA(hKey, "command", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%V\" --user", strlen("\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%V\" --user") + 1);
    RegCloseKey(hKey);

    // Add context menu entry for background of directories (System)
    RegCreateKeyExA(HKEY_CLASSES_ROOT, "Directory\\Background\\shell\\OpenCmdHereAsSystem", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"AuthorityGate CMD (System)", strlen("AuthorityGate CMD (System)") + 1);
    RegSetValueExA(hKey, "Icon", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico") + 1);
    RegCreateKeyExA(hKey, "command", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%V\" --system", strlen("\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%V\" --system") + 1);
    RegCloseKey(hKey);

    // Create system variables for color and default file location
    RegCreateKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\AuthorityGate\\CMDHelper", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, "AdminTextColor", 0, REG_SZ, (BYTE*)"4", 2);      // Red
    RegSetValueExA(hKey, "UserTextColor", 0, REG_SZ, (BYTE*)"A", 2);       // Light Green
    RegSetValueExA(hKey, "SystemTextColor", 0, REG_SZ, (BYTE*)"E", 2);     // Yellow
    RegSetValueExA(hKey, "AdminBackgroundColor", 0, REG_SZ, (BYTE*)"0", 2);  // Black
    RegSetValueExA(hKey, "UserBackgroundColor", 0, REG_SZ, (BYTE*)"0", 2);   // Black
    RegSetValueExA(hKey, "SystemBackgroundColor", 0, REG_SZ, (BYTE*)"0", 2); // Black
    RegSetValueExA(hKey, "DefaultLocation", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper") + 1);
    RegSetValueExA(hKey, "IconPath", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico") + 1);
    RegCloseKey(hKey);

    std::cout << "Registry entries installed successfully." << std::endl;
}

void UninstallRegistryEntry()
{
    // Remove context menu entries for directories
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "Directory\\shell\\OpenCmdHereAsAdmin");
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "Directory\\shell\\OpenCmdHereAsUser");
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "Directory\\shell\\OpenCmdHereAsSystem");
    // Remove context menu entries for background of directories
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "Directory\\Background\\shell\\OpenCmdHereAsAdmin");
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "Directory\\Background\\shell\\OpenCmdHereAsUser");
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "Directory\\Background\\shell\\OpenCmdHereAsSystem");
    // Remove system variables for color and default location
    RegDeleteTreeA(HKEY_LOCAL_MACHINE, "SOFTWARE\\AuthorityGate\\CMDHelper");

    std::cout << "Registry entries uninstalled successfully." << std::endl;
}

std::string GetRegistryValue(const std::string& key, const std::string& valueName)
{
    HKEY hKey;
    char value[256];
    DWORD value_length = sizeof(value);
    RegOpenKeyExA(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &hKey);
    RegQueryValueExA(hKey, valueName.c_str(), NULL, NULL, (LPBYTE)value, &value_length);
    RegCloseKey(hKey);
    return std::string(value, value_length - 1);
}

void ShowHelp()
{
    std::cout << "\nCMDHelper - An AuthorityGate Utility\n";
    std::cout << "Version 1.0.0\n\n";
    std::cout << "Usage: CmdHelper [options] [directory]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help           Show this help message\n";
    std::cout << "  --uninstall      Uninstall registry entries\n";
    std::cout << "  --reinstall      Reinstall registry entries\n";
    std::cout << "  --admin          Open CMD as Admin (Red text)\n";
    std::cout << "  --user           Open CMD as User (Green text)\n";
    std::cout << "  --system         Open CMD as System (Yellow text)\n";
    std::cout << "  --set-colors     Set color codes for Admin, User, and System\n";
    std::cout << "                   Usage: --set-colors <AdminText> <UserText> <SystemText> <AdminBg> <UserBg> <SystemBg>\n";
    std::cout << "\nModes:\n";
    std::cout << "  Admin:   Elevated prompt with RED text on black background\n";
    std::cout << "  User:    Standard prompt with GREEN text on black background\n";
    std::cout << "  System:  Elevated prompt with YELLOW text on black background\n";
    std::cout << "\nColor Codes:\n";
    std::cout << "  0 = Black       8 = Gray\n";
    std::cout << "  1 = Blue        9 = Light Blue\n";
    std::cout << "  2 = Green       A = Light Green\n";
    std::cout << "  3 = Aqua        B = Light Aqua\n";
    std::cout << "  4 = Red         C = Light Red\n";
    std::cout << "  5 = Purple      D = Light Purple\n";
    std::cout << "  6 = Yellow      E = Light Yellow\n";
    std::cout << "  7 = White       F = Bright White\n";
    std::cout << "\nDefault Colors:\n";
    std::cout << "  Admin:  4 (Red) on 0 (Black)\n";
    std::cout << "  User:   A (Light Green) on 0 (Black)\n";
    std::cout << "  System: E (Yellow) on 0 (Black)\n";
}
void SetRegistryValue(const std::string& key, const std::string& valueName, const std::string& value)
{
    HKEY hKey;
    RegOpenKeyExA(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_WRITE, &hKey);
    RegSetValueExA(hKey, valueName.c_str(), 0, REG_SZ, (BYTE*)value.c_str(), value.size() + 1);
    RegCloseKey(hKey);
}

void SetColorCodes(const std::string& adminTextColor, const std::string& userTextColor, const std::string& systemTextColor,
                   const std::string& adminBackgroundColor, const std::string& userBackgroundColor, const std::string& systemBackgroundColor)
{
    SetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "AdminTextColor", adminTextColor);
    SetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "UserTextColor", userTextColor);
    SetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "SystemTextColor", systemTextColor);
    SetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "AdminBackgroundColor", adminBackgroundColor);
    SetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "UserBackgroundColor", userBackgroundColor);
    SetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "SystemBackgroundColor", systemBackgroundColor);
}
int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        std::string arg = argv[1];
        if (arg == "--help")
        {
            ShowHelp();
            return 0;
        }
        else if (arg == "--uninstall")
        {
            UninstallRegistryEntry();
            return 0;
        }
        else if (arg == "--reinstall")
        {
            UninstallRegistryEntry();
            InstallExecutable();
            InstallRegistryEntry();
            return 0;
        }
        else if (arg == "--set-colors" && argc == 8)
        {
            // Usage: --set-colors <AdminText> <UserText> <SystemText> <AdminBg> <UserBg> <SystemBg>
            SetColorCodes(argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
            std::cout << "Color codes updated successfully." << std::endl;
            return 0;
        }
    }

    if (IsFirstRun())
    {
        std::cout << "First run detected. Installing executable and registry entries..." << std::endl;
        InstallExecutable();
        InstallRegistryEntry();
    }

    // Read color settings from registry
    std::string adminTextColor = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "AdminTextColor");
    std::string userTextColor = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "UserTextColor");
    std::string systemTextColor = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "SystemTextColor");
    std::string adminBackgroundColor = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "AdminBackgroundColor");
    std::string userBackgroundColor = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "UserBackgroundColor");
    std::string systemBackgroundColor = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "SystemBackgroundColor");
    std::string defaultLocation = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "DefaultLocation");

    // Build command strings for each mode
    std::string adminCommand = "/k color " + adminBackgroundColor + adminTextColor;
    std::string userCommand = "/k color " + userBackgroundColor + userTextColor;
    std::string systemCommand = "/k color " + systemBackgroundColor + systemTextColor;

    // Determine directory to open
    std::string directory = defaultLocation;
    if (argc > 2)
    {
        // First argument is directory if second argument is a mode flag
        std::string lastArg = argv[argc - 1];
        if (lastArg == "--admin" || lastArg == "--user" || lastArg == "--system")
        {
            directory = argv[1];
        }
    }

    // Append directory change to commands
    adminCommand += " && cd /d \"" + directory + "\"";
    userCommand += " && cd /d \"" + directory + "\"";
    systemCommand += " && cd /d \"" + directory + "\"";

    // Determine which mode to run
    std::string mode = "--user";  // Default mode
    if (argc > 1)
    {
        std::string lastArg = argv[argc - 1];
        if (lastArg == "--admin" || lastArg == "--user" || lastArg == "--system")
        {
            mode = lastArg;
        }
    }

    // Start CMD in the appropriate mode
    if (mode == "--admin")
    {
        StartCmd(adminCommand, true);
    }
    else if (mode == "--system")
    {
        StartCmd(systemCommand, true);  // System also runs elevated
    }
    else
    {
        StartCmd(userCommand, false);
    }

    return 0;
}
