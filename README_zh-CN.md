# LeetCode Debugger for C++

> 为你的[LeetCode](https://leetcode.com/) C++ 代码在[VSCode](https://code.visualstudio.com/)中提供调试支持.

- [English Document](https://github.com/XavierCai1996/vscode-leetcode-cpp-debug/blob/master/README.md) | **中文文档**

## 快速开始

![demo](https://raw.githubusercontent.com/XavierCai1996/vscode-leetcode-cpp-debug/master/docs/imgs/demo.gif)

> **注意**: 在使用此插件启动调试前, 你必须确定你有可用的C++调试工具. 从[官方文档](https://code.visualstudio.com/docs/cpp/config-mingw#cpp-atricles)可以获取更多相关信息.

## 特性

### 启动调试

- 为你的题解代码生成调试代码并启动调试.

- 通过在命令输入板(`Ctrl/Cmd + Shift + P`)输入命令`LeetCode Debugger: Start Debugging`来启动.

### 在线/离线 代码模板

- 代码模板是用来生成调试代码的.

- 在线: 从`LeetCode`官网获取题目的信息. 要求你的题解代码文件名以该题的ID开始, 例如`1.两数之和.cpp` (或者你可以在插件设置页面中自行设置用于ID匹配的正则表达式).

- 离线: 直接使用你的题解代码作为代码模板.

### 输入/输出

- 你可以使用以下代码来设置输入/输出

    ```cpp
    #define INPUT "test_case.txt" // single-input
    #define OUTPUT cout, "output.txt" // multi-output
    ```

- 对于`INPUT`, `std::istream`和`std::string` (从文件输入) 都是可接受的参数类型, 但是你只能有**一个**输入.

- 对于`OUTPUT`, `std::ostream`和`std::string` (输出到文件) 都是可接受的参数类型, 你可以同时有多个输出.

### 交互题

- 交互题目前**不**支持.

- 但是你可以自己实现交互函数! 可以通过[API](https://github.com/XavierCai1996/vscode-leetcode-cpp-debug/blob/master/docs/api_zh-CN.md)和[示例](https://github.com/XavierCai1996/vscode-leetcode-cpp-debug/blob/master/docs/examples_zh-CN.md)了解更多. 这里有一个[题278](https://leetcode-cn.com/problems/first-bad-version/)的例子.

    ```cpp
    #ifdef LEETCODE_DEFINITION // protection
    int firstVersion; // the first bad version
    bool isBadVersion(int version) { // realization
        return version >= firstVersion;
    }
    #define LAZY_INTERACTION firstVersion // input firstVersion
    #endif
    ```

- `LAZY_INTERACTION`指的是在函数参数输入之后的交互输入, 而`INTERACTION`则是在函数参数输入之前的交互输入, 此外, `void ()`类型的函数同样可以作为交互输入. 下面的例子将帮助你理解这些内容.

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
    }

    /*
     * input:
     * 1 // -> value1 -> value1*=2 -> 2
     * 2 // -> v
     * 3 // -> value2 -> value2*=value1 -> 6
     * output:
     * 24 // -> v*value1*value2 -> 2*2*6=24
     * /
    ```

## 插件设置

设置项|描述|默认值
:---|:---|:---
`Source`|用于生成调试代码的代码模板来源.|`"[online]leetcode.com"`
`Delete Temporary Contents`|在调试结束后，是否删除生成的临时文件与代码.|`true`
`Id Match Pattern`|在线获取代码模板时, 用于捕获文件名中问题ID的正则表达式.|`"(\\d+).*"`
`Output File Encoding`|生成的临时代码文件编码|`"utf8"`

- `Id Match Pattern`的默认值能匹配任何以数字开头的文件名.
- 中文下遇到编码问题如中文头文件无法识别时, 可以尝试将`Ouput File Encoding`设置为`gbk`

## 版本日志

详细内容请查看[CHANGELOG](https://github.com/XavierCai1996/vscode-leetcode-cpp-debug/blob/master/CHANGELOG.md).

## 小贴士

你可以使用另外一款很棒的插件[LeetCode](https://marketplace.visualstudio.com/items?itemName=shengchen.vscode-leetcode)来在VS Code中浏览和解决LeetCode问题.
