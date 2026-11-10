# AGENTS.md — base/src/rust/（Rust 实现，CXX 桥接）

动手前先读根目录 `AGENTS.md`；本目录经 `rust_cxx` 桥接为 C++ 提供实现，编译产物为 `utils_rust`。

## 目录职责

| 文件 | 桥接的 C++ 侧 | 知识路由 |
|---|---|---|
| ashmem.rs | ashmem.cpp | `docs/knowledge/file_io.md` |
| file_ex.rs | file_ex.cpp | `docs/knowledge/file_io.md` |
| directory_ex.rs | directory_ex.cpp | `docs/knowledge/file_io.md` |
| lib.rs | cxx 桥接声明（对 C++ 暴露的函数/类型） | — |

## 硬规则

- **接口同步**：CXX 桥接的函数签名是 C++/Rust 双侧契约——改任何一侧签名必须同步另一侧，改 lib.rs 的桥接声明需检查全部调用点；
- 跨界数据用 cxx 提供的映射类型（CxxString/CxxVector 等），不做裸指针/生命周期越界传递；
