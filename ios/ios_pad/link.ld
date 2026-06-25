OUTPUT_ARCH(arm)

INCLUDE "imports.sym"

EXTERN(
    BTA_DmSearch_hook
    _btrmCustomLibHook
    bta_hh_api_disable
    bta_hh_co_data
    bta_hh_di_sdp_callback
    bta_hh_event
    bta_hh_open_act
    bta_search_callback
    bta_sec_callback
    btm_sec_execute_procedure_hook
    processSmdMessages
    writeDevInfo_hook
)

SECTIONS {
    . = 0x11f86000;

    .text : {
        _text_start = .;
        *(.text*);
        *(.rodata*);
        *(.data*);
    }

    /* the kernel will copy the crypto tables here */
    .crypto : {
        __ans_tbl = .;
        . += 0x2a;
        __sboxes = .;
        . += 0x900;
    }
    _text_end = .;

    . = 0x12159000;

    .bss : {
        _bss_start = .;
        *(.bss*);
    }
    _bss_end = .;

    /DISCARD/ : {
        *(*);
    }
}

/* Only mapping 0x6000 in the kernel for now, make sure this isn't bigger than that */
ASSERT(_text_end - _text_start < 0x6000, "ios_pad text is too big");
ASSERT(_bss_end - _bss_start < 0x6000, "ios_pad bss is too big");
