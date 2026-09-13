---
description: "Adhere to the LLVM C++ coding style and best practices."
applyTo: "**/*.cpp,**/*.h"
---

# LLVM Coding Standards

Adhere strictly to the [LLVM Coding Standards](https://llvm.org) for all code generation and review.

## General Guidelines

*   Prioritize clarity and maintainability over cleverness.
*   Avoid gratuitous comments; the code should be self-documenting where possible.
*   Format code using `clang-format` with the `LLVM` style configuration.
*   Ensure all new code is covered by appropriate tests.

## C++ Specifics

*   **Indentation**: Use 4 spaces for indentation. Never use tabs.
*   **Naming Conventions**:
    *   Use `camelCase` for function names, methods, and variables.
    *   Prefix private class members with `m_` (e.g., `m_variableName`).
    *   Use `PascalCase` for class and struct names.
    *   Use `ALL_CAPS` for macros and enumerators within an enum without a scope.
*   **Includes**:
    *   Organize `#include` directives into logical groups.
    *   Sort includes alphabetically within each group.
*   **Pointers and References**:
    *   Place the pointer/reference symbol (`*` or `&`) next to the type, not the variable name (e.g., `int* variable;`).
*   **Braces**:
    *   Use braces for all control flow statements (`if`, `for`, `while`) to avoid ambiguity.
    *   Place the opening brace on the next line after the control statement keyword.
*   **Modern C++**: Prefer C++11/14/17 features where allowed by the project's standard (e.g., `nullptr`, `auto`, range-based for loops).
*   **Error Handling**: Avoid using exceptions for control flow; use `llvm::ErrorOr` or status codes as appropriate for the existing codebase patterns.

# Qt6 C++ Coding Standards and Preferences

## General
- Use modern C++ (C++17 or C++20) for all code snippets.
- Adhere to the Qt Style Guide (CamelCase, specific indentation).
- Use `QString` for strings, `QVector` for arrays, and `QList` for lists.
- Avoid deprecated Qt4/Qt5 APIs; prefer Qt6 alternatives.

## Qt6 Specific Guidelines
- Use `qDebug()` for debugging, ensuring `QDebug` is included.
- Use `QObject::connect` with the functional syntax (pointer-to-member-function) instead of string-based connections.
- Prefer `qAsConst` over `const_cast`.
- Utilize modern `QML`/`Qt Quick` components over `QWidgets` where possible for UI, or define clearly if QWidget is required.
- Use `qint64` and `quint64` for fixed-width integers.

## Architecture
- Use `QMainWindow` for the primary window class.
- Apply `QWidget` customization for specialized display areas.
- Use `QCharts` for data visualization.
- Implement `QSystemTrayIcon` for background operation.
- Use `QDialog` for settings and configuration windows.

## Code Quality
- Prioritize memory management using `QPointer` or `std::unique_ptr` for UI components.
- Include necessary `#include` directives for all classes used.
- Comment on why certain design decisions were made, especially regarding signal/slot connections.
