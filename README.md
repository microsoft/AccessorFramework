[![Build Status](https://microsoft.visualstudio.com/Universal%20Print/_apis/build/status/microsoft.AccessorFramework?branchName=master)](https://microsoft.visualstudio.com/Universal%20Print/_build/latest?definitionId=47587&branchName=master)

## About

The Accessor Framework is a C++ SDK that empowers cyber-physical system application developers to build their
applications using the Accessor Model, a component-based programming model originally conceived by researchers at the
[Industrial Cyber-Physical Systems Center (iCyPhy)](https://ptolemy.berkeley.edu/projects/icyphy/) at UC Berkeley.

The Accessor Model enables embedded applications to embrace heterogeneous protocol stacks and be highly asynchronous
while still being highly deterministic. This enables developers to have well-defined test cases, rigorous
specifications, and reliable error checking without sacrificing the performance gains of multi-threading. In addition,
the model eliminates the need for explicit thread management, eliminating the potential for deadlocks and greatly
reducing the potential for race conditions and other non-deterministic behavior.

The SDK is designed to be cross-platform. It is written entirely in C++ and has no dependencies other than the C++14
Standard Library.

The research paper that inspired this project can be found at https://ieeexplore.ieee.org/document/8343871.

## Getting Started

#### Building from Source

```
git clone https://github.com/microsoft/AccessorFramework.git
cd AccessorFramework
mkdir build
cd build
cmake ..
cmake --build .
```

#### Using in a CMake Project

```cmake
# CMakeLists.txt
project(myProject)

find_package(AccessorFramework REQUIRED)

add_executable(myProject main.cpp)
target_link_libraries(myProject PRIVATE AccessorFramework)
```

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
