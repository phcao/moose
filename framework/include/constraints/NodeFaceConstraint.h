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

#ifndef NODEFACECONSTRAINT_H
#define NODEFACECONSTRAINT_H

//MOOSE includes
#include "Constraint.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TransientInterface.h"
#include "GeometricSearchInterface.h"

//libMesh includes
#include "libmesh/sparse_matrix.h"

//Forward Declarations
class NodeFaceConstraint;

template<>
InputParameters validParams<NodeFaceConstraint>();

/**
 * A NodeFaceConstraint is used when you need to create constraints between
 * two surfaces in a mesh.  It works by allowing you to modify the residual
 * and jacobian entries on "this" side (the node side, also referred to as
 * the slave side) and the "other" side (the face side, also referred to as
 * the master side)
 *
 * This is common for contact algorithms and other constraints.
 */
class NodeFaceConstraint :
  public Constraint,
  public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  NodeFaceConstraint(const std::string & name, InputParameters parameters);
  virtual ~NodeFaceConstraint();

  /**
   * Compute the value the slave node should have at the beginning of a timestep.
   */
  void computeSlaveValue(NumericVector<Number> & current_solution);

  /**
   * Computes the residual Nodal residual.
   */
  virtual void computeResidual();

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian();

  /**
   * Compute the value the slave node should have at the beginning of a timestep.
   */
  virtual Real computeQpSlaveValue() = 0;

  /**
   * This is the virtual that derived classes should override for computing the residual on neighboring element.
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on neighboring element.
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) = 0;

  /**
   * Whether or not this constraint should be applied.
   *
   * Get's called once per slave node.
   */
  virtual bool shouldApply() { return true; }

  /**
   * Whether or not the slave's residual should be overwritten.
   *
   * When this returns true the slave's residual as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
  virtual bool overwriteSlaveResidual();

  /**
   * Whether or not the slave's Jacobian row should be overwritten.
   *
   * When this returns true the slave's Jacobian row as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
  virtual bool overwriteSlaveJacobian(){return overwriteSlaveResidual();};

  // TODO: Make this protected or add an accessor
  // Do the same for all the other public members
  SparseMatrix<Number> * _jacobian;

protected:
  /// Boundary ID for the slave surface
  unsigned int _slave;
  /// Boundary ID for the master surface
  unsigned int _master;

  const MooseArray< Point > & _master_q_point;
  QBase * & _master_qrule;

public:
  PenetrationLocator & _penetration_locator;

protected:

  /// current node being processed
  const Node * & _current_node;
  const Elem * & _current_master;

  /// Value of the unknown variable this BC is action on
  VariableValue & _u_slave;
  /// Shape function on the slave side.  This will always
  VariablePhiValue _phi_slave;
  /// Shape function on the slave side.  This will always only have one entry and that entry will always be "1"
  VariableTestValue _test_slave;

  /// Side shape function.
  const VariablePhiValue & _phi_master;
  /// Gradient of side shape function
  const VariablePhiGradient & _grad_phi_master;

  /// Side test function
  const VariableTestValue & _test_master;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test_master;

  /// Holds the current solution at the current quadrature point
  VariableValue & _u_master;
  /// Holds the current solution gradient at the current quadrature point
  VariableGradient & _grad_u_master;

  /// DOF map
  const DofMap & _dof_map;

  std::map<dof_id_type, std::vector<dof_id_type> > & _node_to_elem_map;

  /**
   * Whether or not the slave's residual should be overwritten.
   *
   * When this is true the slave's residual as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
  bool _overwrite_slave_residual;

public:
  std::vector<dof_id_type> _connected_dof_indices;

  DenseMatrix<Number> _Kne;
  DenseMatrix<Number> _Kee;
};

#endif
