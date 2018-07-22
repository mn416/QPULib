#ifdef QPU_MODE

#ifndef _QPULIB_PERFORMANCECOUNTERS_H
#define _QPULIB_PERFORMANCECOUNTERS_H
#include <string.h>
#include "RegisterMap.h"


namespace QPULib {

class PerformanceCounters {
 public:
	enum {
		SLOT_COUNT = 16,
  	ALL_COUNTERS = (1 << SLOT_COUNT) - 1
	};


	/**
	 * Descriptive enums for the counter indexes.
	 *
	 * These are made up and not part of the reference doc.
	 * Derived from the counter descriptions.
	 */
	enum Index {
		FEP_NO_RENDERED_PIXELS,
		FEP_VALID_PRIMITIVES,
		FEP_CLIPPED_QUADS,
		FEP_VALID_QUADS,
		TLB_ALL_FAILING_STENCIL,
		TLB_ALL_FAILING_Z_AND_STENCIL,
		TLB_ANY_FAILING_Z_AND_STENCIL,
		TLB_ALL_ZERO_COVERAGE,
		TLB_ANY_NONZERO_COVERAGE,
		TLB_VALID_WRITTEN,
		PTB_OUTSIDE_VIEWPORT,
		PTB_NEED_CLIPPING,
		PSE_DISCARDED_REVERSED,
		QPU_IDLE,
		QPU_VERTEX_SHADING,
		QPU_FRAGMENT_SHADING,
		QPU_INSTRUCTIONS,
		QPU_STALLED_TMU,
		QPU_STALLED_SCOREBOARD,
		QPU_STALLED_VARYINGS,
		QPU_INSTRUCTION_CACHE_HITS,
		QPU_INSTRUCTION_CACHE_MISSES,
		QPU_CACHE_HITS,
		QPU_CACHE_MISSES,
		TMU_QUADS_PROCESSED,
		TMU_CACHE_MISSES,
		VPM_VDW_STALLED,
		VPM_VCD_STALLED,
		L2C_CACHE_HITS,
		L2C_CACHE_MISSES,

		NUM_PERF_COUNTERS,  // Should be 30
		END_MARKER
	};


	// Convenience struct to facilitate enabling of timers
	struct Init {
		uint32_t slotIndex;
		Index    counterIndex;

		bool isEnd() const { return (slotIndex == END_MARKER); }
	};


  static void clear(uint32_t bitMask = ALL_COUNTERS);
	static uint32_t enabled();
	static void enable(Init list[]);
	static void disable(uint32_t bitMask = ALL_COUNTERS);
	static std::string showEnabled();

 private:
	static const char *Description[NUM_PERF_COUNTERS];
};

}  // namespace QPULib

#endif  // _QPULIB_PERFORMANCECOUNTERS_H

#endif  // QPU_MODE
