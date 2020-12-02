# References

## Qemu

https://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/

https://stackoverflow.com/questions/45206027/qemu-aarch64-supported-boards/45235392#45235392

https://github.com/freedomtan/aarch64-bare-metal-qemu

Virt board configuration - https://git.qemu.org/?p=qemu.git;a=blob;f=hw/arm/virt.c

## Semihosting

https://static.docs.arm.com/100863/0200/semihosting.pdf

https://stackoverflow.com/questions/31990487/how-to-cleanly-exit-qemu-after-executing-bare-metal-program-without-user-interve

## GCC

http://cs107e.github.io/guides/gcc/

## Armv7A

Store return state - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0802b/Cihfdedi.html

Return from exception - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489c/Cihjacag.html

Change processor state - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0204j/Cihfdbhd.html

Vector tables - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0425/BABHHDII.html

CPSR - https://www.heyrick.co.uk/armwiki/The_Status_register

Interrupt handlers - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13552.html

## Cortex-M4/Thumb

Cortex-M4 User Guide - http://infocenter.arm.com/help/topic/com.arm.doc.dui0553b/DUI0553.pdf

Article on Cortex-M exception handling - https://www.embeddedrelated.com/showarticle/912.php

ARMv7-M Architecture Manual - https://static.docs.arm.com/ddi0403/ec/DDI0403E_c_armv7m_arm.pdf

## Armv8-A

Armv8-A Reference Manual - https://static.docs.arm.com/ddi0487/da/DDI0487D_a_armv8_arm.pdf

Armv8-A cheat sheet - https://courses.cs.washington.edu/courses/cse469/19wi/arm64.pdf

ARM GIC specification v3.0/4.0 - https://static.docs.arm.com/ihi0069/c/IHI0069C_gic_architecture_specification.pdf

IRQ number - https://patchwork.kernel.org/patch/2487601/ (best I could find, there must be an official source too)

Discussion on correct GIC configuration - https://github.com/takeharukato/sample-tsk-sw/issues/1