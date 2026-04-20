#ifndef COMPILE_H
#include "COMPILE.H"
#endif

/*
 Minimal modern C compatibility replacements for the tiny PC byteops layer.
 These are not cycle-accurate Turbo C asm ports, but they preserve the basic
 byte semantics needed for syntax probing and early staged port work.
*/

void F0007_MAIN_CopyBytes(
char*   P0005_pc_Source      SEPARATOR
char*   P0006_pc_Destination SEPARATOR
long    P0007_l_ByteCount
)
{
        unsigned char* L0001_puc_Source;
        unsigned char* L0002_puc_Destination;


        if ((P0005_pc_Source == NULL) || (P0006_pc_Destination == NULL) || (P0007_l_ByteCount <= 0)) {
                return;
        }
        L0001_puc_Source = (unsigned char*)P0005_pc_Source;
        L0002_puc_Destination = (unsigned char*)P0006_pc_Destination;
        if (L0001_puc_Source > L0002_puc_Destination) {
                while (P0007_l_ByteCount-- > 0) {
                        *L0002_puc_Destination++ = *L0001_puc_Source++;
                }
        } else {
                L0001_puc_Source += P0007_l_ByteCount;
                L0002_puc_Destination += P0007_l_ByteCount;
                while (P0007_l_ByteCount-- > 0) {
                        *--L0002_puc_Destination = *--L0001_puc_Source;
                }
        }
}

void F0008_MAIN_ClearBytes(
char HUGE*    P0008_pc_Buffer   SEPARATOR
unsigned long P0009_i_ByteCount
)
{
        unsigned char HUGE* L0003_puc_Buffer;


        if ((P0008_pc_Buffer == NULL) || (P0009_i_ByteCount == 0)) {
                return;
        }
        L0003_puc_Buffer = (unsigned char HUGE*)P0008_pc_Buffer;
        while (P0009_i_ByteCount-- > 0) {
                *L0003_puc_Buffer++ = 0;
        }
}
