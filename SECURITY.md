# Security Policy

## Security by Design

This project follows security-by-design and privacy-by-default principles:

### Core Security Principles

1. **Least Privilege**: The application requests only the minimum permissions needed
2. **Defense in Depth**: Multiple layers of security controls
3. **Fail Securely**: Errors result in secure states, not exposed vulnerabilities
4. **Privacy by Default**: No data collection, logging, or transmission without explicit consent
5. **Secure Defaults**: All security features enabled by default

## OWASP Compliance

### OWASP Top 10 Mitigations

1. **A01:2021 – Broken Access Control**
   - Process termination requires explicit confirmation
   - Platform APIs used correctly with proper permission checks
   - No privilege escalation attempts

2. **A02:2021 – Cryptographic Failures**
   - No sensitive data stored or transmitted
   - No encryption needed (local-only monitoring)
   - Secure memory handling for process information

3. **A03:2021 – Injection**
   - No SQL, command injection vectors (no external input execution)
   - Process names sanitized before display
   - Safe string handling throughout

4. **A04:2021 – Insecure Design**
   - Threat modeling completed
   - Security requirements defined upfront
   - Secure architecture review

5. **A05:2021 – Security Misconfiguration**
   - Secure defaults in all configurations
   - No debug output in release builds
   - Minimal attack surface

6. **A06:2021 – Vulnerable and Outdated Components**
   - Dependencies tracked and updated (FTXUI from official source)
   - SBOM (Software Bill of Materials) maintained
   - Automated dependency scanning

7. **A07:2021 – Identification and Authentication Failures**
   - No authentication required (local system only)
   - Uses OS-level authentication for process access

8. **A08:2021 – Software and Data Integrity Failures**
   - Build reproducibility ensured
   - Code signing recommended for distribution
   - No auto-updates without verification

9. **A09:2021 – Security Logging and Monitoring Failures**
   - Sensitive process info not logged by default
   - Audit trail for process termination actions
   - No PII in logs

10. **A10:2021 – Server-Side Request Forgery (SSRF)**
    - No network requests made
    - Local-only operation

### OWASP Top 10 for LLMs (AI Security)

1. **LLM01: Prompt Injection**
   - Not applicable (no LLM integration)
   - If AI features added: strict input validation required

2. **LLM02: Insecure Output Handling**
   - All terminal output sanitized
   - No code execution from displayed data
   - Control character filtering

3. **LLM03: Training Data Poisoning**
   - Not applicable (no ML models)

4. **LLM04: Model Denial of Service**
   - Not applicable (no AI models)

5. **LLM05: Supply Chain Vulnerabilities**
   - Dependency verification via CMake FetchContent
   - FTXUI fetched from official repository with version pinning
   - Checksum verification recommended

6. **LLM06: Sensitive Information Disclosure**
   - Process command-line arguments may contain secrets
   - Warning in documentation about sensitive data exposure
   - Optional filtering of command-line arguments

7. **LLM07: Insecure Plugin Design**
   - Current implementation: no plugin system
   - Future plugin architecture will require sandboxing

8. **LLM08: Excessive Agency**
   - Process termination requires explicit user confirmation
   - No automated actions on monitored processes
   - Read-only by default except explicit kill operation

9. **LLM09: Overreliance**
   - Not applicable (no AI decision-making)

10. **LLM10: Model Theft**
    - Not applicable (no AI models)

## Privacy Protection

### Data Collection Policy

**The application collects NO data whatsoever:**

- ✅ All processing is local
- ✅ No network connections made
- ✅ No telemetry or analytics
- ✅ No crash reporting unless explicitly enabled
- ✅ No cloud services integration
- ✅ No user tracking

### Privacy by Default

1. **No Logging**: Sensitive process information never written to disk
2. **Memory Safety**: Secure cleanup of process data structures
3. **No Screenshots**: Terminal output not captured automatically
4. **No History**: Command history not stored
5. **No Sharing**: No sharing features that could leak system info

### GDPR Compliance

Since the application:
- Stores no personal data
- Makes no network connections
- Processes data only in volatile memory
- Has no user accounts

It is **GDPR-compliant by design** (no PII processing).

