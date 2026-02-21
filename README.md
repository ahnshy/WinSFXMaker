# 📦 WinSFXMaker
 SelF eXtract Maker for Windows.<br/>
 ✅ Tested on Windows 10 / Visual Studio 2013 (Minimum build WindowsXP)
<br/><br/>

## 📖 Overview
`WinSFXMaker` is a very super simple install-sheild GUI tool for creating **self-extracting (SFX) executables** on Windows. It allows users to bundle ZIP archives into standalone `.exe` files that automatically extract and optionally run an installer or other program.
<br/><br/>

## 🔑 Key Features
- ✅ Easy-to-use SFX archive generator
- ✅ Custom post-extraction command support
- ✅ Lightweight and dependency-free portable exe binary.
- ✅ Intuitive GUI with file pickers
- ✅ ZIP file bundling into a Windows EXE
- ✅ Custom post-extraction execution (e.g., launch installer.exe)
- ✅ Logging support via log4cpp
- ✅ Resource management using `SFXTemplate` and `res/`
<br/><br/>

## 🛠️ Stacks
![C++14](https://img.shields.io/badge/C%2B%2B14-00599C?logo=c%2B%2B&logoColor=white) ![log4cpp](https://img.shields.io/badge/log4cpp-logger-blue?logo=log4cpp&logoColor=white) ![minizip](https://img.shields.io/badge/minizip-zip-green?logo=minizip&logoColor=white)
![MFC](https://img.shields.io/badge/MFC-GUI--framework-0078D7)<br/><br/>

## ⚙️ Dev Environment
- **Platform**: Windows (32-bit / 64-bit)
- **Language**: C+ +ISO 14 with MFC and GDI+
<br/><br/>


## 🧩 Build Requirements
- Visual Studio 2013 (**v120 toolset**)
- MFC (Microsoft Foundation Classes)
- Windows SDK
<br/><br/>


## 📁 Source Tree
```
/
├── Dialog/                # Dialog UI logic
├── Helper/                # Utility functions
├── SFXTemplate/           # Template EXE and metadata
├── UI/                    # Icons and GUI resources
├── ZipArchive/            # ZIP compression/decompression (minizip-based)
├── log4cpp/               # Integrated log4cpp source
├── res/                   # Resource files (icons, templates, etc.)
├── WinSFXMaker.*          # Core application logic
└── README.md              # Project documentation
```

## 📤 How to build (Visual Studio 2013 on Windows 10)
1. Clone the repository:
   ```bash
   git clone https://github.com/ahnshy/WinSFXMaker.git
   cd WinSFXMaker
   ```

2. Open the solution:
   - Launch `WinSFXMaker.sln` in Visual Studio 2013 or later

3. Choose build config:
   - Select `Release` or `Debug` mode
   - Choose x64 or Win32 platform

4. Build:
   - Build `ALL_BUILD` or the `WinSFXMaker` target
<br/><br/>

## 🚀 How to Use
1. Launch `WinSFXMaker.exe`
2. Select the input ZIP file (e.g., `setup.zip`)
3. Specify output EXE file name (e.g., `installer.exe`)
4. Enter post-extraction command (e.g., `setup.exe`)
5. Click "Create SFX" to generate a standalone installer
<br/><br/>


## 🧰 CLI Mode
You can generate SFX files **without the GUI** by using command-line arguments.

### ✅ Basic Usage
```bash
WinSFXMaker.exe [options]
```

### 🧩 Options

| Option | Description |
|------|------|
| `-p, --path <path>` | Folder path to compress **(required)** |
| `-o, --output <file>` | Output SFX file path (auto-generated if omitted) |
| `-i, --icon <file>` | Icon file (`.ico`) |
| `-r, --run <file>` | File to run after extraction (relative or absolute path) |
| `-v, --version <version>` | Version string (e.g., `"1.0.0.0"`) |
| `-n, --name <name>` | Product name |
| `-c, --copyright <text>` | Copyright info |
| `-d, --description <text>` | File description |
| `-s, --silent` | Run with no console output (Silent mode) |
| `-h, --help` | Show help |

### 🧪 Examples

#### 1) Basic
```bash
WinSFXMaker.exe -p "C:\MyApp"
```
Compresses `C:\MyApp` and creates `C:\MyApp\MyApp.exe`

#### 2) Specify output path
```bash
WinSFXMaker.exe -p "C:\MyApp" -o "C:\Output\Setup.exe"
```

#### 3) Run a file after extraction
```bash
WinSFXMaker.exe -p "C:\MyApp" -r "MyApp.exe"
```
Automatically runs `MyApp.exe` after extracting

#### 4) Run a file in a subfolder
```bash
WinSFXMaker.exe -p "C:\MyApp" -r "bin\Release\MyApp.exe"
```
Automatically runs `bin\Release\MyApp.exe` after extracting

#### 5) Absolute-path run target
```bash
WinSFXMaker.exe -p "C:\MyApp" -r "C:\MyApp\bin\Release\MyApp.exe"
```
The run target is automatically stored as a relative path (`bin\Release\MyApp.exe`) when possible

#### 6) Include icon & version metadata
```bash
WinSFXMaker.exe -p "C:\MyApp" -o "C:\Output\Setup.exe" -i "app.ico" -r "Setup.exe" -v "1.0.0.0" -n "My Application" -c "Copyright 2024" -d "My App Installer"
```

#### 7) Silent mode (for automated builds)
```bash
WinSFXMaker.exe -p "C:\MyApp" -r "Setup.exe" -s
```
Runs without console output

#### 8) Show help
```bash
WinSFXMaker.exe --help
```

### 🖥️ Sample CLI Output
```
WinSFXMaker - CLI Mode
======================
Input Path:  C:\MyApp
Output File: C:\MyApp\MyApp.exe
Icon:        C:\icons\app.ico
Run After:   Setup.exe

Compressing files...
Preparing template...
Applying icon...
Applying version info...
Creating SFX file...

SFX file created successfully!
Output: C:\MyApp\MyApp.exe
```
<br/><br/>


## 🧱 SFX File Layout
Internal structure of the generated SFX file:

```
[EXE Template] + [ZIP Data] + [ZIP Size (8 bytes)] + [Run-File Info (optional)]
```

If run-file info exists:
```
[EXE Template] + [ZIP Data] + [ZIP Size (8 bytes)] + [RUNA Marker (4 bytes)] + [Path Length (4 bytes)] + [Path String] + [END! Marker (4 bytes)]
```
<br/><br/>


## 🧾 Log Files
When the program runs, it creates a log file **next to the executable**.

- `WinSFXMaker.exe` → `WinSFXMaker.log`
- `MySetup.exe` (generated SFX) → `MySetup.log`

### 📄 Log Format
```
[2024-12-25 12:34:56.789] [INFO] === WinSFXMaker Started ===
[2024-12-25 12:34:57.123] [INFO] Selected input path: C:\MyApp
[2024-12-25 12:34:58.456] [INFO] Create SFX button clicked
[2024-12-25 12:34:59.789] [INFO] SFX file created successfully: C:\Output\Setup.exe
```
<br/><br/>


## 📌 Notes
- Ensure your post-extraction executable exists inside the ZIP archive
- Some commands may require elevated privileges (UAC prompt)
- You can customize the icon and stub using the `res/` folder
<br/><br/>

## 💻 [Preview]
<img width="578" height="591" alt="image" src="https://github.com/user-attachments/assets/f2212d41-db01-41e9-ba9c-7e562aff1774" />
<img width="582" height="588" alt="image" src="https://github.com/user-attachments/assets/5121f400-59f1-43b8-8ea9-6136c2544501" />
<img width="575" height="589" alt="image" src="https://github.com/user-attachments/assets/83e05f45-932e-4fff-a5bc-ef2b8449b9b9" />
<br/><br/>

## 🔍 Acknowledgements
- Product Design and Developed by `ahnshy`

- Built with:
  - Microsoft Win32 / MFC & GDI+
  - log4cpp for logging
  - minizip for archive management

- Inspired by traditional SFX packaging workflows
<br/><br/>

## 📝 License
 This project includes components like `log4cpp` and `minizip`, which are licensed under their respective open-source licenses. <br/>
 Please verify usage rights before redistribution.<br/>
<br/>
 `log4cpp` LGPL 2.1 (Lesser General Public License v2.1)<br/>
 `minizip` zlib/libpng License
<br/><br/>
