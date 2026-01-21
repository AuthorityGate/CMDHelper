# Contributing to CMDHelper

Thank you for your interest in CMDHelper. This document provides guidelines for contributions.

## Project Status

CMDHelper is provided as an **unsupported open-source utility**. This means:

- Bug reports are welcome but may not receive timely responses
- Pull requests will be reviewed when time permits
- Feature requests will be considered but not guaranteed
- No SLA or support commitments exist

## How to Contribute

### Reporting Bugs

1. Check existing [Issues](https://github.com/AuthorityGate/CMDHelper/issues) to avoid duplicates
2. Use the bug report template if available
3. Include:
   - Windows version (e.g., Windows 11 23H2)
   - CMDHelper version
   - Steps to reproduce
   - Expected vs actual behavior
   - Screenshots if applicable

### Suggesting Features

1. Open an Issue with the "enhancement" label
2. Describe the use case and expected behavior
3. Explain why this would benefit other users

### Submitting Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Make your changes
4. Test thoroughly on Windows 10 and 11
5. Commit with clear messages
6. Push to your fork
7. Open a Pull Request

### Code Style

- Follow existing code conventions in the project
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions focused and reasonably sized

### Build Requirements

- Visual Studio 2022 with C++ Desktop Development workload
- Windows 10 SDK (10.0 or later)
- Target both x64 and Win32 configurations

## What We're Looking For

Contributions that are most likely to be accepted:

- Bug fixes with clear reproduction steps
- Documentation improvements
- Security fixes (please report privately first)
- Performance improvements
- Code cleanup and modernization

## What We're Less Likely to Accept

- Major architectural changes without prior discussion
- Features that significantly increase complexity
- Changes that break backward compatibility
- Platform ports (macOS, Linux) - this is Windows-specific by design

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Questions?

For questions about contributing, open an Issue with the "question" label.

---

*Note: This is an unsupported project. Contributions are appreciated but response times vary.*
