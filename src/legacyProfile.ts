export const JET_ACOUSTIC_PROFILE_FIELDS = [
  "idleN1Pct", "idleN1SpanPct", "thrustReferenceLbf", "fuelReferencePps",
  "n1ReferenceHz", "n2ReferenceHz", "fanMix", "fanToneGain", "coreToneGain",
  "fanNoiseBaseGain", "fanNoisePowerGain", "bypassCenterHz", "bypassPowerHz", "bypassGain",
  "jetBaseHz", "jetPowerHz", "jetGain", "combustorHz", "combustorHighHz", "combustorGain",
  "outputGain", "afterburnerJetGain", "afterburnerJetHz",
  "highFineMixGain", "highFineMixHz", "highShockGain", "highShockHz",
  "highRearMixDirectivity", "highShockDirectivity",
  "highCockpitGain", "highCockpitHz",
] as const;

export const PISTON_ACOUSTIC_PROFILE_FIELDS = [
  "cylinders", "strokeCycle", "propellerBlades", "ratedRpm", "ratedPowerHp",
  "firingGain", "propellerGain", "mechanicalGain", "exhaustNoiseGain", "propellerNoiseGain",
  "pistonOutputGain", "pistonCockpitGain", "pistonCockpitHz",
] as const;
/** Shared fixed-size setup transaction. voiceKind selects its typed parameter section. */
export const ACOUSTIC_PROFILE_FIELDS = [
  ...JET_ACOUSTIC_PROFILE_FIELDS, "voiceKind", ...PISTON_ACOUSTIC_PROFILE_FIELDS,
] as const;
export type EngineAcousticProfile = Readonly<Record<typeof JET_ACOUSTIC_PROFILE_FIELDS[number], number>>;
export type PistonAcousticProfile = Readonly<Record<typeof PISTON_ACOUSTIC_PROFILE_FIELDS[number], number>>;

