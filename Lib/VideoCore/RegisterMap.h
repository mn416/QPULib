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
	enum Indexes {
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


	RegisterMap(RegisterMap const &) = delete;
	void operator=(RegisterMap const &) = delete;

	~RegisterMap();

	static int numSlices();
	static int numQPUPerSlice();

	static uint32_t readRegister(int offset);
	static void writeRegister(int offset, uint32_t value);

private:
	RegisterMap();

	volatile uint32_t *m_addr{nullptr};
	unsigned m_size{0};

	static std::unique_ptr<RegisterMap> m_instance;

	uint32_t read(int offset) const;
	void write(int offset, uint32_t value);

	static RegisterMap *instance();
	static void check_page_align(unsigned addr);
};

}  // namespace QPULib

#endif  // _QPULIB_REGISTERMAP_H

#endif  // QPU_MODE
