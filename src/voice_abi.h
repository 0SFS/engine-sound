#pragma once
namespace osfs_audio {
constexpr int kVoiceMaxSources=4, kVoiceMaxFrames=1024, kVoiceInputStride=32;
enum VoiceInputSlot {
 kVoiceN1Pct=0,
 kVoiceN2Pct=1,
 kVoiceThrustLbf=2,
 kVoiceFuelFlowPps=3,
 kVoiceCombustion=4,
 kVoiceEngineRpm=5,
 kVoicePropellerRpm=6,
 kVoiceShaftPowerHp=7,
 kVoiceKias=8,
 kVoiceGearNorm=9,
 kVoiceFlapNorm=10,
 kVoiceAfterburnerFraction=11,
 kVoiceAfterburnerVolume=12,
 kVoiceCullScale=13,
 kVoiceDownstreamCosine=14,
 kVoiceDirectionWeight=15,
 kVoiceRotorRpm=16,
 kVoiceThrustN=17,
 kVoiceTorqueNm=18,
 kVoiceInducedVelocityMps=19,
 kVoiceVrs=20,
 kVoiceAirDensityKgM3=21,
 kVoiceAxialVelocityMps=22,
 kVoiceArmed=23,
 kVoiceTier=24,
 kVoicePartials=25,
 kVoiceNoiseBands=26,
 kVoiceBypassBand=27,
 kVoiceHighFidelity=28,
 kVoiceResetCounter=29,
 kVoiceValid=30,
 kVoiceAirframeOwner=31
};
}
