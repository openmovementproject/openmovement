// Dan Jackson, 2011-2013
// Median & mean filter

#include <stdlib.h>
#include <math.h>

#include "azimuth/filter.h"


/// Insertion sort a float array -- only use for small filter sizes, otherwise very inefficient!
static void ArraySortFloat(float *values, int length)
{
	float key;
	int i, j;
    for (j = 1; j < length; j++)
    {
        key = values[j];
		for (i = j - 1; i >= 0 && values[i] > key; i--) { values[i + 1] = values[i]; }
        values[i + 1] = key;
    }
}


// Initialize filter
void FilterInit(filter_t *filter, int capacity, float *values, float *sorted)
{
	filter->capacity = capacity;
	if (filter->values == NULL || values != NULL) { filter->values = values; }
	if (filter->sorted == NULL || sorted != NULL) { filter->sorted = sorted; }

	// Allocate, assuming structure was initialized to {0} and parameters passed as NULL
	if (filter->values == NULL) { filter->values = (float *)malloc(filter->capacity * sizeof(float)); }
	if (filter->sorted == NULL) { filter->sorted = (float *)malloc(filter->capacity * sizeof(float)); }

	filter->index = 0;
	filter->count = 0;
	filter->sum = 0;
	filter->invalidated = 1;
	filter->median = 0.0f;
}


// Add an entry to the filter
void FilterAdd(filter_t *filter, float value)
{
	if (filter->count >= filter->capacity)
	{
		int removeIndex = (filter->index + 1) % filter->capacity;
		filter->sum -= filter->values[removeIndex];
		filter->count = filter->capacity - 1;
	}
	filter->values[filter->index] = value;
	filter->sum += value;
	filter->index = (filter->index + 1) % filter->capacity;
	filter->count++;
	filter->invalidated = 1;
	filter->median = 0.0f;
}


// Get mean value
float FilterMean(filter_t *filter)
{
	if (filter->values == NULL || filter->count <= 0)
	{
		return 0.0f; 
	}
	return (float)(filter->sum / filter->count);
}


// Get median value
float FilterMedian(filter_t *filter)
{
	if (filter->values == NULL)
	{
		filter->median = 0.0f; 
	}
	else if (filter->invalidated)
	{
		filter->invalidated = 0;

		if (filter->count <= 0)
		{
			filter->median = 0.0f; 
		}
		else if (filter->count == 1)
		{ 
			filter->median = filter->values[(filter->index + filter->capacity - 1) % filter->capacity];
		}
		else if (filter->count == 2)
		{
			filter->median = (filter->values[(filter->index + filter->capacity - 1) % filter->capacity] + filter->values[(filter->index + filter->capacity - 2) % filter->capacity]) / 2.0f;
		}
		else if (filter->sorted == NULL)
		{
			filter->median = 0.0f;
		}
		else
		{
			int i;

			// Copy entries to sort buffer
			for (i = 0; i < filter->count; i++)
			{
				filter->sorted[i] = filter->values[(filter->index + i) % filter->capacity];
			}

			// Sort
			ArraySortFloat(filter->sorted, filter->count);

			// Return median value
			if (filter->count % 2)
			{
				filter->median = filter->sorted[filter->count / 2];
			}
			else
			{
				filter->median = (filter->sorted[filter->count / 2 - 1] + filter->sorted[filter->count / 2]) / 2.0f;
			}
		}
	}
	return filter->median;
}

