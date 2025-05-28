#pragma once
#include <cstdint>

namespace micropather
{
    const int8_t OffsetCoordinates[4][4][8] = {
        // HEX_ODDR
        { {
          // HEX_ODDR EVEN X
          1,  // E -
          0,  // N
          -1, // W -
          0,  // S
          0,  // NE
          -1, // NW
          -1, // SW
          0   // SE
          },
          {
          // HEX_ODDR EVEN Y
          0,  // E -
          1,  // N
          0,  // W -
          -1, // S
          -1, // NE
          -1, // NW
          1,  // SW
          1   // SE
          },
          {
          // HEX_ODDR ODD X
          1,  // E -
          0,  // N
          -1, // W -
          1,  // S
          1,  // NE
          0,  // NW
          0,  // SW
          1   // SE
          },
          {
          // HEX_ODDR ODD Y
          0,  // E
          1,  // N
          0,  // W
          -1, // S
          -1, // NE
          -1, // NW
          1,  // SW
          1   // SE
          } },

        // HEX_EVENR
        { {
          // HEX_EVENR EVEN X
          1,  // E -
          0,  // N
          -1, // W -
          1,  // S
          1,  // NE
          0,  // NW
          0,  // SW
          1   // SE
          },
          {
          // HEX_EVENR EVEN Y
          0,  // E -
          1,  // N
          0,  // W -
          -1, // S
          -1, // NE
          -1, // NW
          1,  // SW
          1   // SE
          },
          {
          // HEX_EVENR ODD X

          1,  // E -
          0,  // N
          -1, // W -
          0,  // S
          0,  // NE
          -1, // NW
          -1, // SW
          0   // SE
          },
          {
          // HEX_EVENR ODD Y
          0,  // E -
          1,  // N
          0,  // W -
          -1, // S
          -1, // NE
          -1, // NW
          1,  // SW
          1   // SE
          } },

        // HEX_ODDQ
        {

        {
        // HEX_ODDR ODD X
        1,  // E
        0,  // N
        -1, // W
        0,  // S
        1,  // NE
        0,  // NW
        -1, // SW
        0   // SE
        },
        {
        // HEX_ODDR ODD Y
        1,  // E
        1,  // N
        0,  // W
        -1, // S
        0,  // NE
        -1, // NW
        1,  // SW
        1   // SE
        },

        {
        // HEX_ODDR EVEN X

        1,  // E
        0,  // N
        -1, // W
        1,  // S
        1,  // NE
        0,  // NW
        -1, // SW
        0   // SE
        },
        {
        // HEX_ODDR EVEN Y
        0,  // E
        1,  // N
        -1, // W
        -1, // S
        -1, // NE
        -1, // NW
        0,  // SW
        1   // SE
        }

        },

        // HEX_EVENQ
        {

        {
        // HEX_ODDR EVEN X

        1,  // E
        0,  // N
        -1, // W
        1,  // S
        1,  // NE
        0,  // NW
        -1, // SW
        0   // SE
        },
        {
        // HEX_ODDR EVEN Y
        0,  // E
        1,  // N
        -1, // W
        -1, // S
        -1, // NE
        -1, // NW
        0,  // SW
        1   // SE
        },
        {
        // HEX_ODDR ODD X
        1,  // E
        0,  // N
        -1, // W
        0,  // S
        1,  // NE
        0,  // NW
        -1, // SW
        0   // SE
        },
        {
        // HEX_ODDR ODD Y
        1,  // E
        1,  // N
        0,  // W
        -1, // S
        0,  // NE
        -1, // NW
        1,  // SW
        1   // SE
        }

        },
    };

}; // namespace micropather