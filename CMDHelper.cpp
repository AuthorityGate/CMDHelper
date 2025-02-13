// App CMD defaults for users.cpp
// Version: 0.9
// Author: Kevin E. Komlosy
// Company: AuthorityGate Inc.
// Created: 2/2025  Last Modified 2/2025
// Description: This application sets up CMD defaults for users and administrators, including context menu entries and start menu shortcuts.
// Changes:
// - Added functionality to create Windows Start Menu entries under the folder "AuthorityGate Utilities" with shortcuts for --Admin, --User, --Reinstall, and --Uninstall.
// - Embedded an icon into the executable and associated it with the executable in folder view.
// - Added system variables for color, default file location, and icon path.
// - Updated registry entries to use the embedded icon from the executable.

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

    CreateShortcut(exePath, "--admin", folderPath + "\\CMDHelper (Admin).lnk", "Open CMDHelper as Admin");
    CreateShortcut(exePath, "--user", folderPath + "\\CMDHelper (User).lnk", "Open CMDHelper as User");
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
    // Add context menu entry for directories (Admin)
    HKEY hKey;
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

    // Add context menu entry for background of directories (Admin)
    RegCreateKeyExA(HKEY_CLASSES_ROOT, "Directory\\Background\\shell\\OpenCmdHereAsAdmin", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"AuthorityGate CMD (Admin)", strlen("AuthorityGate CMD (Admin)") + 1);
    RegSetValueExA(hKey, "Icon", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico") + 1);
    RegCreateKeyExA(hKey, "command", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%V\" --admin", strlen("\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%V\" --admin") + 1);
    RegCloseKey(hKey);

    RegCreateKeyExA(HKEY_CLASSES_ROOT, "Directory\\Background\\shell\\OpenCmdHereAsUser", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"AuthorityGate CMD (User)", strlen("AuthorityGate CMD (User)") + 1);
    RegSetValueExA(hKey, "Icon", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico") + 1);
    RegCreateKeyExA(hKey, "command", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%V\" --user", strlen("\"C:\\Program Files\\AuthorityGate\\CMDHelper\\CmdHelper.exe\" \"%V\" --user") + 1);
    RegCloseKey(hKey);

    // Create system variables for color and default file location
    RegCreateKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\AuthorityGate\\CMDHelper", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, "AdminTextColor", 0, REG_SZ, (BYTE*)"4", 3);
    RegSetValueExA(hKey, "UserTextColor", 0, REG_SZ, (BYTE*)"A", 3);
    RegSetValueExA(hKey, "AdminBackgroundColor", 0, REG_SZ, (BYTE*)"0", 2);
    RegSetValueExA(hKey, "UserBackgroundColor", 0, REG_SZ, (BYTE*)"0", 2);
    RegSetValueExA(hKey, "DefaultLocation", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CmdHelper", strlen("C:\\Program Files\\AuthorityGate\\CmdHelper") + 1);
    RegSetValueExA(hKey, "IconPath", 0, REG_SZ, (BYTE*)"C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico", strlen("C:\\Program Files\\AuthorityGate\\CMDHelper\\Authority_Gate_CMD.ico") + 1);
    RegCloseKey(hKey);

    std::cout << "Registry entries installed successfully." << std::endl;
}

void UninstallRegistryEntry()
{
    // Remove context menu entry for directories (Admin)
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "Directory\\shell\\OpenCmdHereAsAdmin");
    // Remove context menu entry for directories (User)
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "Directory\\shell\\OpenCmdHereAsUser");
    // Remove context menu entry for background of directories (Admin)
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "Directory\\Background\\shell\\OpenCmdHereAsAdmin");
    // Remove context menu entry for background of directories (User)
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "Directory\\Background\\shell\\OpenCmdHereAsUser");
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
    std::cout << "\nAn AuthorityGate Utility\n";
    std::cout << "Usage: CmdHelper [options] [directory]\n";
    std::cout << "Options:\n";
    std::cout << "  --help           Show this help message\n";
    std::cout << "  --uninstall      Uninstall registry entries\n";
    std::cout << "  --reinstall      Reinstall registry entries\n";
    std::cout << "  --admin          Open CMD as Admin\n";
    std::cout << "  --user           Open CMD as User\n";
    std::cout << "  --set-colors     Set color codes for Admin and User\n";
    std::cout << "                   Usage: --set-colors <AdminTextColor> <UserTextColor> <AdminBackgroundColor> <UserBackgroundColor>\n";
    std::cout << "\nColor Codes:\n";
    std::cout << "  0 = Black       8 = Gray\n";
    std::cout << "  1 = Blue        9 = Light Blue\n";
    std::cout << "  2 = Green       A = Light Green\n";
    std::cout << "  3 = Aqua        B = Light Aqua\n";
    std::cout << "  4 = Red         C = Light Red\n";
    std::cout << "  5 = Purple      D = Light Purple\n";
    std::cout << "  6 = Yellow      E = Light Yellow\n";
    std::cout << "  7 = White       F = Bright White\n";
}
void SetRegistryValue(const std::string& key, const std::string& valueName, const std::string& value)
{
    HKEY hKey;
    RegOpenKeyExA(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_WRITE, &hKey);
    RegSetValueExA(hKey, valueName.c_str(), 0, REG_SZ, (BYTE*)value.c_str(), value.size() + 1);
    RegCloseKey(hKey);
}

