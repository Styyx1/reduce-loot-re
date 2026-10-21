# SKSE Plugin Template

A CommonLibSSE plugin template using [xmake](https://xmake.io) as the build system, with integration of [styyx-utils](https://github.com/Styyx1/StyyxUtils).

Supports **Skyrim 1.5.97**, **Skyrim 1.6.1130+** with separate DLLs.

## Features

- C++23
- Skyrim 1.5.97 support
- Skyrim 1.6+ support
- CommonlibSSE and StyyxUtil as submodules
- TOML support via CommonlibSSE
- GitHub action for building for both versions
- VS Code/clangd setup
- easy project setup via ``moddata.toml`` and the ``setupMod.py`` script

---

## Requirements

Requirements are for Windows cause on linux, there's a lot more you need to set up. You can not build a plugin with this template on linux!

| Tool                                                                          | Notes                                             |
| ----------------------------------------------------------------------------- | ------------------------------------------------- |
| [Visual Studio 2022](https://visualstudio.microsoft.com/)                     | With the**Desktop development with C++** workload |
| [xmake](https://xmake.io/#/guide/installation)                                | v2.8.2 or later                                   |
| [Git](https://git-scm.com/)                                                   | For general interaction with github               |
| [Python 3.11 or newer](https://www.python.org/downloads/release/python-3147/) | Required for ``setupMod.py``                      |

---

## Getting Started

### 1. Create your repository

Click **"Use this template"** at the top of this repository on GitHub, then clone your newly created repo:
```bash
git clone --recurse-submodules https://github.com/your-name/your-plugin-name
cd your-plugin-name
```

The ``--recurse-submodules`` is important, it pulls in CommonLibSSE and styyx-utils automatically.

**Cloning directly?** If you cloned without the flag, run:
```bash
git submodule update --init --recursive
```

### 2. Configure your plugin data

This template uses ``moddata.toml`` to configure your xmake.lua and the workflow file

The default looks like this:
```toml
name = "PLACEHOLDER"
license = "PLACEHOLDER"
workflowname = "PLACEHOLDER"
modversion = "1.0.0"


[configs]
#use-fui = true
```
Change the values to match what you need:
```toml
name = "MySKSEPlugin"
license = "GPL-3.0"
workflowname = "MySKSEPlugin"
modversion = "4.2.0"


[configs]
use-fui = true
rex_ini = true
whatever_setting = 69
```

### 3. Run the setup script

Run:
```bash
python setupMod.py
```
This will update your xmake.lua and workflow file. 

### 4. Build

#### Skyrim 1.6+

It is the default config and can be run like this:

```bash
xmake
```

The compiled binaries are placed in ``Distr/AE/SKSE/Plugins/``

#### Skyirm 1.5.97

Configure xmake for it:

```bash
xmake f --skyrim_ae=false
xmake
```

The compiled binaries are placed in ``Distr/SE/SKSE/Plugins/``

---

## xmake Config Options

Set options with ``set_config("setting_name", true/false)`` in the xmake.lua

| Option            | Default   | Description                                                |
| -                 |-          | -                                                          |
| ``skyrim_ae``     | ``true``  | Target Skyrim AE. Set to``false`` for SE                   |
| ``use-hook-utils``| ``false`` | Enable hooking utilities from styyx-utils (pulls in xbyak) |
| ``use-fui``       | ``false`` | Enable FLICK menu support                                  |
| ``skse_xbyak``    | ``false`` | Enable xbyak support in CommonLibSSE directly              |
| ``rex_toml``      | ``false`` | Enable TOML config file support via CommonLibSSE           |
| ``rex_json``      | ``false`` | Enable JSON config file support via CommonLibSSE           |
| ``rex_ini``       | ``false`` | Enable INI config file support via CommonLibSSE            |

### Example: Enable hook utilities

```bash
xmake f --skyrim_ae=true --use-hook-utils=true
xmake
```

or in xmake.lua

```lua
set_config("use-hook-utils", true)
```

---

## Build both game versions

To build both SE and AE versions in one step:

```bash
xmake shiprelease
```

This will:

1. Build the AE version → `Distr/AE/SKSE/Plugins/`
2. Build the SE version → `Distr/SE/SKSE/Plugins/`
3. Restore the default AE configuration

---

## Runtime differences

Use `SKSE_TEMPLATE_SKYRIM_SE`, `SKSE_TEMPLATE_SKYRIM_AE` when your plugin needs runtime-specific code paths.

For relocation values, the template provides:

```cpp
REL_ID(se, ae)
REL_ID_3(se, ae, vr)
OFFSET(se, ae)
OFFSET_3(se, ae, vr)
```

---

## styyx-utils

This template includes [styyx-utils](https://github.com/Styyx1/StyyxUtils) as a submodule, a header-only utility library for CommonLibSSE plugin development. It provides helpers for actors, cells, forms, magic, menus, timers, and more.

To use it, simply include the main header in your precompiled header:

mind, using the main header does not pull in ``st-fui.h`` which could be used for FLICK menu integration
enable it with ``set_config("use-fuck", true)``

```cpp
#include <styyx-utils.h>
```

If you enabled `use-hook-utils`, hooking utilities are available via:

```cpp
#include <st-hooks.h>
```

---

## License

[MIT](LICENSE)

## Contributions

PRs are welcome
