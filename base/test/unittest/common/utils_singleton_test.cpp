/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>
#include "singleton.h"
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace {
class DelayedSingletonDeclearTest {
    DECLARE_DELAYED_SINGLETON(DelayedSingletonDeclearTest);
public:
    void* GetObjAddr()
    {
        return static_cast<void*>(this);
    }
};

DelayedSingletonDeclearTest::~DelayedSingletonDeclearTest() {};
DelayedSingletonDeclearTest::DelayedSingletonDeclearTest() {};

class SingletonDeclearTest {
    DECLARE_SINGLETON(SingletonDeclearTest);
public:
    void* GetObjAddr()
    {
        return static_cast<void*>(this);
    }
};

SingletonDeclearTest::~SingletonDeclearTest() {};
SingletonDeclearTest::SingletonDeclearTest() {};

class SingletonTest: public Singleton<SingletonTest> {
public:
    void* GetObjAddr()
    {
        return static_cast<void*>(this);
    }
};

class DelayedSingletonTest: public DelayedSingleton<DelayedSingletonTest> {
public:
    void* GetObjAddr()
    {
        return static_cast<void*>(this);
    }
};


class DelayedRefSingletonDeclearTest {
    DECLARE_DELAYED_REF_SINGLETON(DelayedRefSingletonDeclearTest);
public:
    void* GetObjAddr()
    {
        return static_cast<void*>(this);
    }
};

DelayedRefSingletonDeclearTest::DelayedRefSingletonDeclearTest() {};
DelayedRefSingletonDeclearTest::~DelayedRefSingletonDeclearTest() {};

class DelayedRefSingletonTest: public DelayedRefSingleton<DelayedRefSingletonTest> {
public:
    void* GetObjAddr()
    {
        return static_cast<void*>(this);
    }
};


class UtilsSingletonTest : public testing::Test
{
public :
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UtilsSingletonTest::SetUpTestCase(void)
{
    // step 2: input testsuit setup step
}

void UtilsSingletonTest::TearDownTestCase(void)
{
    // step 2: input testsuit teardown step
}

void UtilsSingletonTest::SetUp(void)
{
    // step 3: input testcase setup step
}

void UtilsSingletonTest::TearDown(void)
{
    // step 3: input testcase teardown step
}

HWTEST_F(UtilsSingletonTest, test_DelayedSingletonDeclearTest, TestSize.Level0)
{
    shared_ptr<DelayedSingletonDeclearTest> sp1 = DelayedSingleton<DelayedSingletonDeclearTest>::GetInstance();
    EXPECT_EQ(sp1.use_count(), 2);
    shared_ptr<DelayedSingletonDeclearTest> sp2 = DelayedSingleton<DelayedSingletonDeclearTest>::GetInstance();
    EXPECT_EQ(sp1->GetObjAddr(), sp2->GetObjAddr());
    EXPECT_EQ(sp1.get(), sp2.get());
    EXPECT_EQ(sp2.use_count(), 3);
}


HWTEST_F(UtilsSingletonTest, test_SingletonDeclearTest, TestSize.Level0)
{
    SingletonDeclearTest &st1 = Singleton<SingletonDeclearTest>::GetInstance();
    SingletonDeclearTest &st2 = Singleton<SingletonDeclearTest>::GetInstance();
    EXPECT_EQ(st1.GetObjAddr(), st2.GetObjAddr());
}


HWTEST_F(UtilsSingletonTest, test_SingletonTest, TestSize.Level0)
{
    SingletonTest &st1 = SingletonTest::GetInstance();
    SingletonTest &st2 = SingletonTest::GetInstance();
    EXPECT_EQ(st1.GetObjAddr(), st2.GetObjAddr());
}

HWTEST_F(UtilsSingletonTest, test_DestroyInstanceTest, TestSize.Level0)
{
    shared_ptr<DelayedSingletonTest> sp1 = DelayedSingletonTest::GetInstance();
    auto oldInstance = sp1;
    sp1->DestroyInstance();

    auto newInstance = sp1->GetInstance();
    ASSERT_NE(oldInstance->GetObjAddr(), newInstance->GetObjAddr());
}

HWTEST_F(UtilsSingletonTest, test_DelayedSingletonTest, TestSize.Level0)
{
    shared_ptr<DelayedSingletonTest> sp1 = DelayedSingletonTest::GetInstance();
    EXPECT_EQ(sp1.use_count(), 2);
    shared_ptr<DelayedSingletonTest> sp2 = DelayedSingletonTest::GetInstance();
    EXPECT_EQ(sp1->GetObjAddr(), sp2->GetObjAddr());
    EXPECT_EQ(sp1.get(), sp2.get());
    EXPECT_EQ(sp2.use_count(), 3);
}

HWTEST_F(UtilsSingletonTest, test_DelayedRefSingletonTest, TestSize.Level0)
{
    DelayedRefSingletonTest& p1 = DelayedRefSingletonTest::GetInstance();
    DelayedRefSingletonTest& p2 = DelayedRefSingletonTest::GetInstance();
    EXPECT_EQ(p1.GetObjAddr(), p2.GetObjAddr());
}

HWTEST_F(UtilsSingletonTest, test_DelayedRefSingletonDeclearTest, TestSize.Level0)
{
    DelayedRefSingletonDeclearTest& p1 = DelayedRefSingleton<DelayedRefSingletonDeclearTest>::GetInstance();
    DelayedRefSingletonDeclearTest& p2 = DelayedRefSingleton<DelayedRefSingletonDeclearTest>::GetInstance();
    EXPECT_EQ(p1.GetObjAddr(), p2.GetObjAddr());
}

class DelayedRefSingletonConcurrentInitTest
    : public DelayedRefSingleton<DelayedRefSingletonConcurrentInitTest> {
public:
    DelayedRefSingletonConcurrentInitTest()
    {
        for (int round = 0; round < CTOR_ROUNDS; ++round) {
            for (int j = 0; j < PAYLOAD_SIZE; ++j) {
                payload[j] = j + 1;
            }
        }
        ctorCalls_.fetch_add(1, std::memory_order_relaxed);
    }

    static constexpr int PAYLOAD_SIZE = 64;
    static constexpr int CTOR_ROUNDS = 10000;
    static std::atomic<int> ctorCalls_;
    int payload[PAYLOAD_SIZE];
};

std::atomic<int> DelayedRefSingletonConcurrentInitTest::ctorCalls_(0);

/*
 * @tc.name: test_DelayedRefSingletonConcurrentInit
 * @tc.desc: Concurrent first calls of GetInstance construct exactly one instance,
 *           return the same address and expose fully initialized members.
 */
HWTEST_F(UtilsSingletonTest, test_DelayedRefSingletonConcurrentInit, TestSize.Level0)
{
    const int THREAD_NUM = 8;
    DelayedRefSingletonConcurrentInitTest* results[THREAD_NUM] = {};
    bool intact[THREAD_NUM] = {};
    std::mutex gateMutex;
    std::condition_variable gateCv;
    int ready = 0;
    bool start = false;

    std::vector<std::thread> threads;
    for (int t = 0; t < THREAD_NUM; ++t) {
        threads.emplace_back([&, t] {
            {
                std::unique_lock<std::mutex> lock(gateMutex);
                ++ready;
                gateCv.notify_all();
                gateCv.wait(lock, [&start] { return start; });
            }
            DelayedRefSingletonConcurrentInitTest* ptr =
                &DelayedRefSingletonConcurrentInitTest::GetInstance();
            bool ok = true;
            for (int j = 0; j < DelayedRefSingletonConcurrentInitTest::PAYLOAD_SIZE; ++j) {
                ok = ok && (ptr->payload[j] == j + 1);
            }
            results[t] = ptr;
            intact[t] = ok;
        });
    }

    {
        std::unique_lock<std::mutex> lock(gateMutex);
        gateCv.wait(lock, [&] { return ready == THREAD_NUM; });
        start = true;
        gateCv.notify_all();
    }

    for (auto& thd : threads) {
        thd.join();
    }

    ASSERT_EQ(DelayedRefSingletonConcurrentInitTest::ctorCalls_.load(), 1);
    for (int t = 1; t < THREAD_NUM; ++t) {
        ASSERT_EQ(results[t], results[0]);
    }
    for (int t = 0; t < THREAD_NUM; ++t) {
        ASSERT_TRUE(intact[t]);
    }
}
}  // namespace
}  // namespace OHOS