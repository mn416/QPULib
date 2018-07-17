#ifdef QPU_MODE

#include "RegisterMap.h"
#include <cassert>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <bcm_host.h>
#include "Mailbox.h"  // for mapmem()

namespace QPULib {


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
 * @brief Get the 32-bit value of the register at the given offset in the map
 */
uint32_t RegisterMap::read(int offset) const {
	return m_addr[V3D_BASE + offset];
}


/**
 * @brief Set the 32-bit value of the register at the given offset in the map
 */
void RegisterMap::write(int offset, uint32_t value) {
	m_addr[V3D_BASE + offset] = value;
}


/**
 * @brief Convenience function for getting a register value.
 *
 * This avoids having to use `instance()->` for every read access.
 */
uint32_t RegisterMap::readRegister(int offset) {
	return instance()->read(offset);
}


/**
 * @brief Convenience function for setting a register value.
 *
 * This avoids having to use `instance()->` for every write access.
 */
void RegisterMap::writeRegister(int offset, uint32_t value) {
	instance()->write(offset, value);
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

}  // namespace QPULib

#endif  // QPU_MODE
