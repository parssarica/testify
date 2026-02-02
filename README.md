# Testify 🧪

![Language](https://img.shields.io/badge/language-C-blue)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)
![Status](https://img.shields.io/badge/status-active-success)

A generic, user-level software testing tool.

---

## Table of Contents
- [Overview](#overview)
- [Quick Start](#quick-start)
- [Installation](#installation)
  - [Fedora](#fedora)
  - [Debian-based distros](#debian-based-distros)
  - [Arch Linux](#arch-linux)
- [Features](#features)
- [JSON Structure](#json-structure)
- [Complex Test Cases](#complex-test-cases)
  - [Structure of Complex Test Cases](#structure-of-complex-test-cases)
- [Productivity & Development Speed](#productivity--development-speed)
- [A Few More Features](#a-few-more-features)

---

## Overview 📌
**Testify** focuses on **user-level testing**. Instead of relying on in-code assertions, Testify determines whether a test passes or fails by **analyzing the program’s output and exit behavior**.

This means:

- ✅ No need to modify your source code
- ✅ No test-specific assertions inside the program
- ✅ Black-box testing made simple and powerful

Testify is especially useful for CLI tools, scripts, compilers, interpreters, and system-level programs.

---

## Quick Start ⚡

```bash
git clone https://github.com/parssarica/testify.git
cd testify
sudo make install
```

Create `testcases.json`:

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

Run:
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
- JSON-based test cases
- Complex command pipelines
- Regex matching
- Colored output
- Execution summaries

---

## Productivity & Development Speed 🚀

Testify dramatically improves development speed by enabling fast, automated, black-box testing without touching production code.
