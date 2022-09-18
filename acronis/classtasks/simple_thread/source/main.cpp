#include <fstream>
#include <thread>
#include <vector>

int main()
{
    constexpr auto symbolsToWrite = 100;
    constexpr auto threadNum = 4;

    // Create buffer, reserve capacity for each symbol
    using buffer_t = std::vector<char>;
    buffer_t buffer(symbolsToWrite);

    // Initialize thread container
    std::vector<std::thread> threads;
    threads.reserve(2 * threadNum);

    // Set lambdas that will do write/read work
    auto writeWork = [](buffer_t& buffer, int threadId){
        for (auto i = 0; i < symbolsToWrite; ++i)
            buffer[i] = static_cast<char>(threadId) + '0';
    };

    auto readWork = [](const buffer_t& buffer, int threadId){
        std::ofstream outputFile;
        outputFile.open("output_thread" + std::to_string(threadId) + ".txt");

        for (auto i = 0; i < symbolsToWrite; ++i)
            outputFile << buffer[i];
    };

    // Launch threads that will write to the buffer
    for (int i = 0; i < threadNum; ++i)
        threads.push_back(std::thread(writeWork, std::ref(buffer), i));
    // Launch thread that will read from the buffer
    for (int i = 0; i < threadNum; ++i)
        threads.push_back(std::thread(readWork, std::cref(buffer), i));

    // Wait for all thread to finish job
    for (auto&& thread : threads)
        thread.join();

    return EXIT_SUCCESS;
}