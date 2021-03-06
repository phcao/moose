#ifndef INSTEMPERATURE_H
#define INSTEMPERATURE_H

#include "Kernel.h"

// Forward Declarations
class INSTemperature;

template<>
InputParameters validParams<INSTemperature>();

/**
 * This class computes the residual and Jacobian contributions for the
 * incompressible Navier-Stokes temperature (energy) equation.
 */
class INSTemperature : public Kernel
{
public:
  INSTemperature(const std::string & name, InputParameters parameters);

  virtual ~INSTemperature(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled variables
  VariableValue& _u_vel;
  VariableValue& _v_vel;
  VariableValue& _w_vel;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;

  // Required parameters
  Real _rho;
  Real _k;
  Real _cp;
};


#endif // INSTEMPERATURE_H
