#ifdef QPU_MODE

#ifndef _QPULIB_REGISTERMAP_H
#define _QPULIB_REGISTERMAP_H
#include <memory>
#include <stdint.h>

namespace QPULib {

/**
 * @brief interface for the VideoCore registers.
 *
 * This implementation is not complete. Registers and their
 * handling will be added as needed.
 *
 * Implemented as singleton with lazy load, so that it's 
 * not initialized when it's not used.
 */
class RegisterMap {
public:
	enum {
  	ALL_COUNTERS = (1 << 16) - 1
	};

	RegisterMap(RegisterMap const &) = delete;
	void operator=(RegisterMap const &) = delete;

	~RegisterMap();

	static int numSlices();
	static int numQPUPerSlice();

  static void clearPerformanceCounters(int bitMask = ALL_COUNTERS);
	static uint32_t readRegister(int offset);

private:
	RegisterMap();

	volatile uint32_t *m_addr{nullptr};
	unsigned m_size{0};

	static std::unique_ptr<RegisterMap> m_instance;

	uint32_t read(int offset) const;

	static RegisterMap *instance();
	static void check_page_align(unsigned addr);
};

}  // namespace QPULib

#endif  // _QPULIB_REGISTERMAP_H

#endif  // QPU_MODE
