OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ;
    rodata  PT_LOAD    FLAGS((1 << 2)) ;
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ;
    dynamic PT_DYNAMIC FLAGS((1 << 1) | (1 << 2)) ;
}

SECTIONS
{
    . = 0xffffffff80000000;
    
    text_start_addr = .;

    .text : {
        *(.text .text.*)
        *(.gnu.linkonce.t*)
    } :text
    
    text_end_addr = .;

    . += CONSTANT(MAXPAGESIZE);
    
    rodata_start_addr = .;

    .rodata : {
        start_ctors = .;
        *(SORT(.ctors*))
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)
    } :rodata

    rodata_end_addr = .;

    . += CONSTANT(MAXPAGESIZE);

    data_start_addr = .;

    .data : {
        *(.data .data.*)
    } :data

    .dynamic : {
        *(.dynamic)
    } :data :dynamic

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    data_end_addr = .;

    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
    }
}