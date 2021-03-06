/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef KERNEL_H
#define KERNEL_H

#include "MooseObject.h"
#include "SetupInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
#include "RandomInterface.h"
#include "PostprocessorInterface.h"
#include "GeometricSearchInterface.h"
#include "BlockRestrictable.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Restartable.h"
#include "ZeroInterface.h"
// libMesh
#include "libmesh/fe.h"
#include "libmesh/quadrature.h"

class MooseMesh;
class Problem;
class SubProblem;
class Kernel;

template<>
InputParameters validParams<Kernel>();

class Kernel :
  public MooseObject,
  public BlockRestrictable,
  public SetupInterface,
  public CoupleableMooseVariableDependencyIntermediateInterface,
  public FunctionInterface,
  public UserObjectInterface,
  public TransientInterface,
  public PostprocessorInterface,
  public MaterialPropertyInterface,
  public RandomInterface,
  protected GeometricSearchInterface,
  public Restartable,
  public ZeroInterface
{
public:
  Kernel(const std::string & name, InputParameters parameters);

  virtual ~Kernel();

  virtual void computeResidual();
  virtual void computeJacobian();
  /**
   * Computes d-residual / d-jvar... storing the result in Ke.
   */
  virtual void computeOffDiagJacobian(unsigned int jvar);
  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  virtual void computeOffDiagJacobianScalar(unsigned int jvar);

  /**
   * The variable number that this kernel operates on.
   */
  MooseVariable & variable();

  SubProblem & subProblem();

protected:
  SubProblem & _subproblem;
  FEProblem & _fe_problem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  MooseVariable & _var;
  MooseMesh & _mesh;
//  unsigned int _dim;

  const Elem * & _current_elem;
  /// Volume of the current element
  const Real & _current_elem_volume;

  unsigned int _qp;
  const MooseArray< Point > & _q_point;
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  unsigned int _i, _j;
  // shape functions
  const VariablePhiValue & _phi;
  const VariablePhiGradient & _grad_phi;

// test functions
  const VariableTestValue & _test;
  const VariableTestGradient & _grad_test;
//  const std::vector<std::vector<RealTensor> > & _second_test;

  /// Holds the solution at current quadrature points
  VariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  VariableGradient & _grad_u;

  /// Time derivative of u
  VariableValue & _u_dot;
  /// Derivative of u_dot wrt u
  VariableValue & _du_dot_du;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseVector<Number> _local_re;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseMatrix<Number> _local_ke;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariable*> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariable*> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;

  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  virtual void precalculateResidual();
};

#endif /* KERNEL_H */
