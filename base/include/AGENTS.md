# AGENTS.md — base/include/（对外 API 头文件）

本目录是 c_utils 的**兼容性红线区**。动手前先读根目录 `AGENTS.md`（约束边界、知识路由、验证闭环），再按本文件执行。

## 目录职责

- bundle.json `inner_kits` 声明的 **27 个对外头文件**全部位于本目录（ashmem / refbase / parcel / safe_* / singleton / observer / thread* / timer / mapped_file 等，以 bundle.json 实际清单为准）；
- 未列入声明的头文件（`io_event_handler.h`、`io_event_reactor.h`、`io_event_common.h`、`mapped_file.h`、`common_event_sys_errors.h`、`common_mapped_file_errors.h`）同在本目录，**同样按对外头文件标准对待**。

## 分层速查

| 类别 | 头文件 | 改动风险点 |
|---|---|---|
| 模板全实现（头文件即实现） | safe_block_queue.h / safe_map.h / safe_queue.h / singleton.h / sorted_vector.h / flat_obj.h / nocopyable.h | 实现位移无 ABI 问题，但语义变化（如计数时机、内存序）会改变可观察行为——既有测试和下游可能依赖旧行为 |
| 声明+src 实现的重型类 | refbase.h / parcel.h / ashmem.h / observer.h / thread_ex.h / thread_pool.h / timer.h / rwlock.h / semaphore_ex.h | 改成员布局或方法签名破坏二进制兼容；parcel 读写顺序即 IPC 线上格式 |
| 工具宏 | thread_safety_analysis_macros.h / pubdef.h / errors.h 系列 | 锁注解宏是编译期检查，不得绕过 |

## 硬规则

- **禁止**修改已有 API 的签名、语义、对象布局；**禁止**删除或重命名已有 API；新增对外 API 需评估 bundle.json 声明同步。
- 头文件必须**自包含**（Include What You Use），不得依赖包含顺序。
- 修复缺陷做**最小改动**（如计数入锁、补 memory_order、补判空），不顺手重构、不批量重排。
- 返回裸指针/引用的接口 修判空时对齐同类中已有的安全访问器，不发明第二套守卫。

## 修改后验证（最低要求）

改本目录任一头文件 = **全组件构建 + 全部单测**，命令见根目录 `AGENTS.md`“验证闭环”一节。

涉及 refbase / parcel / ashmem / timer 的改动，额外构建并运行对应 fuzz（`base/test/fuzztest/`）与 benchmark（`base/test/benchmarktest/`）。
