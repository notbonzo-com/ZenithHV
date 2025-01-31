//
// Created by notbonzo on 1/30/25.
//

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define DEBUG
#define DEBUG_VM 1
#if 0
    #define DEBUG_PRINTF
#endif

#ifdef DEBUG
    #if DEBUG_VM == 1 /* QEMU */
        #define VM_SHUTDOWN_PORT 0x604
        #define VM_SHUTDOWN_MAGIC 0x2000
    #elif DEBUG_VM == 2 /* VirtualBox */
        #define VM_SHUTDOWN_PORT 0x4004
        #define VM_SHUTDOWN_MAGIC 0x3400
    #endif
#endif

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define DIV_ROUNDUP(x, y) (((x) + (y) - 1) / (y))
#define DIV_ROUNDDOWN(x, y) ((x) / (y))

#define ALIGN_UP(x, base) (((x) + (base) - 1) & ~((base) - 1))
#define ALIGN_DOWN(x, base) ((x) & ~((base) - 1))

#endif //CONSTANTS_H
