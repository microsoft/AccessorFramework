// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <chrono>
#include <thread>
#include "ExampleHost.h"

using namespace std::chrono_literals;

int main()
{
    // Instantiate and initialize the model
    ExampleHost host("Host");
    host.Setup();

    // Iterate the model five times, then sleep for one second
    host.Iterate(5);
    std::this_thread::sleep_for(1s);

    // Run for five seconds, then pause and sleep for one second
    host.Run();
    std::this_thread::sleep_for(5s);
    host.Pause();
    std::this_thread::sleep_for(1s);

    // Resume for five seconds, then exit
    host.Run();
    std::this_thread::sleep_for(5s);
    host.Exit();

    return 0;
}
