# Testify 🧪

![Language](https://img.shields.io/badge/language-C-blue)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)
![Status](https://img.shields.io/badge/status-active-success)

A fast, JSON-based tool for black-box testing program output and behavior.

---

## Table of Contents
- [Overview](#overview-)
- [Quick Start](#quick-start-)
- [Installation](#installation-)
  - [Fedora](#fedora)
  - [Debian-based distros](#debian-based-distros)
  - [Arch Linux](#arch-linux)
- [Features](#features-)
- [JSON Structure](#json-structure-)
- [Assertions](#assertions-)
- [Complex Test Cases](#complex-test-cases-)
  - [Complete Complex Test Case Example](#complete-complex-test-case-example-)
  - [Structure of Complex Test Cases](#structure-of-complex-test-cases-%EF%B8%8F)
- [Productivity & Development Speed](#productivity--development-speed-)
- [A Few More Features](#a-few-more-features-)

---

## Overview 📌
**Testify** focuses on **user-level testing**. Instead of relying on in-code assertions, Testify determines whether a test passes or fails by **analyzing the program’s output and exit behavior**.

This means:

- ✅ No need to modify your source code
- ✅ No test-specific logic inside the program
- ✅ True black-box testing

Testify is ideal for CLI tools, scripts, compilers, interpreters, and other system-level programs.

---

## Quick Start ⚡

Clone, build, and install:
```bash
git clone https://github.com/parssarica/testify.git
cd testify
sudo make install
````

Create a simple `testcases.json`:

```json
{
  "binary": "./hello",
  "testcases": [
    {
      "type": 1,
      "name": "Hello test",
      "commandArgs": [],
      "expectedOutput": "Hello, world!\n"
    }
  ]
}
```

Run your tests:

```bash
testify run
```

---

## Installation 🔧

### Fedora

```bash
sudo dnf install cjson cjson-devel pcre2 pcre2-devel
sudo make install
```

### Debian-based distros

```bash
sudo apt install libcjson-dev libpcre2-dev
sudo make install
```

### Arch Linux

```bash
sudo pacman -S cjson pcre2
sudo make install
```

---

## Features ✨

* JSON-defined test cases
* Output-based assertions
* Regex matching
* Command-based complex test cases
* Colored output and readable reports
* Execution summaries
* Timeout handling

---

## JSON Structure 🧩

Minimal example:

```json
{
  "binary": "./program",
  "environmentalVariables": ["ENV=5"],
  "testcases": [
    {
      "type": 1,
      "name": "Example test",
      "description": "Optional description",
      "commandArgs": ["arg1"],
      "expectedOutput": "Output\n"
    }
  ]
}
```

### Root Fields

* **binary**: Path to the executable or script to test
* **environmentalVariables**: Global environment variables applied to all tests
* **testcases**: Array of test case objects

### Test Case Fields

* **type**: `1` for simple tests, `2` for complex tests
* **name**: Human-readable test name
* **description**: Optional description
* **commandArgs**: Command-line arguments passed to the program

---

## Assertions ✅

Simple test cases support the following assertions:

* `expectedOutput`
* `notExpectedOutput`
* `containingOutput`
* `notContainingOutput`
* `exitCodeEquals`
* `exitCodeSmaller`
* `exitCodeGreater`
* `matchRegex`
* `notMatchRegex`

Assertions (except exit code checks) may accept arrays:

```json
{
  "containingOutput": ["foo", "bar"],
  "validationType": "AND"
}
```

* **AND**: All elements must match
* **OR**: Any element may match

---

## Complex Test Cases 🧠

Complex test cases allow **logic-driven validation pipelines** using **66 built-in commands**.

They enable:

* Arithmetic on values derived from program output
* Multi-step transformations
* Advanced, programmatic assertions

To define a complex test case, set:

```json
"type": 2
```

### Complete Complex Test Case Example ⭐

Below is a full, real-world example demonstrating execution, data extraction,
arithmetic operations, and a final assertion.

```json
{
  "type": 2,
  "name": "Output arithmetic validation",
  "description": "Validate output length using ASCII arithmetic on output characters",
  "commandArgs": ["greet"],
  "pipeline": [
    { "cmd": "run" },
    { "cmd": "extract_char", "source": "{{output}}", "index": 1, "store": "char1" },
    { "cmd": "extract_char", "source": "{{output}}", "index": 4, "store": "char2" },
    { "cmd": "to_int", "source": "{{char1}}", "store": "value1" },
    { "cmd": "to_int", "source": "{{char2}}", "store": "value2" },
    { "cmd": "add", "lhs": "{{value1}}", "rhs": "{{value2}}", "store": "sum" },
    { "cmd": "multiply", "lhs": "{{sum}}", "rhs": "2", "store": "result" },
    { "cmd": "subtract", "lhs": "{{result}}", "rhs": "5", "store": "result" },
    { "cmd": "length", "source": "{{output}}", "store": "output_length" },
    { "cmd": "assert_equals", "lhs": "{{output_length}}", "rhs": "{{result}}" }
  ]
}
```

and provide a `pipeline`.

---

## Structure of Complex Test Cases 🏗️

Each pipeline command may include:

* **cmd**: Command name
* **source**: Input value
* **lhs / rhs**: Left-hand and right-hand operands
* **index**: Character index (for extraction commands)
* **background**: Keep the process running for interactive tests
* **store**: Variable name to store the result

Variables support **string**, **integer**, and **float** types and are referenced using `{{variable}}` syntax.

The `run` command creates an `output` variable containing the combined stdout and stderr.

---

## Productivity & Development Speed 🚀

Using **Testify significantly accelerates development**:

* 🔁 Instant regression detection
* 🧩 No invasive test code in production
* ⚡ Faster feedback during development
* 🛡️ Safer refactoring and large changes
* 📈 Higher confidence in releases

Testify lets you move fast without breaking things.

---

## A Few More Features 🎯

* Per-test timeouts
* Per-test environment variables
* Designed for interactive and non-interactive CLI programs
  
