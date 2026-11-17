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
#include "observer.h"
#include <algorithm>
#include <atomic>
#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <vector>
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace {
class BookList: public Observable {
public:
    BookList() { books_.clear(); }
    void AddBook(const string& book)
    {
        books_.insert(book);
        SetChanged();
        NotifyObservers();
    }

    void RemoveBook(const string& book)
    {
        books_.erase(book);
        SetChanged();
        NotifyObservers();
    }

    void NoChangeNotify()
    {
        if (HasChanged() == false) {
            NotifyObservers();
        }
    }

    const set<string>& GetBooks() { return books_; }
private:
    set<string> books_;

};

class BookObserver: public Observer {
public:
    virtual void Update(const Observable* o, const ObserverArg* /* arg */)
    {
        BookList* bookList = reinterpret_cast<BookList*>(const_cast<Observable*>(o));
        books_ = bookList->GetBooks();
    }

    int GetBooksCount() { return static_cast<int>(books_.size()); }
    bool BookExists(const string& book) { return books_.count(book) > 0;}
private:
    set<string> books_;
};


class UtilsObserverTest : public testing::Test {
public :
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UtilsObserverTest::SetUpTestCase(void)
{
}

void UtilsObserverTest::TearDownTestCase(void)
{
}

void UtilsObserverTest::SetUp(void)
{
}

void UtilsObserverTest::TearDown(void)
{
}

/*
 * @tc.name: test_Observer
 * @tc.desc: Test add null or repeat observer to the observable object.
 */
HWTEST_F(UtilsObserverTest, test_Observer, TestSize.Level0)
{
    BookList bookList;
    bookList.AddObserver(nullptr);
    shared_ptr<BookObserver> bookObserver1 = make_shared<BookObserver>();
    bookList.AddObserver(bookObserver1);
    bookList.AddObserver(bookObserver1);
    bookList.NoChangeNotify();
    int ret = bookList.GetObserversCount();
    EXPECT_EQ(ret, 1);
}

HWTEST_F(UtilsObserverTest, test_ObserverNotify, TestSize.Level0)
{
    BookList bookList;
    shared_ptr<BookObserver> bookObserver1 = make_shared<BookObserver>();
    shared_ptr<BookObserver> bookObserver2 = make_shared<BookObserver>();
    shared_ptr<BookObserver> bookObserver3 = make_shared<BookObserver>();

    bookList.AddObserver(bookObserver1);
    bookList.AddObserver(bookObserver2);
    bookList.AddObserver(bookObserver3);
    bookList.AddBook("book1");

    EXPECT_EQ(bookObserver1->GetBooksCount(), 1);
    EXPECT_EQ(bookObserver2->GetBooksCount(), 1);
    EXPECT_EQ(bookObserver3->GetBooksCount(), 1);

    bookList.RemoveObserver(bookObserver1);
    bookList.RemoveBook("book1");
    EXPECT_EQ(bookObserver1->GetBooksCount(), 1);
    EXPECT_EQ(bookObserver2->GetBooksCount(), 0);
    EXPECT_EQ(bookObserver3->GetBooksCount(), 0);

    bookList.RemoveObserver(bookObserver2);
    bookList.AddBook("book2");
    bookList.AddBook("book3");
    EXPECT_EQ(bookObserver1->GetBooksCount(), 1);
    EXPECT_EQ(bookObserver2->GetBooksCount(), 0);
    EXPECT_EQ(bookObserver3->GetBooksCount(), 2);

}


HWTEST_F(UtilsObserverTest, test_RemoveAllObserver, TestSize.Level0)
{
    BookList bookList;
    shared_ptr<BookObserver> bookObserver1 = make_shared<BookObserver>();
    shared_ptr<BookObserver> bookObserver2 = make_shared<BookObserver>();
    shared_ptr<BookObserver> bookObserver3 = make_shared<BookObserver>();

    bookList.AddObserver(bookObserver1);
    bookList.AddObserver(bookObserver2);
    bookList.AddObserver(bookObserver3);
    bookList.AddBook("book1");

    EXPECT_EQ(bookObserver1->GetBooksCount(), 1);
    EXPECT_EQ(bookObserver2->GetBooksCount(), 1);
    EXPECT_EQ(bookObserver3->GetBooksCount(), 1);

    bookList.RemoveAllObservers();
    bookList.RemoveBook("book1");
    EXPECT_EQ(bookObserver1->GetBooksCount(), 1);
    EXPECT_EQ(bookObserver2->GetBooksCount(), 1);
    EXPECT_EQ(bookObserver3->GetBooksCount(), 1);
    EXPECT_EQ(bookList.GetObserversCount(), 0);
}

class ConcurrentObservable : public Observable {
public:
    void MarkChanged()
    {
        SetChanged();
    }

    void UnmarkChanged()
    {
        ClearChanged();
    }

    bool IsMarked()
    {
        return HasChanged();
    }
};

class ConcurrentObserver : public Observer {
public:
    virtual void Update(const Observable* /*o*/, const ObserverArg* /*arg*/)
    {
        count_.fetch_add(1, std::memory_order_relaxed);
    }

    std::atomic<int> count_ { 0 };
};

/*
 * @tc.name: test_ObserverConcurrentNotify
 * @tc.desc: Concurrent flag operations, observer churn and notifications on one
 *           observable must not deadlock and must keep the merge semantics intact.
 */
HWTEST_F(UtilsObserverTest, test_ObserverConcurrentNotify, TestSize.Level0)
{
    const int ITERS = 20000;
    ConcurrentObservable subject;
    auto keep = std::make_shared<ConcurrentObserver>();
    auto churn = std::make_shared<ConcurrentObserver>();
    subject.AddObserver(keep);

    std::vector<std::thread> threads;
    threads.emplace_back([&] {
        for (int i = 0; i < ITERS; ++i) {
            subject.MarkChanged();
            subject.NotifyObservers(nullptr);
        }
    });
    threads.emplace_back([&] {
        for (int i = 0; i < ITERS; ++i) {
            subject.UnmarkChanged();
            subject.IsMarked();
        }
    });
    threads.emplace_back([&] {
        for (int i = 0; i < ITERS; ++i) {
            subject.AddObserver(churn);
            subject.RemoveObserver(churn);
        }
    });
    threads.emplace_back([&] {
        for (int i = 0; i < ITERS; ++i) {
            subject.NotifyObservers(nullptr);
        }
    });

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_GT(keep->count_.load(), 0);

    subject.RemoveAllObservers();
    auto fresh = std::make_shared<ConcurrentObserver>();
    subject.AddObserver(fresh);
    subject.MarkChanged();
    subject.NotifyObservers(nullptr);
    ASSERT_EQ(fresh->count_.load(), 1);
    subject.NotifyObservers(nullptr);
    ASSERT_EQ(fresh->count_.load(), 1);
}
}  // namespace
}  // namespace OHOS