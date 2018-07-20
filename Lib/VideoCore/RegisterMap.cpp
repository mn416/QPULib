#ifdef QPU_MODE

#include "RegisterMap.h"
#include <cassert>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <bcm_host.h>
#include "Mailbox.h"  // for mapmem()

namespace QPULib {

enum {
	V3D_BASE = (0xc00000 >> 2),
	V3D_IDENT0 = 0,
	V3D_IDENT1,
	V3D_IDENT2,
	V3D_L2CACTL = (0x00020 >> 2)
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
	// printf("Closing down register map\n");
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
	return instance()->read(V3D_IDENT1);
}


int RegisterMap::numSlices() {
	uint32_t reg = readRegister(V3D_IDENT1);
	// printf("reg V3D_IDENT1: %08X\n", reg);

	return (reg >> 4) & 0xf;
}


int RegisterMap::numQPUPerSlice() {
	return (readRegister(V3D_IDENT1) >> 8) & 0xf;
}


int RegisterMap::numTMUPerSlice() {
	return (readRegister(V3D_IDENT1) >> 12) & 0xf;
}


/**
 * @brief get the size of the VPM.
 *
 * @return Size of VPM in KB
 */
int RegisterMap::VPMSize() {
	uint32_t reg = readRegister(V3D_IDENT1);
	int value = (reg >> 28) & 0xf;

	if (value == 0) return 16;  // According to reference doc p.97
	return value;
}


int RegisterMap::L2CacheEnabled() {
	return (readRegister(V3D_L2CACTL) & 0x1);
}


RegisterMap *RegisterMap::instance() {
	if (m_instance.get() == nullptr) {
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
}

}  // namespace QPULib

#endif  // QPU_MODE
