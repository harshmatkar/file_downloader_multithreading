#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <cwchar> 


#pragma comment(lib, "winhttp.lib")

const int NUM_THREADS = 4;

struct Chunk {
    int id;
    DWORD start;
    DWORD end;
    std::string host;
    std::string path;
    std::vector<char> data;
};

std::mutex cout_mutex;

void download_chunk(Chunk& chunk) {
    HINTERNET hSession = WinHttpOpen(L"Downloader/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    HINTERNET hConnect = WinHttpConnect(hSession, std::wstring(chunk.host.begin(), chunk.host.end()).c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET",
                                            std::wstring(chunk.path.begin(), chunk.path.end()).c_str(),
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            0);

    std::string rangeHeader = "Range: bytes=" + std::to_string(chunk.start) + "-" + std::to_string(chunk.end);
    WinHttpAddRequestHeaders(hRequest, std::wstring(rangeHeader.begin(), rangeHeader.end()).c_str(),
                             -1L, WINHTTP_ADDREQ_FLAG_ADD);

    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                       WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    WinHttpReceiveResponse(hRequest, NULL);

    DWORD bytesRead = 0;
    char buffer[4096];
    do {
        WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead);
        chunk.data.insert(chunk.data.end(), buffer, buffer + bytesRead);
    } while (bytesRead > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "Chunk " << chunk.id << " downloaded: " << chunk.data.size() << " bytes.\n";
}

DWORD get_content_length(const std::wstring& host, const std::wstring& path) {
    DWORD length = 0;
    HINTERNET hSession = WinHttpOpen(L"Downloader/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"HEAD", path.c_str(),
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                       WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    WinHttpReceiveResponse(hRequest, NULL);

    DWORD size = sizeof(length);
    WCHAR szContentLength[32];
    DWORD dwSize = sizeof(szContentLength);
    WinHttpQueryHeaders(hRequest,
        WINHTTP_QUERY_CONTENT_LENGTH,
        NULL,
        &szContentLength,
        &dwSize,
        WINHTTP_NO_HEADER_INDEX);

    length = _wtoi(szContentLength); 


    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return length;
}

void join_chunks(const std::vector<Chunk>& chunks, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    for (const auto& chunk : chunks) {
        out.write(chunk.data.data(), chunk.data.size());
    }
    out.close();
    std::cout << "Download completed: " << filename << "\n";
}

int main() {
    std::string url = "http://speedtest.tele2.net/1MB.zip";  
    std::string host = "speedtest.tele2.net";
    std::string path = "/1MB.zip";

    DWORD totalSize = get_content_length(std::wstring(host.begin(), host.end()), std::wstring(path.begin(), path.end()));
    std::cout << "Total size: " << totalSize << " bytes\n";

    DWORD chunkSize = totalSize / NUM_THREADS;
    std::vector<Chunk> chunks(NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; ++i) {
        chunks[i].id = i;
        chunks[i].start = i * chunkSize;
        chunks[i].end = (i == NUM_THREADS - 1) ? totalSize - 1 : (chunks[i].start + chunkSize - 1);
        chunks[i].host = host;
        chunks[i].path = path;
    }

    std::vector<std::thread> threads;
    for (auto& chunk : chunks) {
        threads.emplace_back(download_chunk, std::ref(chunk));
    }

    for (auto& t : threads) t.join();

    join_chunks(chunks, "output_file.zip");
    return 0;
}
