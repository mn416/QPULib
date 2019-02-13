#ifndef _QPULIB_REGISTERMAP_H
#define _QPULIB_REGISTERMAP_H

#include "qpulib_config.h"
#ifdef QPULIB_QPU_MODE

#include <memory>
#include <stdint.h>

namespace QPULib {

/**
 * @brief interface for the VideoCore registers.
 *
 * This implementation is far from complete. It only reads
 * two fields from a single register. Regard it as a proof of
 * concept which can be expanded as needed.
 *
 * Implemented as singleton with lazy load, so that it's 
 * not initialized when it's not used.
 */
class RegisterMap {
public:
	RegisterMap(RegisterMap const &) = delete;
	void operator=(RegisterMap const &) = delete;

	~RegisterMap();

	static int numSlices();
	static int numQPUPerSlice();

private:
	RegisterMap();

	volatile uint32_t *m_addr{nullptr};
	unsigned m_size{0};

	static std::unique_ptr<RegisterMap> m_instance;

	uint32_t read(int offset) const;
	static uint32_t readRegister(int offset);

	static RegisterMap *instance();
	static void check_page_align(unsigned addr);
};

}  // namespace QPULib

#endif  // QPULIB_QPU_MODE
#endif  // _QPULIB_REGISTERMAP_H
