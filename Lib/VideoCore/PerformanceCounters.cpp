#ifdef QPU_MODE
#include "PerformanceCounters.h"
#include <sstream>


namespace QPULib {

using RM = RegisterMap;


/**
 * Short versions of performance counter descriptions.
 *
 * These are intended as labels on display.
 * The counter id is the index into the array.
 * Padded for display.
 *
 * See: "VideoCore® IV 3D Architecture Reference Guide, Table 82: Sources for Performance Counters", page 97.
 */
const char *PerformanceCounters::Description[PerformanceCounters::NUM_PERF_COUNTERS] = {
	// FEP Valid primitives for all rendered tiles
  "No rendered pixels                ",   // index 0
	"Valid primitives                  ",
	// FEP
	"Early-Z/Near/Far clipped quads    ",
	"Valid quads                       ",
	// TLB Quads pixels
	"Failing stencil test              ",
	"Failing Z and stencil tests       ",    // index 5
	"Any failing Z and stencil tests   ",
	"All having zero coverage          ",
	"Any having non-zero coverage      ",
	"Valid written to color buffer     ",
  // PTB Primitives
	"Outside the viewport              ",    // index 10
	"Need clipping                     ",
	// PSE Primitives
	"Discarded, reversed               ",
	// QPU Total clock cycles for all QPUs
	"Idle                              ",
	"Doing vertex/coordinate shading   ",
	"Doing fragment shading            ",    // index 15
	"Executing valid instructions      ",
	"Stalled waiting for TMUs          ",
	"Stalled waiting for Scoreboard    ",
	"Stalled waiting for Varyings      ",
	// QPU Total, for all slices
	"QPU Instruction cache hits        ",    // index 20
	"QPU Instruction cache misses      ",
	"QPU cache hits                    ",
	"QPU cache misses                  ",
	// TMU texture, total
	"Quads processed                   ",
	"cache misses                      ",    // index 25 (number of fetches from memory/L2cache)
	// VPM Total clock cycles
	"VDW stalled waiting for VPM access",
	"VCD stalled waiting for VPM access",
	// L2C Total
	"Level 2 cache hits                ",
	"Level 2 cache misses              "     // index 29
};


/**
 * @brief Reset the currently mapped performance counters
 *
 * The counters in the slots indicated by the corresponding bit in the passed bitmask
 * are set to zero.
 *
 * @param bitMask  setting a bit to '1' clears the counter in the register corresponding to
 *                 that index. Default: clear all counter registers.
 */
void PerformanceCounters::clear(uint32_t bitMask) {
  bitMask = bitMask & ALL_COUNTERS;   // Top 16 bits should be zero by specification
	RM::writeRegister(RM::V3D_PCTRC, bitMask);
}


/**
 * @brief Return the bitmask for the enabled performance counters
 *
 * @return bitmask; if bit 0 is '1', the performance counter 0 is enabled, etc.
 */
uint32_t PerformanceCounters::enabled() {
	return RM::readRegister(RM::V3D_PCTRE);
}


/**
 * @brief Initialize counter slots to the given counters
 *
 * The input is a list of counter slots paired with the counter to use in that slot.
 *
 * Example:
 * ```c++
 *	Init list[] = {
 *		{ 0, PC::L2C_CACHE_HITS },
 *		{ PC::END_MARKER, PC::END_MARKER }  // End marker required
 *	};
 *
 * 	enable(list);
 * ```
 *
 * ---------------------------
 * ## NOTE:
 *
 * - The passed counter definitions overwrite anything that is already present.
 * - The slot indexes need not be consecutive. valid values are 0..15
 * - There is no problem with designating the same counter index multiple times.
 */
void PerformanceCounters::enable(Init list[]) {
	// Set the passed registers
	for (int i = 0; !list[i].isEnd(); ++i) {
		RM::Index targetIndex = (RM::Index) (RM::V3D_PCTRS0 + 2*list[i].slotIndex);
		//printf("targetIndex: %d\n", targetIndex);
		//printf("counterIndex: %d\n", list[i].counterIndex);
		RM::writeRegister(targetIndex, list[i].counterIndex);
	}

	// Set enabling bitmask 
	uint32_t bitMask = enabled();

	for (int i = 0; !list[i].isEnd(); ++i) {
		//printf("enable handling counter %d\n", list[i].counterIndex);
		bitMask = bitMask | (1 << (list[i].slotIndex));
	}
	bitMask = bitMask & ALL_COUNTERS;   // Top 16 bits should be zero by specification

	// Following is NOT documented in the Ref Doc; I got it from the errata.
	// Top bit of mask must be set for timers to be enabled.
	bitMask = bitMask | (1 << 31);
	
	RM::writeRegister(RM::V3D_PCTRE, bitMask);
}


/**
 * @brief Disable the counters in the slots as specified by the bitmask
 *
 * 
 * @param bitMask  setting a bit to '1' disable the counter in the register corresponding to
 *                 that index. Default: disable all counter registers.
 */
void PerformanceCounters::disable(uint32_t bitMask) {
	bitMask = enabled() & ~bitMask;
	bitMask = bitMask & ALL_COUNTERS;   // Top 16 bits should be zero by specification

	if (bitMask != 0) {
		// Some counters still running, set the top enable bit
		bitMask = bitMask | (1 << 31);
	}

	RM::writeRegister(RM::V3D_PCTRE, bitMask);
}


/**
 * @brief Create a string representations of the enabled counters and their values.
 */
std::string PerformanceCounters::showEnabled() {
	uint32_t bitMask = enabled();
	// printf("bitMask: %X\n", bitMask);
	std::ostringstream os;

	os << "Enabled counters:\n";

	for (int i = 0; i < SLOT_COUNT; ++i) {
		bool enabled = (0 != (bitMask & (1 << i)));
		if (!enabled) continue;

		RM::Index sourceIndex = (RM::Index) (RM::V3D_PCTR0 + 2*i);
		Index counterIndex = (Index) RM::readRegister(sourceIndex + 1);
		//printf("counterIndex: %d\n", counterIndex);
		//fflush(stdout);

		os << "  " <<  Description[counterIndex] << ": " << RM::readRegister(sourceIndex) << "\n";
	}

	return os.str();
}

}  // namespace QPULib


#endif  // QPU_MODE
