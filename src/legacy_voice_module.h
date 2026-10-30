#pragma once

#include "voice_abi.h"
#include "legacy_profile.h"
#include "voice_caps.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define VOICE_EXPORT extern "C" EMSCRIPTEN_KEEPALIVE
#else
#define VOICE_EXPORT extern "C"
#endif

namespace osfs_audio {

static double voiceInput[kVoiceMaxSources * kVoiceMaxFrames * kVoiceInputStride];
static double voiceOutput[kVoiceMaxSources * 2 * kVoiceMaxFrames];
static AcousticProfile voiceProfiles[kVoiceMaxSources];

struct LegacyVoiceState {
  FAMILY_VOICE voice;
  Smoother smooth[kVoiceInputStride];
  int resetCounter = -1;

  void reset() {
    voice.reset();
    for (auto& smoother : smooth) smoother.reset(0.0);
    smooth[kVoiceAfterburnerVolume].reset(1.0);
  }
};

static LegacyVoiceState voiceStates[kVoiceMaxSources];
static double voiceSampleRate = 48000;
static int voiceCount = 0;
static int voiceMaxBlock = 128;

}  // namespace osfs_audio

VOICE_EXPORT int osfs_voice_abi_version() { return 1; }

VOICE_EXPORT int osfs_voice_init(double rate, int block, unsigned seed, int count) {
  using namespace osfs_audio;
  if (!std::isfinite(rate) || rate < 8000 || rate > 96000
      || block < 1 || block > kVoiceMaxFrames || count < 1 || count > kVoiceMaxSources) return 0;
  voiceSampleRate = rate;
  voiceCount = count;
  voiceMaxBlock = block;
  for (int voice = 0; voice < count; ++voice) {
    auto& state = voiceStates[voice];
    state.voice.configure(rate, voice ? seed ^ (0x9e3779b9u * voice) : seed);
    for (auto& smoother : state.smooth) smoother.configure(rate, 0.010);
    state.reset();
    state.resetCounter = -1;
  }
  return 1;
}

VOICE_EXPORT void osfs_voice_reset() {
  using namespace osfs_audio;
  for (int voice = 0; voice < voiceCount; ++voice) {
    voiceStates[voice].reset();
    voiceStates[voice].resetCounter = -1;
  }
}

VOICE_EXPORT double* osfs_voice_input_ptr() { return osfs_audio::voiceInput; }
VOICE_EXPORT double* osfs_voice_output_ptr() { return osfs_audio::voiceOutput; }

VOICE_EXPORT double* osfs_voice_profile_ptr(int voice) {
  using namespace osfs_audio;
  return voice >= 0 && voice < kVoiceMaxSources ? voiceProfiles[voice].values : nullptr;
}

VOICE_EXPORT int osfs_voice_profile_size() { return osfs_audio::kProfileSize; }

VOICE_EXPORT int osfs_voice_commit_profile(int voice) {
  using namespace osfs_audio;
  if (voice < 0 || voice >= voiceCount) return 0;
  return voiceStates[voice].voice.setProfile(voiceProfiles[voice].values) ? 1 : 0;
}

VOICE_EXPORT int osfs_voice_process(int frames) {
  using namespace osfs_audio;
  if (frames < 0 || frames > voiceMaxBlock) return 0;

  for (int voice = 0; voice < voiceCount; ++voice) {
    auto& state = voiceStates[voice];
    const auto& profile = voiceProfiles[voice];
    for (int frame = 0; frame < frames; ++frame) {
      const double* row = voiceInput + (voice * kVoiceMaxFrames + frame) * kVoiceInputStride;
      double* engine = voiceOutput + (voice * 2) * kVoiceMaxFrames + frame;
      double* airframe = voiceOutput + (voice * 2 + 1) * kVoiceMaxFrames + frame;
      const int reset = static_cast<int>(row[kVoiceResetCounter]);
      if (reset != state.resetCounter) {
        state.reset();
        state.smooth[kVoiceAfterburnerVolume].reset(row[kVoiceAfterburnerVolume]);
        state.resetCounter = reset;
      }
      if (row[kVoiceTier] <= 0) {
        *engine = 0;
        *airframe = 0;
        continue;
      }

      EngineCaps caps;
      caps.partials = static_cast<int>(clamp(row[kVoicePartials], 0, 12));
      caps.noiseBands = static_cast<int>(clamp(row[kVoiceNoiseBands], 0, 8));
      caps.bypassBand = row[kVoiceBypassBand] > 0;
      caps.highFidelity = row[kVoiceHighFidelity] > 0;
      state.voice.setCaps(caps);
      const bool valid = row[kVoiceValid] > 0;
      const auto smooth = [&](int slot, double target) {
        return state.smooth[slot].process(valid ? target : 0.0);
      };
      const double kias = smooth(kVoiceKias, std::fmax(0.0, row[kVoiceKias]));
      const double gear = smooth(kVoiceGearNorm, clamp01(row[kVoiceGearNorm]));
      const double flap = smooth(kVoiceFlapNorm, clamp01(row[kVoiceFlapNorm]));
      const double combustion = smooth(kVoiceCombustion, clamp01(row[kVoiceCombustion]));

      FAMILY_INPUT input;
#ifdef PISTON_FAMILY
      input.engineRpm = smooth(kVoiceEngineRpm, clamp(row[kVoiceEngineRpm], 0, 10000));
      input.propellerRpm = smooth(kVoicePropellerRpm, clamp(row[kVoicePropellerRpm], 0, 10000));
      input.powerHp = smooth(kVoiceShaftPowerHp, std::fmax(0.0, row[kVoiceShaftPowerHp]));
#else
      input.n1 = smooth(kVoiceN1Pct, clamp01(row[kVoiceN1Pct] / 100.0));
      input.n2 = smooth(kVoiceN2Pct, clamp01(row[kVoiceN2Pct] / 100.0));
      input.thrustNorm = smooth(kVoiceThrustLbf,
          clamp(row[kVoiceThrustLbf] / state.voice.thrustReference(), 0, state.voice.thrustCeiling()));
      input.fuelNorm = smooth(kVoiceFuelFlowPps, clamp01(row[kVoiceFuelFlowPps] / state.voice.fuelReference()));
      input.afterburnerFraction = smooth(kVoiceAfterburnerFraction, clamp01(row[kVoiceAfterburnerFraction]));
      input.afterburnerVolume = state.smooth[kVoiceAfterburnerVolume].process(row[kVoiceAfterburnerVolume]);
      input.downstreamCosine = row[kVoiceDownstreamCosine];
      input.directionWeight = row[kVoiceDirectionWeight];
#endif
      input.combustion = combustion;
      input.kias = kias;
      input.gear = gear;
      input.flap = flap;
      *airframe = 0;
      *engine = state.voice.process(input, row[kVoiceCullScale], airframe);
      if (row[kVoiceAirframeOwner] <= 0) *airframe = 0;
    }
  }
  return 1;
}
