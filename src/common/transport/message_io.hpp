#pragma once
#include <cstdint>
#include <vector>
#include "protocol/protocol.hpp"
#include "transport/tcp_socket.hpp"

// Header + 本体、の 1 メッセージを送受信する。
// (TCP は流れなので、受信側は Header の size を見て本体を読み切る)
// 名前に Send / Read を使わないのは、Windows のマクロ (SendMessage など) との衝突を避けるため。
namespace cm::net {

// 1 本の接続に対する書き込みは、同時に 2 スレッドから行わないこと。
inline bool WriteMessage(TcpSocket& sock, protocol::MessageType type, const void* body, uint32_t size,
                         uint32_t requestId = 0)
{
    const protocol::Header header{protocol::kMagic, static_cast<uint32_t>(type), size, requestId};
    if (!sock.SendAll(&header, sizeof(header)))
        return false;
    return size == 0 || sock.SendAll(body, size);
}

// 不正なメッセージ (magic 違い、大きすぎる size) と切断は false を返す。
inline bool ReadMessage(TcpSocket& sock, protocol::Header& header, std::vector<uint8_t>& body)
{
    if (!sock.RecvExact(&header, sizeof(header)))
        return false;
    if (header.magic != protocol::kMagic || header.size > protocol::kMaxMessageSize)
        return false;
    body.resize(header.size);
    return header.size == 0 || sock.RecvExact(body.data(), header.size);
}

} // namespace cm::net
