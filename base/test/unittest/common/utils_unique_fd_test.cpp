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

#include "unique_fd.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#ifdef OHOS_PLATFORM
#include <stdio.h>
#endif
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace {
class UtilsUniqueFd : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    static char const* testfilename;
};

char const* UtilsUniqueFd::testfilename = "testfilename.test";

void UtilsUniqueFd::SetUpTestCase(void)
{

    ofstream outfile;
    outfile.open(testfilename, ios::out | ios::trunc);
    outfile << "testdata\n"
            << std::endl;
    outfile.close();
}

void UtilsUniqueFd::TearDownTestCase(void)
{

    ifstream inputfile;
    inputfile.open(testfilename, ios::in);
    std::string testStr;
    inputfile >> testStr;
    inputfile.close();
    EXPECT_EQ(remove(testfilename), 0);
}

HWTEST_F(UtilsUniqueFd, testUtilsUniqueFd, TestSize.Level0)
{
    int fd = open("NOTHISFILE", O_RDWR, 0666);

    UniqueFd ufd2(fd);
    EXPECT_EQ(ufd2, -1);
};

HWTEST_F(UtilsUniqueFd, testUtilsUniqueCtroFromInt, TestSize.Level0)
{

    UniqueFd ufd2(open(testfilename, O_RDWR, 0666));
    EXPECT_NE(ufd2, -1);
};

HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdeqcompare, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR, 0666);
    UniqueFd ufd2(fd);
    ASSERT_EQ(fd, ufd2);
    ASSERT_EQ(ufd2, fd);
};

HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdeqcompareNl, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR, 0666);
    UniqueFd ufd2(fd);
    ASSERT_TRUE(ufd2 >= 0);
    ASSERT_TRUE(0 <= ufd2);
};

HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdeqcompareBg, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR, 0666);
    UniqueFd ufd2(fd);
    ASSERT_TRUE(ufd2 > 0);
    ASSERT_TRUE(0 < ufd2);
};

HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdeqcompareNb, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR, 0666);
    UniqueFd ufd2(fd);
    ASSERT_TRUE(ufd2 <= 1000000);
    ASSERT_TRUE(1000000 >= ufd2);
};

HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdeqcompareLess, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR, 0666);
    UniqueFd ufd2(fd);
    ASSERT_TRUE(ufd2 < 1000000);
    ASSERT_TRUE(1000000 > ufd2);
};

HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdeqcompareNeq, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR, 0666);
    UniqueFd ufd2(fd);
    ASSERT_TRUE(ufd2 != 1000000);
    ASSERT_TRUE(1000000 != ufd2);
};

class NewDeleter {
public:
    static int iflag;
    static void Close(int fd)
    {

        iflag = 10;
        close(fd);
    }
};

int NewDeleter::iflag = 0;

HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdDefineDeletor, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR);

    {
        UniqueFdAddDeletor<NewDeleter> ufd2(fd);
        ASSERT_EQ(NewDeleter::iflag, 0);
        if (ufd2 == -1) {
            std::cout << "open test.h error" << std::endl;
        }
    }
    ASSERT_EQ(NewDeleter::iflag, 10);
};

HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdDefineDeletorCloseStatus, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR);

    {
        UniqueFdAddDeletor<NewDeleter> ufd2(fd);
    }

    char buf[] = "test";
    int ret = write(fd, buf, sizeof(buf));
    ASSERT_EQ(ret, -1);
};

HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdReleaseAndGet, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR);

    UniqueFd ufd2(fd);
    ASSERT_EQ(ufd2.Get(), fd);
    ASSERT_EQ(ufd2.Release(), fd);

    UniqueFd ufd3(fd);
    UniqueFd ufd1;
    ufd1 = std::move(ufd3);
    ASSERT_EQ(ufd1, fd);
};

/*
 * @tc.name: testUtilsUniqueFdMoveCtor
 * @tc.desc: Test the move constructor: the fd is transferred and the source becomes empty.
 */
HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdMoveCtor, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR);
    ASSERT_GE(fd, 0);

    UniqueFd src(fd);
    UniqueFd dst(std::move(src));
    ASSERT_EQ(dst, fd);   // the fd is now owned by dst
    ASSERT_EQ(src, -1);   // the source is empty
    ASSERT_NE(fcntl(fd, F_GETFD), -1); // the fd keeps open after the transfer

    // the moved-from object is reusable
    int fd2 = open(testfilename, O_RDWR);
    src = UniqueFd(fd2);
    ASSERT_EQ(src, fd2);
};

/*
 * @tc.name: testUtilsUniqueFdMoveAssignCloseOld
 * @tc.desc: Test the move assignment: the old fd is closed and the new one is taken over.
 */
HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdMoveAssignCloseOld, TestSize.Level0)
{
    int fd1 = open(testfilename, O_RDWR);
    int fd2 = open(testfilename, O_RDWR);
    ASSERT_GE(fd1, 0);
    ASSERT_GE(fd2, 0);

    UniqueFd a(fd1);
    UniqueFd b(fd2);
    a = std::move(b);
    ASSERT_EQ(a, fd2);   // the fd of b is taken over
    ASSERT_EQ(b, -1);    // the source is empty
    ASSERT_EQ(fcntl(fd1, F_GETFD), -1); // the old fd has been closed
    ASSERT_NE(fcntl(fd2, F_GETFD), -1); // the taken-over fd keeps open
};

/*
 * @tc.name: testUtilsUniqueFdSelfMoveAssign
 * @tc.desc: Test the self move assignment: it must be a no-op.
 */
HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdSelfMoveAssign, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR);
    ASSERT_GE(fd, 0);

    UniqueFd a(fd);
    UniqueFd& self = a;
    a = std::move(self); // self move assignment via alias
    ASSERT_EQ(a, fd);   // the fd is preserved, neither closed nor lost
    ASSERT_NE(fcntl(fd, F_GETFD), -1);
};

/*
 * @tc.name: testUtilsUniqueFdDtorClose
 * @tc.desc: Test the destructor: the managed fd is closed.
 */
HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdDtorClose, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR);
    ASSERT_GE(fd, 0);

    {
        UniqueFd ufd(fd);
    }
    ASSERT_EQ(fcntl(fd, F_GETFD), -1); // closed by the destructor
};

#ifdef OHOS_PLATFORM
/*
 * @tc.name: testUtilsUniqueFdFdsanTagLifecycle
 * @tc.desc: Test the fdsan ownership tag through adoption, move and release.
 */
HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdFdsanTagLifecycle, TestSize.Level0)
{
    int fd = open(testfilename, O_RDWR);
    ASSERT_GE(fd, 0);

    // 1. adoption: the tag becomes non-zero once the fd is taken over
    UniqueFd ufd(fd);
    uint64_t tag = fdsan_get_owner_tag(fd);
    ASSERT_NE(tag, 0u);

    // 2. move: the tag is preserved on the fd (no untagged gap during the transfer)
    UniqueFd ufd2(std::move(ufd));
    ASSERT_EQ(fdsan_get_owner_tag(fd), tag);

    // 3. release: the tag is cleared and the fd is handed over clean
    ASSERT_EQ(ufd2.Release(), fd);
    ASSERT_EQ(fdsan_get_owner_tag(fd), 0u);

    close(fd); // caller-side manual close on the clean fd
};

/*
 * @tc.name: testUtilsUniqueFdFdsanTagMoveAssign
 * @tc.desc: Test the fdsan tag handling of the move assignment.
 */
HWTEST_F(UtilsUniqueFd, testUtilsUniqueFdFdsanTagMoveAssign, TestSize.Level0)
{
    int fd1 = open(testfilename, O_RDWR);
    int fd2 = open(testfilename, O_RDWR);
    ASSERT_GE(fd1, 0);
    ASSERT_GE(fd2, 0);

    UniqueFd a(fd1);
    UniqueFd b(fd2);
    uint64_t tag2 = fdsan_get_owner_tag(fd2);
    ASSERT_NE(tag2, 0u);

    a = std::move(b);
    ASSERT_EQ(fdsan_get_owner_tag(fd1), 0u);  // the closed old fd is cleanly untagged
    ASSERT_EQ(fdsan_get_owner_tag(fd2), tag2); // the transferred fd keeps its tag
};
#endif
}  // namespace
}  // namespace OHOS
