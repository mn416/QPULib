#ifdef QPU_MODE

#include "RegisterMap.h"
#include <cassert>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include "Mailbox.h"  // mapmem()

//
// This ugly part is to ensure:
//
// -  '__unix__' *is* set
// -  '__ANDROID__' is *not* set
//
//  ... for older versions of the bcm-include.
//
// Apparently, old distro's have a gcc version which assumes
// th presence/absence of these directives, which may prevent
// including certain system header files for the Pi.
//
#if !defined(__unix__)
#pragma message "__unix__ is NOT defined"
#define UNIX_PREVIOUSLY_UNDEFINED
#define __unix__
#endif

#if defined(__ANDROID__)
#pragma message "__ANDROID__ is defined"
#define ANDROID_PREVIOUSLY_DEFINED
#undef __ANDROID__
#endif

#include <bcm_host.h>

#if defined(ANDROID_PREVIOUSLY_DEFINED)
#define __ANDROID__
#undef ANDROID_PREVIOUSLY_DEFINED
#endif

#if defined(UNIX_PREVIOUSLY_UNDEFINED)
#undef __unix__
#undef UNIX_PREVIOUSLY_UNDEFINED
#endif

//
// End ugly part
//


namespace QPULib {

enum {
	V3D_BASE = (0xc00000 >> 2),
	V3D_IDENT0 = 0,
	V3D_IDENT1,
	V3D_IDENT2
};

std::unique_ptr<RegisterMap> RegisterMap::m_instance;


RegisterMap::RegisterMap() {
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
