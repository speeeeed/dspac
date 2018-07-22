#ifndef __AEC_H__
#define __AEC_H__

#include <stddef.h>
#include <memory>

extern "C" {
#include "ring_buffer.h"
}

//namespace acowa {

#define FRAME_LEN 80
#define PART_LEN  64
#define PART_LEN1 (PART_LEN + 1)         // length of partition
#define PART_LEN2 (PART_LEN * 2)         // length of fft coef
#define NUM_HIGH_BANDS_MAX 2

typedef float complex_t[2];

enum {
	kOffsetLevel = -100
};

typedef struct Stats {
	float instant;
	float average;
	float min;
	float max;
	float sum;
	float hisum;
	float himean;
	size_t counter;
	size_t hicounter;
} Stats;

// Number of partitions for the extended filter mode. The first one is an enum
// to be used in array declarations, as it represents the maximum filter length.
enum {
	kExtendedNumPartitions = 32
};

static const int kNormalNumPartitions = 12;

enum {
	kLookaheadBlocks = 15;
};

enum {
	kHistorySizeBlocks = 125;
};

typedef struct PowerLevel {
	PowerLevel();

	BlockMeanCalculator framelevel;
	BlockMeanCalculator averagelevel;
	float minlevel;
} PowerLevel;

class BlockBuffer {
public:
	BlockBuffer();
	~BlockBuffer();

	void ReInit();
	void Insert(const float block[PART_LEN]);
	void ExtractExtendedBlock(float extended_block[PART_LEN]);
	int AdjustSize(int buffer_size_decrease);
	size_t Size();
	size_t AvaliableSpace();
private:
	RingBuffer *buffer_;
};
//}


#endif /* __AEC_H__ */
