# GitHub Actions & CI/CD

This directory contains GitHub Actions workflows and configuration for the AnalogIQ project.

## Workflows

### 1. Test Coverage Check (`test-coverage.yml`)

**Triggers:**
- Pull requests to main branch
- Pushes to main branch

**Purpose:**
- Runs the full test suite
- Generates code coverage reports
- Blocks PR merging if coverage is below threshold (20%)
- Uploads HTML coverage reports as artifacts
- Comments coverage results on PRs

**Key Features:**
- Uses LLVM coverage tools for accurate C++ coverage measurement
- Ignores test files and dependencies in coverage calculation
- Provides detailed HTML reports for code review
- Automatically comments results on PRs

### 2. Version Update Check (`semver.yml`)

**Triggers:**
- Pull requests to main branch

**Purpose:**
- Ensures VERSION file is updated in PRs
- Validates semantic versioning format
- Prevents merging without proper version bumps

### 3. Documentation (`docs.yml`)

**Triggers:**
- Pushes to main branch

**Purpose:**
- Generates and deploys documentation

### 4. Release Artifacts (`release-artifacts.yml`)

**Triggers:**
- Release creation

**Purpose:**
- Builds and uploads release artifacts

## Branch Protection

The main branch is protected with the following rules:

### Required Status Checks
- `test-coverage` - Must pass before merging
- `check-version-update` - Must pass before merging

### Protection Rules
- Require status checks to pass before merging
- Require branches to be up to date before merging
- Require pull request reviews before merging
- Restrict direct pushes to main branch

## Coverage Threshold

**Current Threshold:** 20.0% (defined in `COVERAGE_THRESHOLD` file)

This threshold is enforced in:
- `run_tests.sh` (local development)
- `.github/workflows/test-coverage.yml` (CI/CD)

Both read from the single source of truth: `COVERAGE_THRESHOLD`

## Local Development

To run tests locally with coverage:

```bash
./run_tests.sh
```

This will:
1. Build tests with coverage instrumentation
2. Run the test suite
3. Generate coverage reports
4. Check against the 20% threshold
5. Generate HTML coverage report

## Coverage Reports

HTML coverage reports are available:
- **Local:** `coverage_html/index.html`
- **CI/CD:** Download from GitHub Actions artifacts

## Troubleshooting

### Coverage Below Threshold
If coverage is below 20%:
1. Add more tests to cover uncovered code paths
2. Focus on critical business logic first
3. Consider edge cases and error conditions
4. Review the HTML coverage report to identify gaps

### Test Failures
If tests fail in CI:
1. Run tests locally first: `./run_tests.sh`
2. Check for platform-specific issues
3. Verify all dependencies are properly mocked
4. Ensure test isolation is maintained

### Build Failures
If the build fails:
1. Check that all dependencies are installed
2. Verify CMake configuration
3. Ensure JUCE modules are properly linked
4. Check for compiler version compatibility

## Configuration

### Coverage Threshold
To change the coverage threshold:
1. Update the `COVERAGE_THRESHOLD` file (single source of truth)
2. Both local and CI/CD will automatically use the new value
3. Update this documentation if needed

### Test Configuration
Test configuration is in:
- `tests/main.cpp` - Test runner configuration
- `tests/CMakeLists.txt` - Build configuration
- `run_tests.sh` - Local test execution

## Future Improvements

Planned enhancements:
1. **Coverage Trend Analysis** - Track coverage over time
2. **Coverage Badges** - Display coverage in README
3. **Coverage Alerts** - Notify on coverage drops
4. **Performance Testing** - Add performance benchmarks
5. **Security Scanning** - Add security vulnerability scanning 