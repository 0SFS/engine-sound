/** Versioned allocation-free block contract shared by family WASM modules and the host. */
export const ENGINE_VOICE_ABI = "osfs-engine-voice-v1";
export const ENGINE_VOICE_ABI_VERSION = 1;
export const VOICE_MAX_SOURCES = 4;
export const VOICE_MAX_FRAMES = 1024;
export const VOICE_INPUT_FIELDS = ["n1Pct", "n2Pct", "thrustLbf", "fuelFlowPps", "combustion", "engineRpm", "propellerRpm", "shaftPowerHp", "kias", "gearNorm", "flapNorm", "afterburnerFraction", "afterburnerVolume", "cullScale", "downstreamCosine", "directionWeight", "rotorRpm", "thrustN", "torqueNm", "inducedVelocityMps", "vrs", "airDensityKgM3", "axialVelocityMps", "armed", "tier", "partials", "noiseBands", "bypassBand", "highFidelity", "resetCounter", "valid", "airframeOwner"] as const;
export const VOICE_INPUT = Object.freeze(Object.fromEntries(VOICE_INPUT_FIELDS.map((name, index) => [name, index]))) as Readonly<Record<typeof VOICE_INPUT_FIELDS[number], number>>;
export interface EngineVoiceExports {
 memory: WebAssembly.Memory;
 _initialize?: () => void;
 osfs_voice_abi_version(): number;
 osfs_voice_init(sampleRate: number, maxBlock: number, seed: number, voiceCount: number): number;
 osfs_voice_reset(): void;
 osfs_voice_input_ptr(): number;
 osfs_voice_output_ptr(): number;
 osfs_voice_profile_ptr(voice: number): number;
 osfs_voice_profile_size(): number;
 osfs_voice_commit_profile(voice: number): number;
 osfs_voice_process(frames: number): number;
}
