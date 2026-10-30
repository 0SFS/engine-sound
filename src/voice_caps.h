#pragma once
namespace osfs_audio {
/** Per-tier source counts. These are the caps sound.md §1 budgets. */
struct EngineCaps {
  int partials = 0;      // engine oscillators, excluding the tire squeal
  int noiseBands = 0;    // engine/airframe noise sources, excluding the tire
  bool bypassBand = false;
  bool highFidelity = false;
};


constexpr int kMaxPartials = 12;
}
