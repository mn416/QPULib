#ifdef QPU_MODE

#include "RegisterMap.h"
#include <cassert>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include "Mailbox.h"  // mapmem()

#ifdef USE_BCM_HEADERS
// Following works for newer distro's
#include <bcm_host.h>
#else  // USE_BCM_HEADERS
//#pragma message "This is an old pi!"
//
// For old Pi's, calls bcm_host_get_peripheral_address() and
// bcm_host_get_peripheral_size() don't exist, so we need
// to supply them ourselves.
//


/**
 * This returns the ARM-side physical address where peripherals are mapped.
 *
 * Values:
 *
 * - 0x20000000  - Pi Zero, Zero W, and the first generation of the Pi and Compute Module
 * - 0x3f000000  - Pi 2, Pi 3 and Compute Module 3
 *
 * NOTE: We only return the second value for now, to get QPULib to compile on an old Pi 2 distro.
 *       Other values may be added as needed.
 */
unsigned bcm_host_get_peripheral_address() {
	return 0x3f000000;
}


// following is the same for all Pi models
unsigned bcm_host_get_peripheral_size() { return 0x01000000; }

//
// These are the only things we need from bcm_host.h, we declare
// them explicitly to avoid dragging in all the stuff that throws compile errors.
//
#ifdef __cplusplus
extern "C" {
#endif

void bcm_host_init(void);
void bcm_host_deinit(void);

#ifdef __cplusplus
}
#endif
// End things we need from bcm_host.h

#endif  // USE_BCM_HEADERS


namespace QPULib {

enum {
	V3D_BASE = (0xc00000 >> 2),
	V3D_IDENT0 = 0,
	V3D_IDENT1,
	V3D_IDENT2
};

std::unique_ptr<RegisterMap> RegisterMap::m_instance;


RegisterMap::RegisterMap() : m_addr(nullptr), m_size(0) {
	bcm_host_init();
	unsigned addr = bcm_host_get_peripheral_address();
	m_size = bcm_host_get_peripheral_size();

	check_page_align(addr);

	// Following succeeds if it returns.
	m_addr = (uint32_t *) mapmem(addr, m_size);
	assert(m_addr != nullptr);
	// printf("init address: %08X, size: %u\n", m_addr, m_size);
}


RegisterMap::~RegisterMap() {
	unmapmem((void *) m_addr, m_size);
	bcm_host_deinit();
}


/**
 * @brief Get the 32-bit value at the given offset in the map
 */
uint32_t RegisterMap::read(int offset) const {
#ifdef DEBUG
	// Do a spot check on the loaded memory
	if (m_addr[V3D_BASE] == 0XDEADBEEF) {
		printf("WARNING: RegisterMap can not read QPU registers, the VideoCore is not enabled.\n");
	}
#endif

	return m_addr[V3D_BASE + offset];
}


/**
 * @brief Convenience function for getting a register value.
 *
 * This avoids having to use `instance()->` for every read access.
 */
uint32_t RegisterMap::readRegister(int offset) {
  //printf("Called readRegister, m_instance: %p\n", m_instance.get());
	return instance()->read(offset);
}


/**
 * @brief Check if the register map is accessible.
 *
 * This depends on the VideoCore being enabled. Enabling and disabling
 * is done with mailbox call `qpu_enable()`.
 *
 * This method can thus be used to detect if the VideoCore is running.
 *
 * @return true if register map accessible, false otherwise
 */
bool RegisterMap::enabled() {
	// Detect the signature in register 0
	uint32_t reg = readRegister(V3D_IDENT0);
	char *p = (char *) &reg;
#ifdef DEBUG
	printf("Reg 0: '%c%c%c'\n", p[0], p[1], p[2]);
#endif

	bool canRead = (p[0] == 'V' && p[1] == '3' && p[2] == 'D');

#ifdef PROB_NOT_REQUIRED
	if (!canRead) {
		// Reset singleton for a next attempt
		m_instance.reset(nullptr);
	}
#endif  // PROB_NOT_REQUIRED

	return canRead;
}


int RegisterMap::numSlices() {
	uint32_t reg = readRegister(V3D_IDENT1);
	// printf("reg V3D_IDENT1: %08X\n", reg);

	return (reg >> 4) & 0xf;
}


int RegisterMap::numQPUPerSlice() {
	return (readRegister(V3D_IDENT1) >> 8) & 0xf;
}


RegisterMap *RegisterMap::instance() {
  //printf("Called instance(), m_instance: %p\n", m_instance.get());

	if (m_instance.get() == nullptr) {
		//printf("RegisterMap initializing singleton\n");
		m_instance.reset(new RegisterMap);
	}

 // printf("m_instance post: %p\n", m_instance.get());
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
}

}  // namespace QPULib

#endif  // QPU_MODE
