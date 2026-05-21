# phasm-dav1d — dav1d fork with AV1 steganography hooks

This is a fork of [dav1d](https://code.videolan.org/videolan/dav1d)
(VideoLAN's AV1 decoder, BSD-2-Clause) with a custom hook surface that
lets external callers observe and influence the bypass-bin entropy
stream during AV1 decoding. Paired with
[phasm-rav1e](https://github.com/cgaffga/phasm-rav1e) (the encoder
side), it powers AV1 video steganography in
[phasm.app](https://phasm.app).

The unmodified upstream decoder behavior is preserved; the stego hooks
are additive and inert unless callbacks are registered. If you're
looking for the upstream codec for general-purpose AV1 decoding, go to
[code.videolan.org/videolan/dav1d](https://code.videolan.org/videolan/dav1d) —
this fork is not a general-purpose codec replacement.

## Status

- **W3.D Phase A SHIPPED 2026-05-21.** Hook surface added at the four
  bypass-bin emission sites (AC sign + golomb tag site patches),
  matched against [phasm-rav1e](https://github.com/cgaffga/phasm-rav1e)'s
  encoder-side WriterStego hooks. End-to-end rav1e → dav1d AV1
  steganography round-trip verified on a 3-fixture real-content
  corpus (`av1_corpus_validation` test in phasm-core); cross-arch
  determinism verified under Rosetta 2 (W7).
- **NEON + SSE2 asm wrappers** for the bit hook (`W3.D.2.3`) so the
  optimized assembly paths stay on the production wire.
- **Pinned by SHA** in
  [`core/dav1d-sys/build.rs`](https://github.com/cgaffga/phasmcore/blob/main/dav1d-sys/build.rs)
  inside the [phasm-core](https://github.com/cgaffga/phasmcore) crate.
  Branch `phasm-stego`.
- **EXPERIMENTAL.** Phase A is the first end-to-end ship; Phase B
  (production gates + capacity tuning) is in progress.

## What this fork adds

A small hook surface in the bypass-bin path (`src/msac.c` /
`src/getbits.c` and the per-tile entropy decoders). Stego callbacks
fire AFTER each bypass-bin emission, so a downstream
[phasm-core](https://github.com/cgaffga/phasmcore) consumer can replay
the canonical cover-position stream produced by phasm-rav1e on the
encode side, recover the embedded LSBs, and reconstruct the original
plaintext.

Build-time stub fallback: if you build phasm-core without the
`vendor/phasm-dav1d` submodule checked out, `dav1d-sys/build.rs` links
against a stub that returns sentinel values. The Cargo `av1-backend`
feature still gates the code paths, so default-features consumers
never touch the dav1d link.

## License

dav1d is BSD-2-Clause (very liberal — embed anywhere). The phasm-stego
hooks added by this fork are also BSD-2-Clause. There is no patent-pool
constraint on AV1 distribution because AV1 itself is royalty-free per
the [AOM AV1 patent license](https://aomedia.org/license/patent-license/) —
unlike H.264 / AVC.

## Related

- [phasm-core](https://github.com/cgaffga/phasmcore) — the pure-Rust
  steganography engine that consumes this fork via `av1-backend`.
- [phasm-rav1e](https://github.com/cgaffga/phasm-rav1e) — the matched
  encoder fork (rav1e + WriterStego hooks).
- [phasm-openh264](https://github.com/cgaffga/phasm-openh264) — the
  H.264 sibling fork (already at v1.0 production in phasm-core).
- [phasm.app](https://phasm.app) — the end-user steganography app.

---

For the upstream dav1d build instructions / contribution guidelines /
codec details, see the [upstream README](https://code.videolan.org/videolan/dav1d/-/blob/master/README.md).
This fork-specific README is short by design; the bulk of the work
lives in the
[design doc](https://github.com/cgaffga/phasmcore/blob/main/docs/design/video/av1/dav1d-hook-sites.md)
in the phasm-core repo.
