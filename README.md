# Package Installation Management System

## Overview

The Package Installation Management System is a C++ command-line application that simulates software component installation and uninstallation in a deployment environment.

The system models software modules and packages as a hierarchical dependency structure and provides functionality for:

- Installing and uninstalling individual components
- Installing and uninstalling all components at once
- Managing nested package structures with recursive dependencies
- Tracking component states (PENDING, INSTALLED, FAILED)
- Simulating installation failures with mock-fail injection
- Automatic transactional rollback on failed installations
- Reference-counted dependency tracking for safe uninstallation
- Observer-based real-time logging of all state transitions

The project demonstrates object-oriented design principles including inheritance, polymorphism, composition, encapsulation, the Composite pattern, the Observer pattern, and transaction-based error recovery.

---

## Purpose

Modern software systems are often composed of multiple interconnected components that must be installed in a specific order. A failure in one component can affect the entire deployment process.

This project provides a simplified environment for modeling and managing such installation workflows. It allows users to:

- Define software modules and packages
- Build dependency hierarchies via attachment
- Simulate successful and failed deployments
- Observe component state changes in real time
- Verify rollback and recovery behavior

The system acts as a lightweight deployment and dependency management simulator.

---

## Core Concepts

### Module

A Module is the smallest installable unit in the system. It cannot contain other components.

Examples:

- Check OS Version
- Download Core Binary
- Configure Firewall Rules

### Package

A Package is a collection of modules and/or other packages. This allows complex installation trees to be represented.

Example:

```text
Full App Deployment
│
├── Base System Setup
│   ├── Check OS Version
│   ├── Install Core Engine
│   │   ├── Download Core Binary
│   │   └── Extract Core Binary
│   └── Configure Firewall Rules
│
└── Create Admin User
```

### Component States

Each component can be in one of the following states:

| State     | Description                         |
| --------- | ----------------------------------- |
| PENDING   | Component has not been installed    |
| INSTALLED | Installation completed successfully |
| FAILED    | Installation or uninstallation failed |

State transitions are automatically reported through the observer system.

### Observer System

The project implements the Observer design pattern. A central `SystemLogger` observes all components and prints state transitions whenever a component changes state.

Example output:

```text
[OBSERVER] Component M1 changed from PENDING to INSTALLED
```

This provides a complete trace of installation and rollback operations.

### Dependency Tracking

Each component maintains:

- **`installedParentsCount`** — the number of currently installed parent packages that depend on this component.
- **`explicitlyInstalled`** — whether the component was installed directly by the user (via `INSTALL <id>`) rather than as a dependency of a package.

When a package is installed, each child's parent count is incremented. When a package is uninstalled, each child's parent count is decremented. A child is only recursively uninstalled if its parent count reaches zero **and** it was not explicitly installed by the user.

### Transactional Rollback

Installation of a package uses a `TransactionContext` to record all state changes and parent-count increments. If any child fails during installation (e.g. due to a mock failure), the transaction is rolled back:

1. All state changes are reversed in reverse order (via `forcePending()`).
2. All parent-count increments are decremented.
3. The package is set to FAILED.

Each package creates its own scoped `TransactionContext`, so a failure in a nested package only rolls back that subtree — not the parent's other children.

---

## Supported Commands

All commands are read from standard input, one per line. The system processes commands sequentially and writes output to standard output. Empty lines and lines containing only `_` are skipped.

### ADD MODULE

```text
ADD MODULE <id> <title with spaces>
```

Creates a new module with the given ID and title. The title can contain spaces and extends to the end of the line.

Example:

```text
ADD MODULE M1 Check OS Version
```

Error: `ERROR: Component with ID <id> already exists` if the ID is already taken.

### ADD PACKAGE

```text
ADD PACKAGE <id> <title with spaces>
```

Creates a new package with the given ID and title.

Example:

```text
ADD PACKAGE P1 Install Core Engine
```

Error: `ERROR: Component with ID <id> already exists` if the ID is already taken.

### ATTACH

