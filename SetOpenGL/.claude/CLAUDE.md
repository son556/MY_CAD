# SetOpenGL Project

## 프로젝트 개요
OpenGL 학습 프로젝트. GLEW + GLFW 기반, Visual Studio (MSVC) 빌드.

## 빌드 환경
- IDE: Visual Studio
- 플랫폼: Windows x64
- OpenGL: 4.3 Core Profile
- 라이브러리: GLEW, GLFW3, GLM

## 프로젝트 구조

```
SetOpenGL/
├── .claude/
│   └── CLAUDE.md               # 이 파일
├── main.cpp                    # 진입점, 렌더 루프
├── OpenGLInit.h / .cpp         # GLFW+GLEW 초기화 (static 클래스)
├── Shader.h / .cpp             # 셰이더 컴파일 클래스
├── Program.h / .cpp            # 셰이더 프로그램 링크 클래스
├── Mesh.h / .cpp               # VAO/VBO/EBO 관리 클래스
├── ReadSource.h                # GLSL 파일 읽기 (static 클래스, header-only)
├── VertexShader.glsl           # 버텍스 셰이더 (version 330 core)
├── FragmentShader.glsl         # 프래그먼트 셰이더 (고정 색상: 주황색)
├── include/
│   ├── GL/                     # GLEW 헤더
│   ├── GLFW/                   # GLFW 헤더
│   └── GLM/                    # GLM 헤더 (header-only 수학 라이브러리)
├── lib/
│   ├── glew32.lib
│   ├── glew32s.lib
│   ├── glfw3.lib
│   ├── glfw3_mt.lib
│   └── glfw3dll.lib
├── dll/
│   ├── glew32.dll
│   └── glfw3.dll
├── SetOpenGL.vcxproj
├── SetOpenGL.vcxproj.filters
└── SetOpenGL.vcxproj.user
```

## 주요 클래스 설명

### OpenGLInit (static 클래스)
- `Init(GLFWwindow*& window)`: GLFW 초기화, 윈도우 생성, GLEW 초기화
- 인스턴스화 불가 (생성자/소멸자 delete)

### Shader
- `Shader(filePath, GLenum shaderType)`: GLSL 파일 읽어서 컴파일
- 복사 불가, 이동 생성자 지원
- `GetShader()`: 핸들 반환

### Program
- `Program()`: `glCreateProgram()` 호출
- `AttachShader(handle)`, `Link()`: 셰이더 연결 및 링크
- 복사 불가, 이동 생성자 지원
- `GetProgram()`: 핸들 반환

### Mesh
- `Mesh(vertices, verticesByteSize, indices, indicesByteSize)`: VAO/VBO/EBO 생성 및 설정
- `GetVAO()`: VAO 핸들 반환

### ReadSource (static 클래스, header-only)
- `Read(src, srcPath)`: 파일을 문자열로 읽기
- 인스턴스화 불가

## 셰이더
- `VertexShader.glsl`: `layout(location=0) in vec3 aPos` → `gl_Position`
- `FragmentShader.glsl`: 고정 색상 `vec4(1.0, 0.5, 0.2, 1.0)` (주황색)