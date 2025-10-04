# Contributing to System Monitor

Thank you for your interest in contributing! This document provides guidelines and information for contributors.

## Code of Conduct

We are committed to providing a welcoming and inclusive environment. Please treat all contributors with respect.

## How to Contribute

### Reporting Bugs

1. **Check existing issues** to avoid duplicates
2. **Use the bug template** when creating new issues
3. **Provide details**:
   - OS and version
   - Build configuration
   - Steps to reproduce
   - Expected vs. actual behavior
   - Logs if available

### Suggesting Features

1. **Open a discussion** before implementing large features
2. **Describe the use case** and benefits
3. **Consider alternatives** and trade-offs
4. **Be open to feedback** from maintainers

### Pull Requests

1. **Fork the repository** and create a feature branch
2. **Follow coding standards** (see below)
3. **Add tests** for new functionality
4. **Update documentation** as needed
5. **Keep commits atomic** and well-described
6. **Sign your commits** (GPG recommended)

## Development Setup

### Prerequisites

- CMake 3.20+
- C++20 compiler (GCC 10+, Clang 12+, MSVC 2019+)
- Git

### Building from Source

```bash
git clone https://github.com/yourusername/system-monitor.git
cd system-monitor
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### Running Tests

```bash
cd build
ctest --output-on-failure
```

## Coding Standards

### C++ Style

- **Standard**: C++20
- **Naming**:
  - Classes: `PascalCase`
  - Functions: `camelCase`
  - Variables: `camelCase_`
  - Constants: `UPPER_SNAKE_CASE`
  - Private members: trailing underscore

- **Formatting**:
  - Indentation: 4 spaces (no tabs)
  - Line length: 100 characters
  - Braces: K&R style

```cpp
class ExampleClass {
public:
    void doSomething() {
        if (condition) {
            // code here
        }
    }
    
private:
    int memberVariable_{0};
};
```

### Modern C++ Practices

- **Use smart pointers** instead of raw pointers
- **Use const** wherever possible
- **Use auto** when type is obvious
- **Use range-based for loops** when applicable
- **Avoid manual memory management**
- **Prefer RAII** over manual resource handling

### Error Handling

- **Use exceptions** for exceptional conditions
- **Validate inputs** at API boundaries
- **Check system call returns**
- **Log errors** appropriately
- **Fail securely** (no sensitive data in error messages)

## Security Guidelines

### Required Security Practices

1. **Input Validation**:
   ```cpp
   if (value < MIN_VALUE || value > MAX_VALUE) {
       throw std::out_of_range("Invalid value");
   }
   ```

2. **Bounds Checking**:
   ```cpp
   if (index >= vector.size()) {
       return std::nullopt;
   }
   ```

3. **Safe String Handling**:
   ```cpp
   std::string safe = input.substr(0, MAX_LENGTH);
   ```

4. **No Hardcoded Secrets**:
   - Never commit API keys, passwords, or tokens
   - Use environment variables for sensitive config
   - Run GitLeaks before committing

### Security Review Checklist

Before submitting PRs:
- [ ] No hardcoded credentials
- [ ] Input validation on all external data
- [ ] Bounds checking on array access
- [ ] No SQL injection vectors (N/A for this project)
- [ ] No command injection vectors
- [ ] Proper error handling
- [ ] Thread-safe shared data access
- [ ] No race conditions
- [ ] Memory leaks checked (valgrind/sanitizers)

## Testing Guidelines

### Unit Tests

```cpp
TEST(SystemMetrics, DefaultInitialization) {
    SystemMetrics metrics;
    EXPECT_EQ(metrics.cpuUsagePercent, 0.0);
    EXPECT_EQ(metrics.totalMemoryBytes, 0);
}
```

### Integration Tests

- Test complete workflows
- Verify thread coordination
- Check resource cleanup

### Manual Testing

- Test on all supported platforms
- Test with various terminal sizes
- Test with high process counts
- Test error conditions

## Documentation

### Code Documentation

```cpp
/**
 * @brief Collects current CPU metrics from the system
 * 
 * @param metrics Output structure to populate with CPU data
 * @throws std::runtime_error if system APIs fail
 * 
 * @note Thread-safe, can be called concurrently
 */
void collectCPUMetrics(SystemMetrics& metrics);
```

### Commit Messages

Format:
```
type(scope): brief description

Detailed explanation if needed.

Fixes #123
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

Examples:
```
feat(ui): add color-coded CPU gauges

Implements visual feedback for CPU usage levels using green/yellow/red
color progression based on utilization percentage.

Closes #45
```

## Platform-Specific Development

### Windows Development

- Use Visual Studio 2019+
- Link against pdh.lib, psapi.lib, iphlpapi.lib
- Test with Windows Defender enabled
- Verify no admin privileges required

### Linux Development

- Test on Ubuntu 20.04+ and Fedora 35+
- Verify /proc filesystem access
- Test with SELinux/AppArmor enabled
- Check for memory leaks with valgrind

### macOS Development

- Use Xcode 13+
- Request only necessary entitlements
- Test sandbox compatibility
- Verify on both Intel and Apple Silicon

## Review Process

### For Contributors

1. Ensure CI/CD passes
2. Address review comments
3. Keep PR updated with main branch
4. Be patient and responsive

### For Reviewers

1. Review within 48 hours if possible
2. Provide constructive feedback
3. Approve when ready
4. Squash-merge to keep history clean

## Release Process

1. Update version in CMakeLists.txt
2. Update CHANGELOG.md
3. Create release branch
4. Tag release (vX.Y.Z)
5. Build release artifacts
6. Publish release notes
7. Merge back to main

## Getting Help

- **Questions**: Open a discussion
- **Bugs**: Create an issue
- **Security**: Email security team (see SECURITY.md)
- **Chat**: [Link to chat platform if available]

## Recognition

Contributors will be:
- Listed in CONTRIBUTORS.md
- Credited in release notes
- Recognized in project documentation

Thank you for contributing to System Monitor!

---

**Last Updated**: October 3, 2025
