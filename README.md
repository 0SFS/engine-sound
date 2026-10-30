# Engine sound

Shared contracts and DSP primitives for the independently compiled `turbine-sound`, `piston-sound` and `electric-sound` generators. Engine mods supply their hardware parameters and acoustic profiles; this package contains no engine presets.

The `sound` package owns the flight's AudioContext, worklet, telemetry clock, source placement, propagation, tire cue and final limiter. Each family module runs inside that worklet and returns source audio. The worklet performs one host prepare, one family render and one host mix per audio block. Four electric rotors retain separate source signals and placement.

## Block interface

`@0sfs/engine-sound/voiceAbi` exports the TypeScript contract and field names. `src/voice_abi.h` defines the same numeric slots for the legacy C++ family wrappers. The ABI identifier is `osfs-engine-voice-v1`, version `1`.

Modules export their memory, initialization/reset functions, profile buffers and one `osfs_voice_process(frames)` function. Buffers have a fixed capacity of four sources and 1024 frames. Inputs use 32 Float64 fields per source and frame; outputs have separate Float64 source and airframe planes. The host selects one airframe owner so multiple motors do not duplicate aircraft wind noise.

The host resolves timestamped telemetry, availability and events. Family generators own their parameter validation, normalization, smoothing, oscillator phases and noise state. Per-frame tier, detail limits and reset counters preserve transitions inside an audio block. Source synthesis uses unshifted frequencies; the host applies Doppler once through propagation, while the family receives its ratio only for alias prevention.

Initialization and profile setup may allocate. Rendering must not allocate, grow memory, wait, fetch, or call JavaScript. Family modules have no imports. A missing or incompatible module prevents the engine installation from starting; the tire cue can still run independently.

## Legacy extraction helpers

`legacyProfile`, `legacy_profile.h`, `voice_caps.h` and `legacy_voice_module.h` retain the existing piston/turbine profile layout and numerical behavior. They are compatibility support for those extracted kernels. New electric synthesis is Rust/WebAssembly with a TypeScript reference in `electric-sound`.

`@0sfs/engine-sound/family-artifact` provides the build and verification helper used by the two legacy family packages. It resolves shared headers from this installed package and records logical package source paths, so the family checkout need not be beside this checkout. Engine-specific setup data stays in the engine mod manifests.

From each legacy family package, run `npm install` followed by `npm run build:wasm` with Emscripten 6.0.9 installed. `npm run verify:wasm` checks source hashes, artifact and manifest hashes, imports, exports and ABI version. From `fly`, `npm run verify:audio` checks the host and all three family artifacts together.

## Community

- [Code of conduct](https://github.com/0SFS/.github/blob/main/CODE_OF_CONDUCT.md)
- [Contributing](https://github.com/0SFS/.github/blob/main/CONTRIBUTING.md)
- [License](LICENSE)
- [Security](https://github.com/0SFS/.github/blob/main/SECURITY.md)
