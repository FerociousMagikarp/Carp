#pragma once

#include <type_traits>
#include <utility>

namespace Carp
{

enum class PieceType : std::uint8_t
{
    King,      // 帅，将
    Advisor,   // 士
    Elephant,  // 相，象
    Knight,    // 马
    Rook,      // 车
    Cannon,    // 炮
    Pawn,      // 兵，卒
};

enum class PlayerType : std::uint8_t
{
    Red   = 0x00,
    Black = 0x08,
};

enum class PlayerPieceType : std::uint8_t
{
    RedKing = 0,
    RedAdvisor,
    RedElephant,
    RedKnight,
    RedRook,
    RedCannon,
    RedPawn,
    BlackKing = 8,
    BlackAdvisor,
    BlackElephant,
    BlackKnight,
    BlackRook,
    BlackCannon,
    BlackPawn,
};

constexpr PlayerPieceType ComposePlayerPiece(PlayerType player, PieceType piece)
{
    auto piece_val  = static_cast<std::underlying_type_t<PieceType>>(piece);
    auto player_val = static_cast<std::underlying_type_t<PlayerType>>(player);
    return static_cast<PlayerPieceType>(player_val | piece_val);
}

constexpr std::pair<PlayerType, PieceType> DeComposePlayerPiece(PlayerPieceType player_piece)
{
    auto player_piece_val = static_cast<std::underlying_type_t<PlayerPieceType>>(player_piece);
    auto player = static_cast<PlayerType>(player_piece_val & 0x08);
    auto piece = static_cast<PieceType>(player_piece_val & 0x07);
    return std::make_pair(player, piece);
}

} // namespace Carp
