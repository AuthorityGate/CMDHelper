# Changelog

All notable changes to CMDHelper will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Comprehensive README documentation
- MIT License with no-support disclaimer
- SECURITY.md for vulnerability reporting
- CHANGELOG.md for version tracking
- Improved .gitignore for Visual Studio C++ projects

## [0.9.0] - 2025-02

### Added
- Initial public release
- Windows Explorer context menu integration
  - "AuthorityGate CMD (Admin)" for elevated prompts
  - "AuthorityGate CMD (User)" for standard prompts
- Context menu on folder background (right-click empty space)
- Self-installing functionality on first run
- Start Menu shortcuts under "AuthorityGate Utilities" folder
- Customizable color schemes for Admin/User modes
- Command-line options: `--help`, `--admin`, `--user`, `--reinstall`, `--uninstall`, `--set-colors`
- Custom application icon (`Authority_Gate_CMD.ico`)
  - Embedded in executable as Windows resource
  - Displayed in Explorer context menus
  - Used for Start Menu shortcuts
  - Copied to installation directory for registry references
- System PATH integration
- Registry-based configuration storage

### Notes
- Icon must be in same directory as executable during first-run installation
- Icon path stored in registry at `HKLM\SOFTWARE\AuthorityGate\CMDHelper\IconPath`

### Technical
- Built with Visual Studio 2022 (v143 toolset)
- Windows 10 SDK compatibility
- Support for both x64 and Win32 platforms
- COM Shell Link API for shortcut creation
- Win32 Registry API for configuration

## Version History

| Version | Date | Notes |
|---------|------|-------|
| 0.9.0 | Feb 2025 | Initial public release |

---

## Future Considerations

The following features may be considered for future versions (no timeline or commitment):

- [ ] Windows Terminal integration option
- [ ] PowerShell mode in addition to CMD
- [ ] Configurable installation directory
- [ ] Silent installation mode for enterprise deployment
- [ ] Code signing for SmartScreen compatibility
