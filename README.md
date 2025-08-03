# 📦 WinSFXMaker
 SelF eXtract Maker for Windows.<br/>
 ✅ Tested on Windows 10 / Visual Studio 2013 (Minimum build WindowsXP)
<br/><br/>

## 📖 Overview
`WinSFXMaker` is a very super simple install-sheild GUI tool for creating **self-extracting (SFX) executables** on Windows. It allows users to bundle ZIP archives into standalone `.exe` files that automatically extract and optionally run an installer or other program.
<br/><br/>

## 🛠️ Stacks
![C++14](https://img.shields.io/badge/C%2B%2B14-00599C?logo=c%2B%2B&logoColor=white) ![log4cpp](https://img.shields.io/badge/log4cpp-logger-blue?logo=log4cpp&logoColor=white) ![minizip](https://img.shields.io/badge/minizip-zip-green?logo=minizip&logoColor=white)
![MFC](https://img.shields.io/badge/MFC-GUI--framework-0078D7)<br/><br/>

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
<img src="https://img.shields.io/badge/-C++-000000?logo=c%2B%2B" /> <img src="https://img.shields.io/badge/Spring-6DB33F?logo=Spring&logoColor=white"/> <img src="https://img.shields.io/badge/MariaDB-003545?logo=mariaDB&logoColor=white"/><br/><br/>
<br/><br/>

## ⚙️ Dev Environment
- **Platform**: Windows (32-bit / 64-bit)
- **Language**: C+ +ISO 14 with MFC and GDI+
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

## 📌 Notes
- Ensure your post-extraction executable exists inside the ZIP archive
- Some commands may require elevated privileges (UAC prompt)
- You can customize the icon and stub using the `res/` folder
<br/><br/>

## 💻 [Preview]
<img width="576" height="581" alt="image" src="https://github.com/user-attachments/assets/8ae6901e-29f8-42b7-92a7-be63d0f192b7" />
<img width="578" height="608" alt="image" src="https://github.com/user-attachments/assets/09f1f970-4d3a-4848-9791-259acaf084cd" />
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
