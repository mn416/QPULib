#ifdef QPU_MODE

#ifndef _QPULIB_REGISTERMAP_H
#define _QPULIB_REGISTERMAP_H
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

	static bool enabled();
	static int  numSlices();
	static int  numQPUPerSlice();

private:
	RegisterMap();

	volatile uint32_t *m_addr;
	unsigned m_size;

	static std::unique_ptr<RegisterMap> m_instance;

	uint32_t read(int offset) const;
	static uint32_t readRegister(int offset);

	static RegisterMap *instance();
	static void check_page_align(unsigned addr);
};

}  // namespace QPULib

#endif  // _QPULIB_REGISTERMAP_H

#endif  // QPU_MODE
