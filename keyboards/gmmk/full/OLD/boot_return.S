                .cpu    cortex-m0
                .fpu    softvfp
                .syntax unified
                .thumb
                .section .bootreturn,"ax"

                .align  2
                .thumb_func
                .global __boot_return
__boot_return:
                b _crt0_entry