```text
ATTACH <parentId> <childId>
```

Attaches a child component (module or package) to a parent package. Children are installed in the order they are attached.

Errors:

- `ERROR: Component <id> does not exist` if either ID is not found.
- `ERROR: Cannot attach to a module` if the parent is a module.
- `ERROR: Cannot attach to an already installed package` if the parent is already INSTALLED.
- `ERROR: Component <childId> is already attached to <parentId>` if the child is already attached.

### INSTALL

```text
INSTALL <id>
```

Installs a single component. Sets the component as explicitly installed.

- For a **module**: if the mock-fail flag is set, the module transitions to FAILED. Otherwise it transitions to INSTALLED.
- For a **package**: recursively installs all children in attachment order. If any child fails, the entire installation is rolled back and the package transitions to FAILED.

Errors:

- `ERROR: Component <id> does not exist` if the ID is not found.
- `ERROR: Component <id> is already installed` if the component is already INSTALLED.

### INSTALL -A

```text
INSTALL -A
```

Iterates through all components in creation order and installs each one that is not already INSTALLED. Each component is marked as explicitly installed.

### UNINSTALL

```text
UNINSTALL <id>
```

Uninstalls a single INSTALLED component. The component transitions to PENDING.

For packages, uninstallation is reference-counted: each child's parent count is decremented. A child is recursively uninstalled only if:

1. Its parent count reaches zero after decrement, **and**
2. It was not explicitly installed by the user.

Children are processed in reverse attachment order.

Errors:

- `ERROR: Component <id> does not exist` if the ID is not found.
- `ERROR: Component <id> is not currently installed` if the component is not INSTALLED (i.e. it is PENDING or FAILED).
- `ERROR: Component <id> is required by another package` if an installed package still depends on this component.

### UNINSTALL -A

```text
UNINSTALL -A
```

Performs a complete system cleanup. Traverses all components in reverse creation order. For every component whose status is INSTALLED or FAILED:

- Resets its status to PENDING.
- Clears all dependency counters (`installedParentsCount` = 0).
- Resets all installation-related flags (`explicitlyInstalled` = false, `mockFail` = false).

After completion, the system returns to its initial state as if no installation operations had been performed.

Error: `ERROR: No installed components to uninstall` if all components are already PENDING.

### MOCK_FAIL

```text
MOCK_FAIL <id>
```

Marks a component to fail during its next installation attempt. When the component is installed, it will transition to FAILED instead of INSTALLED.

Errors:

- `ERROR: Component <id> does not exist` if the ID is not found.
- `ERROR: Component <id> is already installed` if the component is currently INSTALLED.
- `ERROR: Component <id> is already set to fail` if the mock-fail flag is already set.

### RESOLVE_FAIL

```text
RESOLVE_FAIL <id>
```

Clears the mock-fail flag on a component, allowing it to be installed successfully again.

Errors:

- `ERROR: Component <id> does not exist` if the ID is not found.
- `ERROR: Component <id> is not in a mock fail state` if the mock-fail flag is not set.

### CMD

```text
CMD <id>
```

Prints the current state of a component in the format:

```text
<id> (<type>): <title> [<state>]
```

Where `<type>` is `MODULE` or `PACKAGE`, and `<state>` is `PENDING`, `INSTALLED`, or `FAILED`.

Error: `ERROR: Component <id> does not exist` if the ID is not found.

### END / EN

```text
END
```

Terminates command processing and exits the program. `EN` is also accepted.

### Invalid Commands

Any unrecognized command produces:

```text
ERROR: Invalid command
```

This includes `ADD` with an invalid type (anything other than `MODULE` or `PACKAGE`).

---

## Example Workflow

### Create Components

```text
ADD MODULE M1 Check OS Version
ADD MODULE M2 Download Core Binary
ADD PACKAGE P1 Install Core Engine
```

### Build Package Structure

```text
ATTACH P1 M1
ATTACH P1 M2
```

### Install Package

```text
INSTALL P1
```

Expected output (children install before the parent):

