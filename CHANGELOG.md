# Changelog

## [1.2.1] - 2026-08-14

- Simplified optional registration to email only.
- Made **Not now** close registration immediately and ask again on a later launch.
- Prevented multiple registration windows from stacking when several consoles open.

## [1.2.0] - 2026-08-14

- Renamed the product to AuthorityGate ShellColors.
- Added reliable `/silent`, `/quiet`, and `/S` unattended setup modes for WinGet validation.
- Added free first-use registration and silent startup check-ins to License AuthorityGate with email, computer name, starting version, current version, and last-used time.
- Moved Windows integration into setup so installation no longer starts an interactive child console.
- Added signed update discovery for the renamed ShellColors installer.

## [1.1.1] - 2026-08-14

- Replaces the standalone release executable with a signed setup wizard
- Adds in-place upgrade handling, Program Files installation, and Windows uninstall registration
- Makes automatic updates target the versioned setup package

## [1.1.0] - 2026-08-14

- Restores distinct console color identities on Windows 11 for PowerShell, Administrator, User, and System sessions
- Adds PowerShell Start Menu and Explorer context-menu entries
- Adds automatic daily and on-demand GitHub release checks
- Rejects downloaded updates unless Authenticode validates the AuthorityGate code-signing certificate

All notable changes to CMDHelper will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-01

### Added
- **System Mode** - New `--system` option with yellow text on black background
  - Context menu entry: "AuthorityGate CMD (System)"
  - Start Menu shortcut: "CMDHelper (System).lnk"
  - Elevated prompt for system-level tasks
- Three distinct operating modes:
  - Admin: Red text (elevated)
  - User: Green text (standard)
  - System: Yellow text (elevated)
- Comprehensive README documentation
- MIT License with no-support disclaimer
- SECURITY.md for vulnerability reporting
- CHANGELOG.md for version tracking
- Improved .gitignore for Visual Studio C++ projects

### Changed
- `--set-colors` now accepts 6 parameters (added System colors)
- Updated help text to show all three modes
- Version bumped to 1.0.0 for public release

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
| 1.0.0 | Jan 2025 | Added System mode, public release |
| 0.9.0 | Feb 2025 | Initial development release |

---

## Future Considerations

The following features may be considered for future versions (no timeline or commitment):

- [ ] Windows Terminal integration option
- [ ] PowerShell mode in addition to CMD
- [ ] Configurable installation directory
- [ ] Silent installation mode for enterprise deployment
- [x] Code signing for SmartScreen compatibility
