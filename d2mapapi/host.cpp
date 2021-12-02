#include <windows.h>
#include <string>
#include <cstdio>
#include <cstdint>

HANDLE childStdinRd = nullptr;
HANDLE childStdinWr = nullptr;
HANDLE childStdoutRd = nullptr;
HANDLE childStdoutWr = nullptr;

void queryMap(uint32_t, uint32_t, uint32_t);
void errorExit(LPCSTR);

int main(int argc, char *argv[]) {
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;
    if (!CreatePipe(&childStdoutRd, &childStdoutWr, &saAttr, 0))
        errorExit("StdoutRd CreatePipe");
    if (!SetHandleInformation(childStdoutRd, HANDLE_FLAG_INHERIT, 0))
        errorExit("Stdout SetHandleInformation");
    if (!CreatePipe(&childStdinRd, &childStdinWr, &saAttr, 0))
        errorExit("Stdin CreatePipe");
    if (!SetHandleInformation(childStdinWr, HANDLE_FLAG_INHERIT, 0))
        errorExit("Stdin SetHandleInformation");

    char szCmdline[] = "d2mapapi_piped.exe";
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = childStdoutWr;
    siStartInfo.hStdOutput = childStdoutWr;
    siStartInfo.hStdInput = childStdinRd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcess(nullptr, szCmdline, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &siStartInfo, &piProcInfo);

    if (!bSuccess)
        errorExit("CreateProcess");
    else {
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
        CloseHandle(childStdoutWr);
        CloseHandle(childStdinRd);
    }

    queryMap(10001, 0, 101);
    queryMap(10001, 2, 101);
    CloseHandle(childStdoutRd);
    CloseHandle(childStdinWr);

    return 0;
}

void queryMap(uint32_t seed, uint32_t difficulty, uint32_t levelId) {
    struct Req {
        uint32_t seed;
        uint32_t difficulty;
        uint32_t levelId;
    };
    Req req = {seed, difficulty, levelId};
    DWORD dwWritten;
    WriteFile(childStdinWr, &req, sizeof(uint32_t) * 3, &dwWritten, nullptr);

    DWORD dwRead;
    uint32_t size;
    auto bSuccess = ReadFile(childStdoutRd, &size, sizeof(size), &dwRead, nullptr);
    if (!bSuccess) {
        return;
    }
    std::string str;
    str.resize(size);
    bSuccess = ReadFile(childStdoutRd, str.data(), size, &dwRead, nullptr);
    if (!bSuccess) { return; }
    fprintf(stdout, "%s\n", str.c_str());
}

void errorExit(LPCSTR lpszFunction) {
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf,
        0, nullptr);

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
                                      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40)
                                          * sizeof(char));
    snprintf((LPSTR)lpDisplayBuf,
             LocalSize(lpDisplayBuf),
             "%s failed with error %d: %s",
             lpszFunction, dw, lpMsgBuf);
    MessageBox(nullptr, (LPCTSTR)lpDisplayBuf, "Error", MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}
