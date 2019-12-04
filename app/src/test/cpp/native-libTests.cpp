#include <gtest/gtest.h>
#include <native-lib.h>

using namespace std;

TEST(CreateWelcomingString, NotEmpty)
{
    EXPECT_NE(createWelcomingString().length(), 0);
}

TEST(CreateWelcomingString, Validity)
{
    const string welcomingString = createWelcomingString();
    EXPECT_EQ(welcomingString, "Hello from C++");
    EXPECT_NE(welcomingString, "Good bye from C++");
}
