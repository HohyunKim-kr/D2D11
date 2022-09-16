// Input.cpp
#include <Windows.h>
#include <stdio.h>

namespace Input
{
    class
    {
    public:
        void Down(WPARAM const code)
        {
            State.Pressed[code >> 0x4] =
                State.Pressed[code >> 0x4] | (0x8000 >> (code & 0xf));
        }

        /*
         'A' == 65 == 0100 , 0001(2)

            State.Pressed[code >> 0x4] =
            State.Pressed[code >> 0x4] | (0x8000 >> (code & 0xf));

            ==
            (code == 0100 , 0001)
            State.Pressed[0100 , 0001 >> 0x4] =
            State.Pressed[0100 , 0001 >> 0x4] | (0x8000 >> (0100 , 0001 & 0xf));

            ==
            ( 0x4 == 4 )
            ( 0xf == 15)
                              10 11 12 13 14 15
            1 2 3 4 5 6 7 8 9 A  B  C  D  E  F
            State.Pressed[0100 , 0001 >> 4] =
            State.Pressed[0100 , 0001 >> 4] | (0x8000 >> (0100 , 0001 & 15));

            ==
            (0100 , 0001 >> 4 == 0000, 0100 == 4)

            State.Pressed[4] =
            State.Pressed[4] | (0x8000 >> (0100 , 0001 & 15));

            ==
            (0100 , 0001 & 15)
               0100 , 0001
             & 0000 , 1111
               0000 , 0001 == 1

            State.Pressed[4] =
            State.Pressed[4] | (0x8000 >> 1);

            ==
            ( 0x8000 == 1000 , 0000 , 0000, 0000 )

            1000 , 0000 , 0000 , 0000 >> 1
            0100 , 0000 , 0000 , 0000

            State.Pressed[4] =
            State.Pressed[4] | (0100 , 0000 , 0000 , 0000);

            0000 , 0000 , 0000 , 0000 == State.Pressed[4]
          | 0100 , 0000 , 0000 , 0000
            0100 , 0000 , 0000 , 0000

            State.Pressed[4] = 0100 , 0000 , 0000 , 0000;
        */

        // State.Pressed[4] = 0100 , 0000 , 0000 , 0000;
        //                    0100 , 0000 , 0000 , 0000
        //                    0000 , 0000 , 0000 , 0000
        void Up(WPARAM const code)
        {
            State.Pressed[code >> 0x4] =
                State.Pressed[code >> 0x4] ^ (0x8000 >> (code & 0xf));
        }

        bool Pressed(WPARAM const code)
        {
            return State.Pressed[code >> 4] & (0x8000 >> (code & 0xf));
        }

        bool Changed(WPARAM const code)
        {

        }

        void Update()
        {
        }

    private:
        struct
        {
            USHORT Pressed[16];
            USHORT Changed[16];
        }State;
    }Key;

    namespace Get
    {
        bool Down(size_t code)
        {
            return Key.Pressed(code);
        }
    }

    void Procedure(HWND const hWindow, UINT const uMessage, WPARAM const wParam, LPARAM const lParam)
    {
        switch (uMessage)
        {
        case WM_KEYDOWN:
            Key.Down(wParam);
            return;
        case WM_KEYUP:
            Key.Up(wParam);
            return;
        default:
            break;
        }
    }
}