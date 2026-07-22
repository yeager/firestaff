#ifndef REDMCSB_BYTEOPS_PC34_COMPAT_H
#define REDMCSB_BYTEOPS_PC34_COMPAT_H

/* ReDMCSB CPCLRBYT.C/COPYBYTE.C portable byte primitives. */
void F0007_MAIN_CopyBytes(char *source, char *destination, long byte_count);
void F0008_MAIN_ClearBytes(char *buffer, unsigned long byte_count);

#endif
