// Standard rectangle rule integrator, and Runge-kutta integrator
// Dan Jackson, 2013

#include "azimuth/integrator.h"


void IntegratorInit(integrator_t *integrator)
{
    integrator->value = 0.0f;
}

void IntegratorAdd(integrator_t *integrator, float value)
{
    integrator->value += value;
}

float IntegratorGet(integrator_t *integrator)
{
    return integrator->value;
}




void RKIntegratorInit(rkintegrator_t *integrator)
{
    int i;
    for (i = 0; i < 4; i++) { integrator->values[i] = 0.0f; }
    integrator->index = 0;
    integrator->value = 0.0f;
}

void RKIntegratorAdd(rkintegrator_t *integrator, float value)
{
    float sum;

    integrator->values[integrator->index] = value;

    sum = 0.0f;
    integrator->index = (integrator->index + 1) & 3; sum += integrator->values[integrator->index] * 1.0f;
    integrator->index = (integrator->index + 1) & 3; sum += integrator->values[integrator->index] * 2.0f;
    integrator->index = (integrator->index + 1) & 3; sum += integrator->values[integrator->index] * 2.0f;
    integrator->index = (integrator->index + 1) & 3; sum += integrator->values[integrator->index] * 1.0f;
    integrator->index = (integrator->index + 1) & 3; 
    sum /= 6.0f;

    integrator->value += sum;
}

float RKIntegratorGet(rkintegrator_t *integrator)
{
    return integrator->value;
}

