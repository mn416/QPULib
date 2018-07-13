#ifdef QPU_MODE

#include "RegisterMap.h"
#include <cassert>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <bcm_host.h>
#include "Mailbox.h"  // for mapmem()

namespace QPULib {

const int NUM_PERFORMANCE_COUNTERS = 30;

enum {
	V3D_BASE = (0xc00000 >> 2),
	V3D_IDENT0 = 0,
	V3D_IDENT1,
	V3D_IDENT2,

	//
	// Performance counter registers.
	//
	// There are 30 performance counters, but only 16 registers available
	// to access them. You therefore have to map beforehand the counters
	// you are interested in to an available slot.
	//
	// PC: 'Performance Counter' below
	//
  V3D_PCTRC = (0x00670 >> 2),  // PC Clear     - write only
	V3D_PCTRE,                   // PC Enables   - read/write
	V3D_PCTR0,                   // PC Count 0   - read/write
	V3D_PCTRS0,                  // PC Mapping 0 - read/write
	V3D_PCTR1,                   // PC Count 1
	V3D_PCTRS1,                  // PC Mapping 1
	V3D_PCTR2,                   // etc.
	V3D_PCTRS2,
	V3D_PCTR3,
	V3D_PCTRS3,
	V3D_PCTR4,
	V3D_PCTRS4,
	V3D_PCTR5,
	V3D_PCTRS5,
	V3D_PCTR6,
	V3D_PCTRS6,
	V3D_PCTR7,
	V3D_PCTRS7,
	V3D_PCTR8,
	V3D_PCTRS8,
	V3D_PCTR9,
	V3D_PCTRS9,
	V3D_PCTR10,
	V3D_PCTRS10,
	V3D_PCTR11,
	V3D_PCTRS11,
	V3D_PCTR12,
	V3D_PCTRS12,
	V3D_PCTR13,
	V3D_PCTRS13,
	V3D_PCTR14,
	V3D_PCTRS14,
	V3D_PCTR15,
	V3D_PCTRS15
};


/**
 * Short versions of performance counter descriptions.
 *
 * These are intended as labels on display.
 * The counter id is the index into the array.
 *
 * See: "VideoCore® IV 3D Architecture Reference Guide, Table 82: Sources for Performance Counters", page 97.
 */
static const char *PerformanceCounterDescription[NUM_PERFORMANCE_COUNTERS] = {
	// FEP Valid primitives for all rendered tiles
  "No rendered pixels",                    // index 0
	"Valid primitives",
	// FEP
	"Early-Z/Near/Far clipped quads",
	"Valid quads",
	// TLB Quads pixels
	"Failing stencil test",
	"Failing Z and stencil tests",           // index 5
	"Any failing Z and stencil tests",
	"All having zero coverage",
	"Any having non-zero coverage",
	"Valid written to color buffer",
  // PTB Primitives
	"Outside the viewport",                  // index 10
	"Need clipping",
	// PSE Primitives
	"Discarded, reversed",
	// QPU Total clock cycles for all QPUs
	"Idle",
	"Doing vertex/coordinate shading",
	"Doing fragment shading",                // index 15
	"Executing valid instructions",
	"Stalled waiting for TMUs",
	"Stalled waiting for Scoreboard",
	"Stalled waiting for Varyings",
	// QPU Total, for all slices
	"Instruction cache hits",                // index 20
	"Instruction cache misses",
	"cache hits",
	"cache misses",
	// TMU texture, total
	"Quads processed",
	"cache misses",                          // index 25 (number of fetches from memory/L2cache)
	// VPM Total clock cycles
	"VDW stalled waiting for VPM access",
	"VCD stalled waiting for VPM access",
	// L2C Total
	"Level 2 cache hits",
	"Level 2 cache misses"                  // index 29
};


std::unique_ptr<RegisterMap> RegisterMap::m_instance;


RegisterMap::RegisterMap() {
	bcm_host_init();
	unsigned addr = bcm_host_get_peripheral_address();
	m_size = bcm_host_get_peripheral_size();
	printf("peripheral address: %08X, size: %08X\n", addr, m_size);

	check_page_align(addr);

	// Following succeeds if it returns.
	m_addr = (uint32_t *) mapmem(addr, m_size);
	assert(m_addr != nullptr);
	printf("init address: %08X, size: %u\n", m_addr, m_size);
}


RegisterMap::~RegisterMap() {
	printf("Closing down register map\n");
	unmapmem((void *) m_addr, m_size);
	bcm_host_deinit();
}


/**
 * @brief Get the 32-bit value at the given offset in the map
 */
uint32_t RegisterMap::read(int offset) const {
	return m_addr[V3D_BASE + offset];
}


/**
 * @brief Convenience function for getting a register value.
 *
 * This avoids having to use `instance()->` for every read access.
 */
uint32_t RegisterMap::readRegister(int offset) {
	return instance()->read(offset);
}


int RegisterMap::numSlices() {
	uint32_t reg = readRegister(V3D_IDENT1);
	// printf("reg V3D_IDENT1: %08X\n", reg);

	int ret = (reg >> 4) & 0xf;
	//printf("numSlices ret: %d\n", ret);
	return ret;
}


int RegisterMap::numQPUPerSlice() {
	int ret = (readRegister(V3D_IDENT1) >> 8) & 0xf;
	//printf("numQPUPerSlice ret: %d\n", ret);
	return ret;
}


RegisterMap *RegisterMap::instance() {
	if (m_instance.get() == nullptr) {
		printf("Creating RegisterMap singleton\n");
		m_instance.reset(new RegisterMap);
	}

	return m_instance.get();
}


void RegisterMap::check_page_align(unsigned addr) {
	long pagesize = sysconf(_SC_PAGESIZE);

	if (pagesize <= 0) {
		fprintf(stderr, "error: sysconf: %s\n", strerror(errno));
		exit(-1);
	}

	if (addr & (((unsigned) pagesize) - 1)) {
		fprintf(stderr, "error: peripheral address is not aligned to page size\n");
		exit(-1);
	}

	//printf("check_page_align addr %X is properly aligned\n", addr);
}


/**
 * Reset the currently mapped performance counters
 *
 * @param bitMask  setting a bit to '1' clears the counter in the register corresponding to
 *                 that index. Default: clear all counter registers.
 */
void RegisterMap::clearPerformanceCounters(int bitMask) {
  bitMask = bitMask & ALL_COUNTERS;   // Top 16 bits should be zero by specification
}

}  // namespace QPULib

#endif  // QPU_MODE
