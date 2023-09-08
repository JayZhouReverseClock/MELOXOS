ENTRY(start)

SECTIONS {
    . = 0x100000;

    .virk_init_text BLOCK(4K) : {
        * (.multiboot_header)
        build/obj/load_os/*.o (.virk_init)
        build/obj/load_os/*.o (.text)
    }

    .virk_init_bss BLOCK(4K) : {
        build/obj/load_os/*.o (.bss)
    }

    .virk_init_data BLOCK(4k) : {
        build/obj/load_os/*.o (.data)
    }

    .virk_init_rodata BLOCK(4K) : {
        build/obj/load_os/*.o (.rodata)
    }
    __init_phykernel_end = ALIGN(4K);


    . += 0xC0000000;

    .text BLOCK(4K) : AT ( ADDR(.text) - 0xC0000000 ) {
        virkernel_start = .;
        build/obj/kernel/*.o (.text)
    }

    .bss BLOCK(4K) : AT ( ADDR(.bss) - 0xC0000000 ) {
        build/obj/kernel/*.o (.bss)
    }

    .data BLOCK(4k) : AT ( ADDR(.data) - 0xC0000000 ) {
        build/obj/kernel/*.o (.data)
    }

    .rodata BLOCK(4K) : AT ( ADDR(.rodata) - 0xC0000000 ) {
        build/obj/kernel/*.o (.rodata)
    }

    .kernerl_page BLOCK(4K) : AT ( ADDR(.kernerl_page) - 0xC0000000 ) {
        build/obj/load_os/*.o (.kernerl_page)
    }

    virkernel_end = ALIGN(4K);
    __heap_start = ALIGN(4K);    /* the begin of heap */
}