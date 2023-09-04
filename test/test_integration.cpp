// Must include the gtest header to use the testing library
#include <gtest/gtest.h>
#include <iostream>
#include <ExgBackE.hpp>
#include <InpStrm.hpp>
#include <thread>
#include <chrono>

void saveToCSV(const std::vector<std::string> &data, const std::string &filename)
{
    std::ofstream outputFile(filename);

    if (!outputFile.is_open())
    {
        return;
    }

    for (const std::string &line : data)
    {
        outputFile << line << std::endl;
    }

    outputFile.close();
}

bool compare(std::vector<std::string> &a, std::vector<std::string> &b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    for (size_t i = 0; i < a.size(); i++)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }
    return true;
}

std::vector<std::string> doTestReturnOutput(int sleep_ms, std::string inputf, std::string destination_address = "127.0.0.1", int port = 1234)
{
    boost::asio::io_context ioContext;
    ExchangeBackend backend(ioContext, destination_address, port);
    backend.start();

    std::thread workerThread(sendFileOverUDP, inputf, destination_address, port);
    workerThread.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms)); // wait for messages to be processed before reading output
    std::vector<std::string> output = backend.getPrintOutput();
    backend.stop();
    return output;
}

bool doTest(std::string inputf, std::string outputf, int sleep_ms = 1, std::string destination_address = "127.0.0.1", int port = 1234)
{
    std::vector<std::string> output = doTestReturnOutput(sleep_ms, inputf, destination_address, port);
    // expected output
    std::ifstream ofile(outputf);
    std::vector<std::string> expected;

    if (ofile)
    {
        std::string line;
        while (std::getline(ofile, line))
        {
            expected.push_back(line);
        }
    }
    return compare(output, expected);
}

// Define a parameterized test fixture
class ParameterizedTest : public testing::TestWithParam<std::string>
{
};

// Define a data provider function that returns a list of test values
std::vector<std::string> TestData()
{
    return {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
            "11", "12", "13", "14", "15", "16", "_all_scenarios"};
}

// Use TEST_P to define a parameterized test
TEST_P(ParameterizedTest, IntegrationTest)
{
    std::string val = GetParam();
    std::string currentFile = __FILE__;
    std::string inp1 = "/test_cases/input" + val + ".csv";
    std::string out1 = "/test_cases/output" + val + ".csv";

    // Combine the paths to get the full path to the CSV file.
    std::string inputf = currentFile.substr(0, currentFile.find_last_of("/\\") + 1) + inp1;
    std::string outputf = currentFile.substr(0, currentFile.find_last_of("/\\") + 1) + out1;

    bool out = doTest(inputf, outputf);
    ASSERT_TRUE(out);
}

TEST(StressTest, IntegrationTest)
{
    std::string currentFile = __FILE__;
    std::string inp1 = "/test_cases/stress_test.csv";
    std::string out1 = "/test_cases/st_o.csv";

    // Combine the paths to get the full path to the CSV file.
    std::string inputf = currentFile.substr(0, currentFile.find_last_of("/\\") + 1) + inp1;
    std::string outputf = currentFile.substr(0, currentFile.find_last_of("/\\") + 1) + out1;

    std::vector<std::string> output = doTestReturnOutput(500, inputf);

    size_t actualSize = output.size();
    bool condition = (actualSize = 363000);
    std::cout << "output size: " << actualSize << std::endl;
    // saveToCSV(output, outputf);
    EXPECT_TRUE(condition);
}

// Instantiate the test cases with the values from TestData
INSTANTIATE_TEST_CASE_P(InstantiationName, ParameterizedTest, testing::ValuesIn(TestData()));

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}