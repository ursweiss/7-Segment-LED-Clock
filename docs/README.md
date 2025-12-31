# Documentation Index

This directory contains technical documentation for the 7-Segment LED Clock project.

## Available Documentation

### [API Reference](API.md)

REST API endpoint documentation for the web-based configuration interface. Includes all 8 endpoints with request/response examples, error codes, and security considerations.

**Contents:**

- Complete endpoint reference (GET /, /api/config, /api/schema, etc.)
- Request/response examples with curl commands
- Error handling and status codes
- Security and rate limiting notes

### [Architecture Overview](ARCHITECTURE.md)

High-level system architecture with component diagrams and data flow descriptions.

**Contents:**

- System component diagram (Mermaid)
- Component descriptions (ConfigManager, WiFi_Manager, LED_Clock, etc.)
- Data flow diagrams (configuration, display, weather, WiFi recovery)
- Task scheduling architecture
- Memory architecture and flash/RAM usage
- Boot sequence and error handling

### [Code Review Report - December 30, 2025](code-review/code-review-report_2025-12-30_1.md)

Comprehensive reassessment after Phases 3-7 improvements. Health score improved from 7.0/10 to 8.5/10.

**Contents:**

- Executive summary with health score comparison (7.0 → 8.5)
- Analysis of 28 resolved issues (60% resolution rate)
- 22 remaining issues categorized by priority
- Detailed assessment across security, error handling, performance, code quality
- Recommendations for next steps (security fixes and testing)

### [Code Review Report - December 28, 2025](code-review/code-review-report_2025-12-28_1.md)

Initial comprehensive code quality assessment (47 identified issues).

**Contents:**

- Executive summary with health score
- 47 identified issues categorized by priority
- Security analysis (TLS, authentication, credentials)
- Error handling and robustness review
- Performance assessment
- Code quality evaluation
- Testing status and gaps
- Detailed recommendations

## Additional Resources

- **[Main README](../README.md)** - Project overview, setup instructions, and feature list
- **[TODO List](../TODO.md)** - Future improvements and feature requests
- **[License](../LICENSE.md)** - GNU GPL v3.0 license text
- **[Copilot Instructions](../.github/copilot-instructions.md)** - Development guidelines for contributors

## Need More Documentation?

The project is continuously being improved. Planned documentation additions include:

- Hardware assembly guide with wiring diagrams
- Detailed configuration walkthrough
- Development environment setup guide
- Security best practices documentation
- Testing documentation (when tests are implemented)

For the latest updates, check the [GitHub repository](https://github.com/ursweiss/7-Segment-LED-Clock).
