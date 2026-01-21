# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 0.9.x   | :white_check_mark: |

## Reporting a Vulnerability

If you discover a security vulnerability in CMDHelper, please report it by:

1. **DO NOT** open a public GitHub issue for security vulnerabilities
2. Email security concerns to: security@authoritygate.com
3. Include a detailed description of the vulnerability
4. Include steps to reproduce if possible

## Response Timeline

As this is an unsupported open-source utility:
- We will make reasonable efforts to review security reports
- Response times are not guaranteed
- Critical vulnerabilities will be prioritized

## Security Considerations

### Permissions Required

CMDHelper requires elevated permissions for:
- **Installation**: Writing to `C:\Program Files\` and system registry
- **Admin Mode**: Launching elevated command prompts via UAC

### What CMDHelper Does NOT Do

- Does not collect or transmit any data
- Does not connect to the internet
- Does not install any services or drivers
- Does not modify system files beyond documented registry entries
- Does not store any credentials or sensitive information

### Registry Modifications

CMDHelper creates registry entries in:
- `HKEY_CLASSES_ROOT\Directory\shell\` (context menu)
- `HKEY_LOCAL_MACHINE\SOFTWARE\AuthorityGate\CMDHelper` (settings)

All registry changes can be completely reversed using `--uninstall`.

### Code Signing

Pre-built releases are **not** code-signed. Windows SmartScreen may display a warning. Users concerned about this should:
1. Build from source using Visual Studio
2. Review the source code before building
3. Sign the resulting executable with their own certificate

## Best Practices

1. Download releases only from the official GitHub repository
2. Verify file hashes if provided with releases
3. Review source code changes before updating
4. Run `--uninstall` before removing the application manually