## Reporting a Vulnerability

### Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | :white_check_mark: |
| < 1.0   | :x:                |

### How to Report

**DO NOT** open public issues for security vulnerabilities.

Instead:

1. **Email**: security@systemmonitor.local (or repository owner)
2. **Subject**: "SECURITY: [Brief Description]"
3. **Include**:
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if any)

### Response Timeline

- **Initial Response**: Within 48 hours
- **Status Update**: Within 7 days
- **Fix Timeline**: Critical issues within 30 days

### Disclosure Policy

- Vulnerabilities will be disclosed after patches are available
- Credit given to reporters (unless anonymity requested)
- CVE assignment for critical vulnerabilities

## Security Best Practices for Users

### Running the Application

1. **Least Privilege**: Run without administrator/root unless required
2. **Verify Build**: Build from source or verify signed binaries
3. **Monitor Resources**: The monitor should use <1% CPU
4. **Check Permissions**: Review what the app can access

### Secure Configuration

```ini
# .sysmonrc - Secure defaults
cpu_interval=1000
memory_interval=5000
# Never store credentials in config files
```

### Build Security

1. **Verify Dependencies**:
   ```bash
   # Check FTXUI source
   git verify-tag v5.0.0
   ```

2. **Enable Security Features**:
   ```cmake
   # CMake security flags enabled by default
   # Stack protection, ASLR, DEP
   ```

3. **Code Signing** (recommended for distribution):
   - Windows: Sign with authenticode
   - macOS: Sign with Apple Developer ID
   - Linux: GPG sign packages

## Security Features

### Memory Safety

- Modern C++20 with RAII
- Smart pointers eliminate manual memory management
- No buffer overflows (safe string handling)
- Bounds checking in all array access

### Thread Safety

- Mutex protection for shared data
- Atomic operations for flags
- No race conditions in metric collection
- Deadlock prevention via consistent lock ordering

### Input Validation

- Configuration values validated and bounded
- Process IDs verified before operations
- Path traversal prevention
- Safe integer parsing with overflow checks

### Platform Security

#### Windows
- Follows Windows Security Best Practices
- Uses official Win32 APIs
- No COM security issues
- Handle leak prevention

#### Linux
- Respects SELinux/AppArmor policies
- No /proc race conditions
- Safe signal handling
- Capability-based security where applicable

#### macOS
- Sandbox-compatible
- Hardened runtime support
- Notarization ready
- Proper entitlements

## Security Testing

### Automated Scans

- GitLeaks for secrets detection
- Static analysis (clang-tidy, cppcheck)
- Dynamic analysis (AddressSanitizer, UBSan)
- Dependency vulnerability scanning

### Manual Security Review

- Code review checklist
- Threat modeling updates
- Penetration testing for major releases

## Secure Development

### Build Process

1. **Reproducible Builds**: Same input = same output
2. **Supply Chain Security**: Dependencies from trusted sources
3. **Signed Commits**: GPG signing recommended
4. **Protected Branches**: Main branch protected

### Code Quality

- All warnings treated as errors
- Static analysis in CI/CD
- 100% const-correctness where applicable
- No undefined behavior

## Incident Response

### Security Incident Procedure

1. **Detection**: Automated scanning + user reports
2. **Triage**: Severity assessment within 24h
3. **Containment**: Temporary mitigations if needed
4. **Remediation**: Patch development and testing
5. **Recovery**: Release and notification
6. **Lessons Learned**: Post-mortem analysis

## Compliance Certifications

### Current Status

- ✅ OWASP ASVS Level 1 (in progress)
- ✅ CWE Top 25 mitigations implemented
- ✅ CERT C++ Coding Standard compliance (subset)

### Planned

- OWASP MASVS (if mobile version developed)
- ISO 27001 alignment for development process

## Security Contacts

- **Security Team**: security@systemmonitor.local
- **Bug Bounty**: Not currently available
- **Security Advisories**: GitHub Security Advisories

## Acknowledgments

We thank the security research community for responsible disclosure and the OWASP Foundation for comprehensive security guidance.

---

**Last Updated**: October 3, 2025  
**Version**: 1.0.0
