#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

// セッションファイル: host が起動時に書き、コネクタが読んで接続先を知るための小さなテキスト。
//
//   port=49321
//   token=0123456789abcdef0123456789abcdef
//   pid=1234
//
// Wine 内のコネクタとネイティブ host など、プロセス間でファイルシステムだけが共通の場合にも使える。
// 置き場所は Windows では %TEMP%\cometsuite\session.txt。
// (Linux / macOS 対応時は /tmp/cometsuite/session.txt を想定し、Wine からは Z:\tmp\cometsuite を参照する)
namespace cm::protocol {

struct SessionInfo {
    uint16_t port = 0;
    std::string token;
    uint32_t pid = 0;
};

inline std::filesystem::path SessionFilePath()
{
#ifdef _WIN32
    wchar_t buf[MAX_PATH + 1] = {};
    const DWORD n = GetTempPathW(MAX_PATH, buf);
    return std::filesystem::path(std::wstring(buf, n)) / L"cometsuite" / L"session.txt";
#else
    return std::filesystem::path("/tmp/cometsuite/session.txt");
#endif
}

// 一時ファイルに書いてから置き換える (読み手が書きかけを見ないようにする)
inline bool WriteSession(const SessionInfo& info)
{
    const auto path = SessionFilePath();
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    auto tmp = path;
    tmp += L".tmp";
    {
        std::ofstream f(tmp, std::ios::trunc);
        if (!f)
            return false;
        f << "port=" << info.port << "\ntoken=" << info.token << "\npid=" << info.pid << "\n";
        if (!f)
            return false;
    }
    std::filesystem::rename(tmp, path, ec);
    return !ec;
}

// 読めて、内容が妥当なら true。host が起動していない / 書きかけのときは false。
inline bool ReadSession(SessionInfo& out)
{
    std::ifstream f(SessionFilePath());
    if (!f)
        return false;

    SessionInfo info;
    std::string line;
    try {
        while (std::getline(f, line)) {
            const auto eq = line.find('=');
            if (eq == std::string::npos)
                continue;
            const std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);
            while (!value.empty() && (value.back() == '\r' || value.back() == '\n'))
                value.pop_back();
            if (key == "port")
                info.port = static_cast<uint16_t>(std::stoul(value));
            else if (key == "token")
                info.token = value;
            else if (key == "pid")
                info.pid = static_cast<uint32_t>(std::stoul(value));
        }
    } catch (...) {
        return false;
    }
    if (info.port == 0 || info.token.size() != 32)
        return false;
    out = info;
    return true;
}

// 自分 (pid) が書いたセッションファイルだけを消す。後から起動した別の host のものは消さない。
inline void RemoveSession(uint32_t pid)
{
    SessionInfo info;
    if (ReadSession(info) && info.pid == pid) {
        std::error_code ec;
        std::filesystem::remove(SessionFilePath(), ec);
    }
}

} // namespace cm::protocol
