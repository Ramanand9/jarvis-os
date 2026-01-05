#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

// ---------- Debug output (QEMU port 0xE9) ----------
// This is a *debug-only* trick: QEMU can redirect writes to I/O port 0xE9
// to your terminal if run with: -debugcon stdio
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static void dbg_putc(char c) { outb(0xE9, (uint8_t)c); }
static void dbg_puts(const char* s) { while (*s) dbg_putc(*s++); }

// ---------- Limine requests ----------
// Limine discovers what your kernel wants by scanning special ELF sections.
// These globals MUST NOT be optimized away (hence used/volatile/section).

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = nullptr
};

// Markers so Limine can find the request list range
__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// Halt-and-catch-fire: stop the CPU cleanly
static __attribute__((noreturn)) void hcf() {
    for (;;) asm volatile ("hlt");
}

extern "C" void kmain(void) {
    dbg_puts("[K] kmain entered\r\n");

    // Ensure bootloader supports the base revision we require
    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)) {
        dbg_puts("[K] Limine base revision not supported\r\n");
        hcf();
    }

    // Ensure we got a framebuffer
    if (!fb_req.response || fb_req.response->framebuffer_count < 1) {
        dbg_puts("[K] No framebuffer\r\n");
        hcf();
    }

    limine_framebuffer* fb = fb_req.response->framebuffers[0];
    dbg_puts("[K] Got framebuffer\r\n");

    // Draw a short diagonal line (32-bit pixels assumed)
    volatile uint32_t* px = (volatile uint32_t*)fb->address;
    size_t pitch32 = fb->pitch / 4;
    for (size_t i = 0; i < 200; i++) {
        px[i * pitch32 + i] = 0x00FFFFFF;
    }

    dbg_puts("[K] Drew pixels. Halting.\r\n");
    hcf();
}
