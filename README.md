# LeetCode Debugger for C++

> Debug support for [LeetCode](https://leetcode.com/) with C++ in [VSCode](https://code.visualstudio.com/)

- **English Document** | [中文文档](https://github.com/XavierCai1996/vscode-leetcode-cpp-debug/blob/master/README_zh-CN.md)

## Quick Start

![demo](https://raw.githubusercontent.com/XavierCai1996/vscode-leetcode-cpp-debug/master/docs/imgs/demo.gif)

> **Attention**: Before start debugging, you must check availability of your C++ debugger tools. Get more information from [VSCode documents](https://code.visualstudio.com/docs/cpp/config-mingw#cpp-atricles).

## Features

### Start Debugging

- Generate debugging code and start a debug session for your solution.

- You can run the `LeetCode Debugger: Start Debugging` command from the command palette (`Ctrl/Cmd + Shift + P`).

### Online/Offline Code Template

- Code template is used to generate the debugging code.

- Online: Fetching problem from `LeetCode`. Requires your solution file start with problem ID, like `1.two-sum.cpp` (Or you can modify the regular expression for capturing problem ID in extension settings).

- Offline: Using your solution code as code template.

### Input/Output

- You can use the code below to change input/output:

    ```cpp
    #define INPUT "test_case.txt" // single-input
    #define OUTPUT cout, "output.txt" // multi-output
    ```

- For `INPUT`, both `std::istream` and `std::string` (input from file) are acceptable, but you can only have **ONE** input.

- For `OUTPUT`, both `std::ostream` and `std::string` (output to file) are acceptable, you can have multiple outputs.

### Interactive Problem

- Interactive problem is **NOT** supported yet.

- But you can realize the interactive function by yourself! Know more from [API](https://github.com/XavierCai1996/vscode-leetcode-cpp-debug/blob/master/docs/api.md) and [examples](https://github.com/XavierCai1996/vscode-leetcode-cpp-debug/blob/master/docs/examples.md). Here is an example for [problem 278](https://leetcode.com/problems/first-bad-version/).

    ```cpp
    #ifdef LEETCODE_DEFINITION // protection
    int firstVersion; // the first bad version
    bool isBadVersion(int version) { // realization
        return version >= firstVersion;
    }
    #define LAZY_INTERACTION firstVersion // input firstVersion
    #endif
    ```

- `LAZY_INTERACTION` means interactive inputs after function's arguments, `INTERACTION` means interactive inputs before function's arguments, in addition, function with type `void ()` is also acceptable. The example below will help you to understand these stuff.

    ```cpp
    #ifdef LEETCODE_DEFINITION // protection
    int value1, value2; // interactive values
    void before() {
        value1 *= 2;
    }
    void after() {
        value2 *= value1;
    }
    #define INTERACTION value1, before // input value1, then call the function 'before()'
    #define LAZY_INTERACTION value2, after // input value2, then call the function 'after()'
    #endif

    class Solution {
    public:
        int exampleFunction(int v) {
            return v * value1 * value2;
        }
    };

    /*
     * input:
     * 1 // -> value1 -> value1*=2 -> 2
     * 2 // -> v
     * 3 // -> value2 -> value2*=value1 -> 6
     * output:
     * 24 // -> v*value1*value2 -> 2*2*6=24
     * /
    ```

## Extension Settings

Setting Name|Description|Default Value
:---|:---|:---
`Source`|Source of code template for generating debugging code.|`"[online]leetcode.com"`
`Delete Temporary Contents`|Delete temporary codes and files after debugging.|`true`
`Id Match Pattern`|Regular expression for capturing problem ID when fetching problem online.|`"(\\d+).*"`
`Output File Encoding`|Encoding of temporary code files|`"utf8"`

- The default value of `Id Match Pattern` can match any file name begin with a number.

## Release Notes

Refer to [CHANGELOG](https://github.com/XavierCai1996/vscode-leetcode-cpp-debug/blob/master/CHANGELOG.md).

## Tips

You can solve LeetCode problems in VSCode with another amazing extension [LeetCode](https://marketplace.visualstudio.com/items?itemName=shengchen.vscode-leetcode).
