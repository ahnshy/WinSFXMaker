# log4cpp Static Library Integration Guide

## 개요
log4cpp를 static library로 빌드하여 WinSFXMaker와 SFXTemplete에서 로깅 기능을 사용합니다.

## 1. log4cppLIB 빌드 설정

### Visual Studio 프로젝트 설정 변경사항:

1. **Runtime Library를 Static으로 변경**
   - Debug: `Multi-threaded Debug (/MTd)`
   - Release: `Multi-threaded (/MT)`

2. **Character Set**: Unicode

### 빌드 순서:
1. `log4cpp\log4cppLIB\log4cppLIB.vcxproj` 열기
2. Configuration을 Debug 또는 Release로 선택
3. 빌드 실행
4. 출력 위치:
   - Debug: `log4cpp\log4cppLIB\Debug\log4cppD.lib`
   - Release: `log4cpp\log4cppLIB\Release\log4cppLIB.lib`

## 2. WinSFXMaker 프로젝트 설정

### Additional Include Directories 추가:
```
$(SolutionDir)log4cpp\include
```

### Additional Library Directories 추가:
```
$(SolutionDir)log4cpp\log4cppLIB\$(Configuration)
```

### Additional Dependencies 추가:
- Debug: `log4cppD.lib`
- Release: `log4cppLIB.lib`

### Runtime Library 설정 (static 링크를 위해):
- Debug: `Multi-threaded Debug (/MTd)`
- Release: `Multi-threaded (/MT)`

## 3. SFXTemplete 프로젝트 설정

WinSFXMaker와 동일한 설정 적용

## 4. 사용법

### 헤더 포함:
```cpp
#include "Helper/Logger.h"
```

### 초기화 (프로그램 시작 시):
```cpp
LOG_INIT(NULL);  // 실행 파일과 같은 경로에 WinSFXMaker.log 생성
// 또는
LOG_INIT(_T("C:\\custom\\path\\mylog.log"));  // 사용자 정의 경로
```

### 로그 작성:
```cpp
LOG_INFO(_T("Application started"));
LOG_DEBUG(_T("Debug value: %d"), value);
LOG_WARN(_T("Warning: %s"), message);
LOG_ERROR(_T("Error occurred: %d"), errorCode);
```

### 종료 (프로그램 종료 시, 선택사항):
```cpp
LOG_SHUTDOWN();
```

## 5. 로그 파일 형식

```
[2024-12-25 12:34:56.789] [INFO] Application started
[2024-12-25 12:34:56.790] [DEBUG] Debug value: 123
[2024-12-25 12:34:56.791] [WARN] Warning: Low memory
[2024-12-25 12:34:56.792] [ERROR] Error occurred: 5
```

## 6. 주의사항

- log4cppLIB와 메인 프로젝트의 Runtime Library 설정이 일치해야 합니다.
- Unicode/MultiByte 설정도 일치해야 합니다.
- Append 모드로 동작하므로 기존 로그가 유지됩니다.
