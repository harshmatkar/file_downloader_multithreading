# Multithreaded HTTP File Downloader (C++ / WinHTTP)

This project is a multithreaded file downloader built using C++ and WinHTTP on Windows. It downloads a file from a given HTTP URL by dividing it into multiple chunks and downloading them in parallel using threads. This approach improves download performance by utilizing concurrent HTTP requests.

## Features

- Supports multi-threaded downloading via HTTP Range headers
- Adjustable number of threads through command-line input
- Combines all downloaded chunks into a single output file
- Measures and displays total download time
- Generates an optional SHA256 hash verification command

## Requirements

- Windows operating system
- C++ compiler (Microsoft Visual C++ or g++)
- Internet connection
- Basic familiarity with the command prompt

## Building the Project

### Using Microsoft Visual C++ (Command Prompt)

1. Open the "Developer Command Prompt for Visual Studio"
2. Run the following command:

```bash
cl /EHsc downloader.cpp /link winhttp.lib
```
```bash
Sample Output
Downloading: http://speedtest.tele2.net/1MB.zip using 4 threads
Total file size: 1048576 bytes
Chunk 0 downloaded: 0-262143 (262144 bytes)
Chunk 1 downloaded: 262144-524287 (262144 bytes)
Chunk 2 downloaded: 524288-786431 (262144 bytes)
Chunk 3 downloaded: 786432-1048575 (262144 bytes)
Total download time: 0.72 seconds
Final file written: output_file.zip

