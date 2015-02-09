// Runge-kutta integrator
// Dan Jackson, 2013

#ifndef INTEGRATOR_H
#define INTEGRATOR_H


#ifdef __cplusplus
extern "C" {
#endif

// Standard rectangle rule integrator
typedef struct
{
    float value;
} integrator_t;

void IntegratorInit(integrator_t *integrator);
void IntegratorAdd(integrator_t *integrator, float value);
float IntegratorGet(integrator_t *integrator);


// Runge-kutta integrator
typedef struct
{
    float values[4];
    int index;
    float value;
} rkintegrator_t;

void RKIntegratorInit(rkintegrator_t *integrator);
void RKIntegratorAdd(rkintegrator_t *integrator, float value);
float RKIntegratorGet(rkintegrator_t *integrator);

#ifdef __cplusplus
}
#endif

#endif
