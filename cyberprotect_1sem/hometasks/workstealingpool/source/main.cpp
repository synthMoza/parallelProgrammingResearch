// C++ headers
#include <iostream>
#include <string_view>
#include <stdexcept>
#include <array>
// C headers
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <threadpool.h>

using namespace util;

constexpr size_t NUM_THREADS = 8; 
constexpr size_t MAX_EVENTS = 5;
constexpr size_t BUFFER_SIZE = 4096;

void EventLoopDescriptors(const std::vector<int>& descs)
{
    std::string readBuffer(BUFFER_SIZE, ' ');
    std::array<epoll_event, MAX_EVENTS> events;

    // init epoll
    int epollFd = epoll_create1(0);
    if (epollFd < 0)
        throw std::runtime_error("Failed to create epoll descriptor");
    
    // add descs to epoll
    epoll_event event = {};
    for (auto&& desc : descs)
    {
        event.data.fd = desc;
        event.events = EPOLLIN;

        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, desc, &event) < 0) {
            close(epollFd);
            perror("");
            std::cout << "desc = " << desc << std::endl;
            throw std::runtime_error("Failed to add file descriptor to epoll");
	    }
    }

    // to stop this loop, we will add STDIN to epoll, too
    event.data.fd = STDIN_FILENO;
    event.events = EPOLLIN;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &event) < 0) {
        close(epollFd);
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }

    bool isActive = true;
    ThreadPool threadPool(NUM_THREADS);
    while (isActive) // we might come up with a better stop criteria
    {
        size_t eventCount = epoll_wait(epollFd, events.data(), MAX_EVENTS, -1);
        for (size_t i = 0; i < eventCount; ++i)
        {
            if (events[i].data.fd == STDIN_FILENO)
            {
                isActive = false;
                break;
            }

            ssize_t bytesRead = read(events[i].data.fd, readBuffer.data(), BUFFER_SIZE);
            if (bytesRead < 0)
                throw std::runtime_error("Failed to read from file descriptor");
            if (bytesRead == 0)
                continue;

            // Create a task for a thread
            auto f = threadPool.Enqueue([](std::string buffer){
                int result = 0;
                int modulus = 72783;
                // don't really know what to do, improvise some task
                for (auto&& c : buffer)
                    result = (result + c) % modulus;
                
                //std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 15000));
            }, readBuffer);
        }
    }

    if (close(epollFd) < 0)
        // only report error
        std::cerr << "Failed to close epoll descriptor" << std::endl;
}

constexpr size_t NUM_DESCS = 25;
constexpr std::string_view FILE_NAME = "/bin/bash";

constexpr size_t PIPE_READ = 0;
constexpr size_t PIPE_WRITE = 1;

// get test descriptots
std::vector<int> GetDescriptors()
{
    std::vector<int> outputDescs(NUM_DESCS), inputDescs(NUM_DESCS);

    // create n pipes
    int pipeFds[2] = {};
    for (size_t i = 0; i < NUM_DESCS; ++i)
    {
        if (pipe(pipeFds) < 0)
            throw std::runtime_error("Can't create pipe");

        inputDescs[i] = pipeFds[PIPE_WRITE];
        outputDescs[i] = pipeFds[PIPE_READ];
    }

    auto pid = fork();
    if (pid == 0)
    {
        // child process, close all read fds
        for (auto&& desc : outputDescs)
            close(desc);
        
        // start to read from test file and put data into random pipe
        int fd = open(FILE_NAME.data(), O_RDONLY);
        if (fd < 0)
            throw std::runtime_error("Can't open test file");
        
        ssize_t readBytes = 0;
        std::string readBuffer(BUFFER_SIZE, ' ');
        while ((readBytes = read(fd, readBuffer.data(), BUFFER_SIZE)) > 0)
        {
            int randomFdIdx = rand() % NUM_DESCS;
            ssize_t writtenBytes = write(inputDescs[randomFdIdx], readBuffer.data(), readBytes);
            if (writtenBytes != readBytes)
                throw std::runtime_error("Failed to write into pipe");
        }

        for (auto&& desc : inputDescs)
            close(desc);
        std::this_thread::sleep_for(std::chrono::seconds(10));
        exit(EXIT_SUCCESS); // work is done, exit the program
    }
    else
    {
        // parent process, close all write fds
        for (auto&& desc : inputDescs)
            close(desc);
    }

    return outputDescs;
}

int main()
{
    auto descs = GetDescriptors();
    EventLoopDescriptors(descs);

    for (auto&& desc : descs)
        close(desc);

    return 0;
}