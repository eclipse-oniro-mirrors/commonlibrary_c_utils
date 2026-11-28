使用agent修改代码，新增代码结构/功能特性必须同步更新本AGENTS

# AGENTS.md — base/test/（测试树）

动手前先读根目录 `AGENTS.md`；本文件覆盖 unittest / benchmarktest / fuzztest 全部子目录。

## 目录结构与注册方式

| 目录 | 内容 | 注册 |
|---|---|---|
| unittest/common/ | 22 个 `utils_*_test.cpp`（gtest） | 同目录 BUILD.gn：`ohos_unittest("UtilsXxxTest")` **且**加入 `group("unittest")` 的 deps |
| unittest/rust/ | `rust_utils_{ashmem,directory,file}_test.rs` | 同目录 BUILD.gn（仅 linux host、非 asan/arkui_x、含 thirdparty_rust_cxx 部件时编入） |
| benchmarktest/ | 21 个基准子目录（benchmark） | 各子目录 BUILD.gn |
| fuzztest/ | 7 个 fuzzer（refbase/directory/timer/parcel/ashmem/thread/string） | 各子目录 BUILD.gn + project.xml + corpus/init |

## 新增测试的硬规则

- 新文件命名 `utils_<模块>_test.cpp`；**三步注册缺一不可**：定义 `ohos_unittest` 目标 → 加入 `group("unittest")` deps → 确认 `external_deps` 引用 `googletest:gtest_main`；
- 用例格式遵循现有惯例：fixture 类（继承 `testing::Test`）+ `HWTEST_F(...)` + `/* @tc.name / @tc.desc */` 注释块 + `TestSize.Level0`；
- **禁止**用固定 `sleep_for` 做并发同步（根 AGENTS.md 明令）——用 `std::latch` / `std::barrier` / condition_variable / 原子标志对齐线程；存量 sleep 用例不要求顺手重写，但新用例不得再引入；
- 并发用例只断言**确定性不变量**（构造恰好一次、所有结账返回值、终态计数、同地址），不断言依赖调度时序的中间态（如“排空过程中 Join 尚未返回”）；
- fuzz 新目录必须带 `corpus/init` 与 `project.xml`；benchmark 新目录参照同类现有 Fixture 结构。

## 修改后验证（对应根 AGENTS.md“任务分级”新增测试行）

构建该测试目标 + 运行该测试 + 运行相邻测试（同 fixture 或同模块既有用例），命令见根目录 `AGENTS.md`“验证闭环”一节。
