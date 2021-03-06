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

// MOOSE includes
#include "TableOutputter.h"
#include "FEProblem.h"
#include "Postprocessor.h"
#include "PetscSupport.h"
#include "Executioner.h"
#include "MooseApp.h"
#include "Conversion.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<TableOutputter>()
{
  // Fit mode selection Enum
  MooseEnum pps_fit_mode(FormattedTable::getWidthModes());

  // Base class parameters
  InputParameters params = validParams<FileOutputter>();

  // Suppressing the output of nodal and elemental variables disables this type of output
  params.suppressParameter<bool>("output_elemental_variables");
  params.suppressParameter<bool>("output_nodal_variables");
  params.suppressParameter<bool>("elemental_as_nodal");
  params.suppressParameter<bool>("scalar_as_nodal");
  params.suppressParameter<bool>("output_input");

  return params;
}

TableOutputter::TableOutputter(const std::string & name, InputParameters parameters) :
    FileOutputter(name, parameters),
    _postprocessor_table(declareRestartableData<FormattedTable>("postprocessor_table")),
    _scalar_table(declareRestartableData<FormattedTable>("scalar_table")),
    _all_data_table(declareRestartableData<FormattedTable>("all_data_table"))
{
}

TableOutputter::~TableOutputter()
{
}

void
TableOutputter::outputNodalVariables()
{
  mooseError("Nodal nonlinear variable output not supported by TableOutputter output class");
}

void
TableOutputter::outputElementalVariables()
{
  mooseError("Elemental nonlinear variable output not supported by TableOutputter output class");
}

void
TableOutputter::outputPostprocessors()
{
  // List of names of the postprocessors to output
  const std::vector<std::string> & out = getPostprocessorOutput();

  // Loop through the postprocessor names and extract the values from the PostprocessorData storage
  for (std::vector<std::string>::const_iterator it = out.begin(); it != out.end(); ++it)
  {
    PostprocessorValue value = _problem_ptr->getPostprocessorValue(*it);
    _postprocessor_table.addData(*it, value, _time);
    _all_data_table.addData(*it, value, _time);
  }
}

void
TableOutputter::outputScalarVariables()
{
  // List of scalar variables
  const std::vector<std::string> & out = getScalarOutput();

  // Loop through each variable
  for (std::vector<std::string>::const_iterator it = out.begin(); it != out.end(); ++it)
  {
    // Get reference to the variable and the no. of components
    VariableValue & variable = _problem_ptr->getScalarVariable(0, *it).sln();
    unsigned int n = variable.size();

    // If the variable has a single component, simply output the value with the name
    if (n == 1)
    {
      _scalar_table.addData(*it, variable[0], _time);
      _all_data_table.addData(*it, variable[0], _time);
    }

    // Multi-component variables are appended with the component index
    else
      for (unsigned int i = 0; i < n; ++i)
      {
        std::ostringstream os;
        os << *it << "_" << i;
        _scalar_table.addData(os.str(), variable[i], _time);
        _all_data_table.addData(os.str(), variable[i], _time);
      }
  }
}
