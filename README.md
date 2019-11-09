# wsvc

## About

wsvc is a simple Windows Service application written in pure C.

The purpose of the project is to demonstrate a clean way of writing Windows Service applications. The service is basically just logging events to the sytem event log and waits for any Windows Service commands.

The code mainly based on Microsoft's example here: https://code.msdn.microsoft.com/windowsapps/CppWindowsService-cacf4948. This project is pretty much the same as the one linked without any C++ usage.

If one is looking to implement a Windows Service from scratch, it is likely more ideal to write the Windows Service code from .NET as the system call APIs are better supported from that interface. .NET code can take advantage of interop (P/invoke or C++/CLI) so any performance critical code can still be written natively.

## License

Please see the included LICENSE file.

## Requirements

* Microsoft Visual C++ 2019 (Visual Studio 2019)
