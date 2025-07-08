# Test Documentation

## Known JUCE Assertion Failures

### Overview
During test execution, you may see JUCE internal assertion failures in the output. These are **expected and safe to ignore**. They do not indicate test failures or problems with your code.

### Common Assertion Messages
The following assertion messages commonly appear during tests:

```
JUCE Assertion failure in juce_File.cpp:219
JUCE Assertion failure in juce_LookAndFeel.cpp:82
JUCE Assertion failure in juce_String.cpp:1401
JUCE Assertion failure in juce_LeakedObjectDetector.h:104
```

### Why These Occur
These assertions are triggered by JUCE's internal validation system when:
1. **Creating real JUCE components** in tests (PluginEditor, DraggableListBox, etc.)
2. **JUCE's file system** validating paths and operations
3. **JUCE's LookAndFeel system** accessing system resources
4. **JUCE's string handling** performing internal validation
5. **JUCE's leak detection** finding objects that weren't explicitly deleted

### Impact
- **No functional impact**: Tests still pass and validate correct behavior
- **No runtime impact**: Your actual plugin will work fine in DAWs
- **No code quality impact**: These are JUCE internals, not your code

### When to Investigate
Only investigate if you see:
- Assertions from your own code files (not JUCE internal files)
- Test failures accompanied by assertions
- Assertions that don't match the patterns above

### Future Improvements
To eliminate these assertions, consider:
- Implementing full GUI/logic separation
- Using mock components instead of real JUCE components
- Creating test-specific component implementations

### Current Status
These assertions are **accepted as known behavior** and do not require immediate action. The test suite is functional and reliable despite these messages. 