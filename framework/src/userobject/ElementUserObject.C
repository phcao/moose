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

#include "Moose.h"
#include "ElementUserObject.h"
#include "MooseVariable.h"
#include "SubProblem.h"

template<>
InputParameters validParams<ElementUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params.addRequiredParam<VariableName>("variable", "The name of the variable that this object operates on");
  std::vector<SubdomainName> everywhere(1);
  everywhere[0] = "ANY_BLOCK_ID";
  params.addParam<std::vector<SubdomainName> >("block", everywhere, "block ID or name where the object works");
  return params;
}

ElementUserObject::ElementUserObject(const std::string & name, InputParameters parameters) :
    UserObject(name, parameters),
    Coupleable(parameters, false),
    UserObjectInterface(parameters),
    MooseVariableInterface(parameters, false),
    TransientInterface(parameters),
    MaterialPropertyInterface(parameters),
    _blocks(parameters.get<std::vector<SubdomainName> >("block")),
    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),
    _q_point(_subproblem.points(_tid)),
    _qrule(_subproblem.qRule(_tid)),
    _JxW(_subproblem.JxW(_tid)),
    _coord(_subproblem.coords(_tid)),
    _current_elem(_subproblem.elem(_tid)),
    _current_elem_volume(_subproblem.elemVolume(_tid)),
    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid]),
    _grad_zero(_problem._grad_zero[_tid]),
    _second_zero(_problem._second_zero[_tid])
{
}