```text
[OBSERVER] Component M1 changed from PENDING to INSTALLED
[OBSERVER] Component M2 changed from PENDING to INSTALLED
[OBSERVER] Component P1 changed from PENDING to INSTALLED
```

### Failure Simulation and Rollback

```text
ADD MODULE M3 Failing Module
ADD PACKAGE P2 Test Package
ATTACH P2 M3
MOCK_FAIL M3
INSTALL P2
```

Expected output (M3 fails, P2 rolls back):

```text
[OBSERVER] Component M3 changed from PENDING to FAILED
[OBSERVER] Component P2 changed from PENDING to FAILED
```

### Resolve and Retry

```text
RESOLVE_FAIL M3
INSTALL P2
```

Expected output:

```text
[OBSERVER] Component M3 changed from PENDING to INSTALLED
[OBSERVER] Component P2 changed from PENDING to INSTALLED
```

---

## Building the Project

### Requirements

- C++17 compatible compiler (GCC, Clang, or equivalent)
- GNU Make

### Compile

```bash
make
```

This produces the `installer` binary in the project root.

### Run

```bash
./installer
```

Or via Make:

```bash
make run
```

The program reads commands from standard input. You can pipe a file:

```bash
./installer < in/in1.txt
```

### Clean Build Artifacts

```bash
make clean
```

---

## Testing

The project includes a local judge script and 100 test cases for automated verification.

### Test Structure

- `in/in1.txt` through `in/in100.txt` — input files containing command sequences.
- `out/out1.txt` through `out/out100.txt` — expected output files.

### Running Tests

```bash
bash localJudge.sh
```

The script will:

1. Compile the project using `make`.
2. Run the binary against each of the 100 input files.
3. Compare the actual output against the expected output (after normalizing line endings and trimming blank lines).
4. Report per-test verdicts (Accepted, Wrong Answer, Time Limit Exceeded, Runtime Error, Missing Files).
5. Print a summary with pass rate and a visual progress bar.

### Test Configuration

The judge script (`localJudge.sh`) supports the following configuration at the top of the file:

| Variable     | Default   | Description                        |
| ------------ | --------- | ---------------------------------- |
| `BINARY`     | `./installer` | Path to the compiled binary    |
| `IN_DIR`     | `./in`    | Directory containing input files   |
| `OUT_DIR`    | `./out`   | Directory containing expected outputs |
| `TIME_LIMIT` | `3`       | Time limit per test in seconds     |
| `NUM_TESTS`  | `100`     | Number of test cases to run        |

---

## Project Structure

```text
.
├── include/
│   ├── ComponentState.hpp       # ComponentState enum (PENDING, INSTALLED, FAILED)
│   ├── Installable.hpp          # Abstract base class for all components
│   ├── InstallationEngine.hpp   # Central engine managing all components
│   ├── Module.hpp               # Concrete leaf component
│   ├── Observer.hpp             # Observer interface for state changes
│   ├── Package.hpp              # Composite component containing children
│   ├── SystemLogger.hpp         # Concrete observer that logs to stdout
│   └── TransactionContext.hpp   # Tracks state changes and count increments for rollback
│
├── src/
│   ├── ComponentState.cpp       # stateToString() implementation
│   ├── Installable.cpp          # Base class implementation (state, flags, observer notification)
│   ├── InstallationEngine.cpp   # Engine implementation (add, attach, install, uninstall, etc.)
│   ├── Module.cpp               # Module install/uninstall implementation
│   ├── Package.cpp              # Package install (with rollback) and uninstall (with ref-counting)
│   ├── SystemLogger.cpp         # Observer callback that prints state transitions
│   └── main.cpp                 # Entry point and command parser
│
├── in/                          # 100 test input files (in1.txt ... in100.txt)
├── out/                         # 100 expected output files (out1.txt ... out100.txt)
├── localJudge.sh                # Automated test runner script
├── Makefile                     # Build configuration
├── installer                    # Compiled binary (after make)
└── README.md                    # This file
```

---

## Architecture

