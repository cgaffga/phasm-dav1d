/*
 * Copyright © 2019, VideoLAN and dav1d authors
 * Copyright © 2019, Two Orioles, LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DAV1D_SRC_ARM_MSAC_H
#define DAV1D_SRC_ARM_MSAC_H

unsigned dav1d_msac_decode_symbol_adapt4_neon(MsacContext *s, uint16_t *cdf,
                                              size_t n_symbols);
unsigned dav1d_msac_decode_symbol_adapt8_neon(MsacContext *s, uint16_t *cdf,
                                              size_t n_symbols);
unsigned dav1d_msac_decode_symbol_adapt16_neon(MsacContext *s, uint16_t *cdf,
                                               size_t n_symbols);
unsigned dav1d_msac_decode_hi_tok_neon(MsacContext *s, uint16_t *cdf);
unsigned dav1d_msac_decode_bool_adapt_neon(MsacContext *s, uint16_t *cdf);
unsigned dav1d_msac_decode_bool_equi_neon(MsacContext *s);
unsigned dav1d_msac_decode_bool_neon(MsacContext *s, unsigned f);

#if defined(__ARM_NEON) || defined(__APPLE__) || defined(_WIN32) || ARCH_AARCH64
#define dav1d_msac_decode_symbol_adapt4  dav1d_msac_decode_symbol_adapt4_neon
#define dav1d_msac_decode_symbol_adapt8  dav1d_msac_decode_symbol_adapt8_neon
#define dav1d_msac_decode_symbol_adapt16 dav1d_msac_decode_symbol_adapt16_neon
#define dav1d_msac_decode_hi_tok         dav1d_msac_decode_hi_tok_neon
#define dav1d_msac_decode_bool_adapt     dav1d_msac_decode_bool_adapt_neon
#define dav1d_msac_decode_bool           dav1d_msac_decode_bool_neon

/* phasm-stego (W3.D.2.3): NEON-path wrapper around the asm variant
 * of dav1d_msac_decode_bool_equi. Calls the NEON asm, then fires the
 * phasm-stego per-bit hook with the decoded value + current channel
 * tag. NULL bit_hook = no-op = byte-identical decode timing on the
 * hot path (single conditional branch on a likely-cached pointer).
 *
 * Mirror of the hook insertion in src/msac.c::dav1d_msac_decode_bool_equi_c.
 * Required for ARM macOS / iOS / Android builds because the C variant
 * is compiled out via #if !(HAVE_ASM && TRIM_DSP_FUNCTIONS &&
 * ARCH_AARCH64) — NEON is the runtime decoder.
 *
 * The wrapper is given a distinct name + remapped via #define so
 * src/msac.h's `#ifndef dav1d_msac_decode_bool_equi` guard sees the
 * macro as set (otherwise it falls through to the _c default and
 * the linker fails on ARM-asm builds where _c is compiled out).
 *
 * See phasm-av1/docs/design/video/av1/dav1d-hook-sites.md § 4.
 */
static inline unsigned phasm_dav1d_msac_decode_bool_equi_neon_wrapper(
    MsacContext *const s)
{
    const unsigned out = dav1d_msac_decode_bool_equi_neon(s);
    if (s->phasm_hooks.bit_hook) {
        s->phasm_hooks.bit_hook(s->phasm_hooks.cookie, out,
                                s->phasm_current_tag);
    }
    return out;
}
#define dav1d_msac_decode_bool_equi      phasm_dav1d_msac_decode_bool_equi_neon_wrapper
#endif

#endif /* DAV1D_SRC_ARM_MSAC_H */
