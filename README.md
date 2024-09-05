
# ESP_SR_VoiceCommunication

## Supported Targets

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

## Sample Project

This is the simplest buildable example. The project is used by the command `idf.py create-project`, which copies the project to a user-specified path and sets its name. For more information, follow the [documentation page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

## How to Use the Example

We encourage users to use this example as a template for new projects. A recommended way is to follow the instructions on the [documentation page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

## Example Folder Contents

The project **sample_project** contains one source file in C language, [main.c](main/main.c), which is located in the [main](main) folder. ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide directives and instructions describing the project's source files and targets (executable, library, or both).

Below is a short explanation of the remaining files in the project folder:

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md (This is the file you are currently reading)
```

Additionally, the sample project contains Makefile and component.mk files, which were used for the legacy Make-based build system. They are not used or needed when building with CMake and `idf.py`.
