使用agent修改代码，新增代码结构/功能特性必须同步更新本AGENTS

# AGENTS.md — base/src/（C++ 实现）

动手前先读根目录 `AGENTS.md`（约束边界、知识路由、验证闭环），再按本文件执行。

## 构建关系

- `sources_utils`（21 个 .cpp，见 base/BUILD.gn）同时编入 **utilsbase（静态库）** 与 **utils（动态库）**——改任何源文件即同时影响两个库；
- `utils` 额外链接 Rust CXX 桥接产物（见 `rust/AGENTS.md`）；
- 依赖方向：utils → utilsbase，**不得反向**；
- 平台条件编译：`ashmem.cpp` 仅 OHOS 产物编入；win/mac/ios 只有子集（parcel/refbase/string 等）——改通用代码时勿破坏子集编译。

## 知识路由

| 域 | 文件 | 先读 |
|---|---|---|
| 引用计数 | refbase.cpp | `docs/knowledge/memory.md` |
| 序列化 | parcel.cpp | `docs/knowledge/serialization.md` |
| 共享内存 | ashmem.cpp（经 CXX 桥接 rust/ashmem.rs） | `docs/knowledge/file_io.md` |
| 并发原语 | thread_ex.cpp / thread_pool.cpp / rwlock.cpp / semaphore_ex.cpp | `docs/knowledge/threading.md` |
| 事件/IO/定时 | event_handler.cpp / event_reactor.cpp / event_demultiplexer.cpp / io_event_*.cpp / timer.cpp / timer_event_handler.cpp | `docs/knowledge/event_handling.md` |
| 文件/目录 | file_ex.cpp / directory_ex.cpp / mapped_file.cpp | `docs/knowledge/file_io.md` |
| 字符串/时间 | string_ex.cpp / unicode_ex.cpp / datetime_ex.cpp | — |
| 观察者 | observer.cpp | `docs/knowledge/event_handling.md` |

## 内部头文件

`event_demultiplexer.h / event_handler.h / event_reactor.h / io_event_epoll.h / timer_event_handler.h / unicode_ex.h / utils_log.h` 为**实现内部头**：仅 base/src 内部使用，不得被外部或测试引用，不得声明到 bundle.json。

## 硬规则

- 日志（utils_log.h 的 `UTILS_LOGx`）、错误码、诊断信息**不得为通过测试而删除**；
- 并发修复优先复用同文件/同类已有的同步模式（对齐兄弟实现），不引入第二套原语；
- ashmem 相关接口是 C++/Rust 双侧契约，改动需同步 `rust/`。

## 修改后验证

改单 .cpp：构建所属模块 + 运行对应单测（`base/test/unittest/common` 中同名 Utils*Test）。命令见根 AGENTS.md“验证闭环”。