void SetColorCodes(const std::string& adminTextColor, const std::string& userTextColor, const std::string& adminBackgroundColor, const std::string& userBackgroundColor)
{
    SetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "AdminTextColor", adminTextColor);
    SetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "UserTextColor", userTextColor);
    SetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "AdminBackgroundColor", adminBackgroundColor);
    SetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "UserBackgroundColor", userBackgroundColor);
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
        else if (arg == "--set-colors" && argc == 6)
        {
            std::string adminTextColor = argv[2];
            std::string userTextColor = argv[3];
            std::string adminBackgroundColor = argv[4];
            std::string userBackgroundColor = argv[5];
            SetColorCodes(adminTextColor, userTextColor, adminBackgroundColor, userBackgroundColor);
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

    std::string AdminTextColor = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "AdminTextColor");
    std::string UserTextColor = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "UserTextColor");
    std::string adminBackgroundColor = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "AdminBackgroundColor");
    std::string userBackgroundColor = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "UserBackgroundColor");
    std::string defaultLocation = GetRegistryValue("SOFTWARE\\AuthorityGate\\CMDHelper", "DefaultLocation");

    // Debug output to verify registry values
    std::cout << "AdminTextColor: " << AdminTextColor << std::endl;
    std::cout << "UserTextColor: " << UserTextColor << std::endl;
    std::cout << "AdminBackgroundColor: " << adminBackgroundColor << std::endl;
    std::cout << "UserBackgroundColor: " << userBackgroundColor << std::endl;
    std::cout << "DefaultLocation: " << defaultLocation << std::endl;

    std::string adminCommand = "/k color " + adminBackgroundColor + AdminTextColor;
    std::string userCommand = "/k color " + userBackgroundColor + UserTextColor;

    if (argc > 2)
    {
        std::string directory = argv[1];
        adminCommand += " && cd /d \"" + directory + "\"";
        userCommand += " && cd /d \"" + directory + "\"";
    }
    else
    {
        adminCommand += " && cd /d \"" + defaultLocation + "\"";
        userCommand += " && cd /d \"" + defaultLocation + "\"";
    }

    if (argc > 1 && std::string(argv[argc - 1]) == "--admin")
    {
        // Start CMD in Admin mode
        StartCmd(adminCommand, true);
    }
    else
    {
        // Start CMD in User mode
        StartCmd(userCommand, false);
    }

    return 0;
}
