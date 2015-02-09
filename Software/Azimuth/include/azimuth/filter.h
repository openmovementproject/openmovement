// Dan Jackson, 2011
// Median & mean filter

#ifndef FILTER
#define FILTER

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int capacity;
    float *values;
	int index;
	int count;
	double sum;

	// median filter
    float *sorted;
	char invalidated;
	float median;
} filter_t;

void FilterInit(filter_t *filter, int capacity, float *values, float *sorted);
void FilterAdd(filter_t *filter, float value);
float FilterMean(filter_t *filter);
float FilterMedian(filter_t *filter);

#ifdef __cplusplus
}
#endif

#endif

