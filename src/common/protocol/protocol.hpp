#pragma once
#include <cstddef>
#include <cstdint>

// host <-> コネクタ間のプロトコル定義。通信手段 (TCP / パイプ) は知らない。
//
// 規約:
//   - 固定幅の整数のみ、リトルエンディアン、パディングなし (#pragma pack(1) + static_assert)
//   - 1 メッセージ = Header + 本体 (Header::size バイト)
//   - YMM4 (C#) からも同じ構造体を手で書けるよう、単純な POD にしておく
namespace cm::protocol {

constexpr uint32_t kMagic = 0x4D435443; // バイト列 "CTCM"
constexpr uint32_t kVersion = 1;
constexpr uint32_t kMaxMessageSize = 64u * 1024u * 1024u; // 不正な長さの受信を弾く上限
constexpr size_t kTokenSize = 32;                          // 16 バイトの乱数を 16 進にした文字列

enum class MessageType : uint32_t {
    Hello = 1,   // コネクタ -> host : 接続の挨拶 (版・種類・トークン)
    Welcome = 2, // host -> コネクタ : 挨拶への応答 (結果)
    Ping = 3,    // どちらからでも。往復の確認用 (本体なし)
    Pong = 4,    // Ping への応答 (本体なし)
    Bye = 5,     // 切断の予告 (本体なし)
};

enum class ConnectorKind : uint32_t {
    Unknown = 0,
    AviUtl2 = 1,
    YMM4 = 2,
};

enum class WelcomeResult : uint32_t {
    Ok = 0,
    BadToken = 1,
    VersionMismatch = 2,
};

#pragma pack(push, 1)

struct Header {
    uint32_t magic;     // kMagic
    uint32_t type;      // MessageType
    uint32_t size;      // 続く本体のバイト数
    uint32_t requestId; // 応答は、要求と同じ値を返す
};

struct Hello {
    uint32_t protocolVersion;
    uint32_t connectorKind; // ConnectorKind
    uint32_t processId;
    uint32_t reserved;
    char token[kTokenSize]; // セッションファイルに書かれたトークン (終端の NUL なし)
};

struct Welcome {
    uint32_t protocolVersion;
    uint32_t hostProcessId;
    uint32_t result; // WelcomeResult
    uint32_t reserved;
};

#pragma pack(pop)

static_assert(sizeof(Header) == 16, "Header layout");
static_assert(sizeof(Hello) == 16 + kTokenSize, "Hello layout");
static_assert(sizeof(Welcome) == 16, "Welcome layout");

} // namespace cm::protocol
