[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Functions]
  [./f_fn]
    type = ParsedFunction
    value = -4
  [../]
  [./bc_all_fn]
    type = ParsedFunction
    value = x*x+y*y
  [../]

  # ODEs
  [./exact_x_fn]
    type = ParsedFunction
    value = (-1/3)*exp(-t)+(4/3)*exp(5*t)
  [../]
[]

# NL

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]

  # ODE variables
  [./x]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
  [./y]
    family = SCALAR
    order = FIRST
    initial_condition = 2
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./uff]
    type = UserForcingFunction
    variable = u
    function = f_fn
  [../]
[]

[ScalarKernels]
  [./td1]
    type = ODETimeDerivative
    variable = x
  [../]
  [./ode1]
    type = ImplicitODEx
    variable = x
    y = y
  [../]

  [./td2]
    type = ODETimeDerivative
    variable = y
  [../]
  [./ode2]
    type = ImplicitODEy
    variable = y
    x = x
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = bc_all_fn
  [../]
[]

[Postprocessors]
  active = 'exact_x l2err_x x y'

  [./x]
    type = PrintScalarVariable
    variable = x
    execute_on = timestep
  [../]
  [./y]
    type = PrintScalarVariable
    variable = y
    execute_on = timestep
  [../]

  [./exact_x]
    type = PlotFunction
    function = exact_x_fn
    execute_on = timestep
    point = '0 0 0'
  [../]

  [./l2err_x]
    type = ScalarL2Error
    variable = x
    function = exact_x_fn
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 0.01
  num_steps = 100
  petsc_options = '-snes_mf_operator'
[]

[Output]
  output_initial = true
  exodus = true
[]