### Class Hierarchy

```text
Installable (abstract)
├── Module      (leaf — no children)
└── Package     (composite — contains children)

Observer (abstract)
└── SystemLogger (logs state transitions to stdout)
```

### Key Classes

#### `Installable` (abstract base)

The base class for all components. Manages:

- Identity (`id`, `title`)
- State (`ComponentState`)
- Mock-fail flag
- Parent reference count
- Explicit-install flag
- Observer list
- Pure virtual `install()` and `uninstall()` methods

#### `Module`

A leaf installable component. On install, transitions directly to INSTALLED (or FAILED if mock-fail is set). On uninstall, transitions to PENDING.

#### `Package`

A composite installable component. Maintains a list of children.

- **`install()`**: Creates a scoped `TransactionContext`, installs all children in order, increments each child's parent count on success. On failure, rolls back all state changes and parent-count increments within the local transaction, then transitions to FAILED.
- **`uninstall()`**: Transitions to PENDING, then processes children in reverse order. Decrements each child's parent count. Recursively uninstalls a child only if its parent count is zero and it was not explicitly installed.

#### `InstallationEngine`

The central orchestrator. Manages the global list of all components and provides the command API:

- `addModule()` / `addPackage()` — create components
- `attach()` — build dependency structure
- `install()` / `uninstall()` — single-component lifecycle
- `installAll()` / `uninstallAll()` — bulk operations
- `mockFail()` / `resolve()` — failure simulation
- `printComponent()` — debug utility

#### `TransactionContext`

A lightweight struct that accumulates two lists during an installation transaction:

- `stateChangedNodes` — components whose state was changed (for rollback via `forcePending()`)
- `countIncreasedNodes` — components whose parent count was incremented (for rollback via `decrementParents()`)

#### `SystemLogger`

A concrete `Observer` that prints formatted state transitions to standard output.

---

## Design Patterns

### Composite Pattern

`Module` and `Package` share the common `Installable` interface. Packages contain a vector of `Installable*` children, allowing recursive nesting of packages within packages.

### Observer Pattern

Components maintain a list of `Observer*` pointers. When `setState()` is called, all registered observers are notified via `onStateChanged()`. The `SystemLogger` is registered as an observer on every component at creation time.

### Transaction / Memento Pattern

Each package installation creates a `TransactionContext` that records all side effects. If installation fails, the transaction is rolled back by reversing state changes and decrementing parent counts. Each package scopes its own transaction, so nested failures are isolated.

### Template Method (via Polymorphism)

`Installable::install()` and `Installable::uninstall()` are pure virtual functions. `Module` and `Package` provide their own implementations, and the engine calls them polymorphically.

---

## Error Handling

The system validates all commands and produces descriptive error messages for invalid operations. Error messages follow the format:

```text
ERROR: <description>
```

Common errors include:

| Error Message | Cause |
| --- | --- |
| `ERROR: Invalid command` | Unrecognized command or invalid ADD type |
| `ERROR: Component with ID <id> already exists` | Duplicate ID in ADD |
| `ERROR: Component <id> does not exist` | Reference to nonexistent component |
| `ERROR: Cannot attach to a module` | ATTACH target is a module, not a package |
| `ERROR: Cannot attach to an already installed package` | ATTACH to an INSTALLED package |
| `ERROR: Component <id> is already attached to <pkg>` | Duplicate attachment |
| `ERROR: Component <id> is already installed` | INSTALL or MOCK_FAIL on INSTALLED component |
| `ERROR: Component <id> is not currently installed` | UNINSTALL on non-INSTALLED component |
| `ERROR: Component <id> is required by another package` | UNINSTALL blocked by dependent package |
| `ERROR: Component <id> is already set to fail` | MOCK_FAIL on component already flagged |
| `ERROR: Component <id> is not in a mock fail state` | RESOLVE_FAIL on component without flag |
| `ERROR: No installed components to uninstall` | UNINSTALL -A with nothing active |

---

## License

This project is provided for educational and demonstration purposes.
