# CMDHelper

A lightweight Windows utility that enhances the command prompt experience with context menu integration, admin/user mode switching, and customizable color schemes.

![Windows](https://img.shields.io/badge/platform-Windows%2010%2B-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Version](https://img.shields.io/badge/version-0.9-orange)

## Features

- **Right-Click Context Menu** - Open CMD in any folder directly from Windows Explorer
- **Admin/User Mode** - Quickly switch between elevated and standard prompts
- **Color-Coded Terminals** - Visual distinction between Admin (red) and User (green) sessions
- **Self-Installing** - First run automatically installs and configures the utility
- **Customizable** - Configure colors via command line or registry

## Screenshots

| Admin Mode (Red) | User Mode (Green) |
|------------------|-------------------|
| Elevated prompt with red text | Standard prompt with green text |

## Installation

### Option 1: Download Release
1. Download the latest `CMDHelper.exe` from [Releases](https://github.com/AuthorityGate/CMDHelper/releases)
2. Run `CMDHelper.exe` - it will self-install on first run
3. Right-click any folder to see the new context menu options

### Option 2: Build from Source
```bash
# Clone the repository
git clone https://github.com/AuthorityGate/CMDHelper.git
cd CMDHelper

# Open in Visual Studio 2022
start CMDHelper.sln

# Build Release|x64 configuration
```

**Requirements:**
- Visual Studio 2022 (v143 toolset)
- Windows 10 SDK

## Usage

### Context Menu
After installation, right-click on any folder or folder background:
- **AuthorityGate CMD (Admin)** - Opens elevated command prompt
- **AuthorityGate CMD (User)** - Opens standard command prompt

### Command Line

```
CMDHelper.exe [options] [directory]

Options:
  --help           Show help message
  --admin          Open CMD as Administrator
  --user           Open CMD as standard User
  --reinstall      Reinstall registry entries and shortcuts
  --uninstall      Remove all registry entries and shortcuts
  --set-colors     Customize terminal colors

Examples:
  CMDHelper.exe --admin                    # Open admin prompt in default location
  CMDHelper.exe "C:\Projects" --admin      # Open admin prompt in C:\Projects
  CMDHelper.exe --set-colors 4 A 0 0       # Red admin text, green user text, black backgrounds
```

### Color Codes

Use these codes with `--set-colors <AdminText> <UserText> <AdminBg> <UserBg>`:

| Code | Color | Code | Color |
|------|-------|------|-------|
| 0 | Black | 8 | Gray |
| 1 | Blue | 9 | Light Blue |
| 2 | Green | A | Light Green |
| 3 | Aqua | B | Light Aqua |
| 4 | Red | C | Light Red |
| 5 | Purple | D | Light Purple |
| 6 | Yellow | E | Light Yellow |
| 7 | White | F | Bright White |

**Default colors:**
- Admin: Red text (4) on Black background (0)
- User: Light Green text (A) on Black background (0)

## Installation Details

CMDHelper installs to:
```
C:\Program Files\AuthorityGate\CMDHelper\
├── CMDHelper.exe
└── Authority_Gate_CMD.ico
```

### Icon Configuration

The application uses a custom icon (`Authority_Gate_CMD.ico`) which appears in:
- Windows Explorer context menu entries
- Start Menu shortcuts
- The executable file itself (embedded resource)

**Important:** When running CMDHelper for the first time, ensure `Authority_Gate_CMD.ico` is in the same directory as `CMDHelper.exe`. The installer copies both files to the Program Files location.

If the icon doesn't appear in context menus:
1. Verify the icon exists at `C:\Program Files\AuthorityGate\CMDHelper\Authority_Gate_CMD.ico`
2. Run `CMDHelper.exe --reinstall` as Administrator
3. Restart Windows Explorer or log out/in to refresh the icon cache

To manually refresh the Windows icon cache:
```batch
ie4uinit.exe -show
```

Start Menu shortcuts are created in:
```
Start Menu\Programs\AuthorityGate Utilities\
├── CMDHelper (Admin).lnk
├── CMDHelper (User).lnk
├── CMDHelper (Reinstall).lnk
└── CMDHelper (Uninstall).lnk
```

Registry entries are created at:
- `HKEY_CLASSES_ROOT\Directory\shell\OpenCmdHereAsAdmin`
- `HKEY_CLASSES_ROOT\Directory\shell\OpenCmdHereAsUser`
- `HKEY_CLASSES_ROOT\Directory\Background\shell\OpenCmdHereAsAdmin`
- `HKEY_CLASSES_ROOT\Directory\Background\shell\OpenCmdHereAsUser`
- `HKEY_LOCAL_MACHINE\SOFTWARE\AuthorityGate\CMDHelper`

## Uninstallation

### Option 1: Using the utility
```bash
CMDHelper.exe --uninstall
```

### Option 2: Manual removal
1. Delete `C:\Program Files\AuthorityGate\CMDHelper\`
2. Delete Start Menu shortcuts from `AuthorityGate Utilities`
3. Remove registry keys (run as Administrator):
```batch
reg delete "HKCR\Directory\shell\OpenCmdHereAsAdmin" /f
reg delete "HKCR\Directory\shell\OpenCmdHereAsUser" /f
reg delete "HKCR\Directory\Background\shell\OpenCmdHereAsAdmin" /f
reg delete "HKCR\Directory\Background\shell\OpenCmdHereAsUser" /f
reg delete "HKLM\SOFTWARE\AuthorityGate\CMDHelper" /f
```

## Building

### Prerequisites
- Windows 10 or later
- Visual Studio 2022 with C++ Desktop Development workload
- Windows 10 SDK (10.0 or later)

### Build Steps
1. Open `CMDHelper.sln` in Visual Studio 2022
2. Select configuration: `Release | x64` (recommended) or `Release | Win32`
3. Build → Build Solution (Ctrl+Shift+B)
4. Output: `x64\Release\CMDHelper.exe`

### Build Configurations
| Configuration | Platform | Use Case |
|---------------|----------|----------|
| Release x64 | 64-bit | Modern Windows (recommended) |
| Release Win32 | 32-bit | Legacy compatibility |
| Debug x64 | 64-bit | Development/debugging |
| Debug Win32 | 32-bit | Development/debugging |

## Technical Details

- **Language:** C++ (C++17)
- **Framework:** Win32 API
- **Dependencies:** None (standalone executable)
- **Size:** ~50KB (Release build)
- **Privileges:** Requires Administrator for installation; context menu can launch Admin or User mode

### How It Works
1. On first run, detects absence of registry key and triggers installation
2. Copies executable to Program Files
3. Creates registry entries for Explorer context menu integration
4. Creates Start Menu shortcuts via COM Shell Link interface
5. Adds install directory to system PATH
6. On subsequent runs, reads color settings from registry and launches CMD with appropriate parameters

## FAQ

**Q: Why does Windows SmartScreen warn about this application?**  
A: The executable is not code-signed. You can click "More info" → "Run anyway" or build from source.

**Q: Can I change the installation directory?**  
A: Currently, the installation path is hardcoded. You can modify `CMDHelper.cpp` and rebuild if needed.

**Q: Does this work with Windows Terminal?**  
A: This utility specifically launches `cmd.exe`. Windows Terminal integration would require different implementation.

**Q: How do I reset to default colors?**  
A: Run `CMDHelper.exe --set-colors 4 A 0 0`

## Contributing

This is a standalone utility provided as-is. While we welcome bug reports via GitHub Issues, please note:

- This project is provided without support or warranty
- Pull requests may be reviewed but response times are not guaranteed
- Feature requests will be considered but not guaranteed

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Disclaimer

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED. USE AT YOUR OWN RISK. THE AUTHORS AND COPYRIGHT HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY ARISING FROM THE USE OF THIS SOFTWARE.

---

**AuthorityGate, Inc.** - [authoritygate.com](https://authoritygate.com)
