#include "FiniteStrainRatePlasticMaterial.h"


/**
FiniteStrainRatePlasticMaterial integrates the rate dependent J2 plasticity model (Overstress model) in a
finite strain framework using return mapping algorithm.
Integration is performed in an incremental manner using Newton Raphson.
Isotropic hardening has been incorporated where yield stress as a function of equivalent
plastic strain has to be specified by the user.
*/

template<>
InputParameters validParams<FiniteStrainRatePlasticMaterial>()
{
  InputParameters params = validParams<FiniteStrainPlasticMaterial>();

  params.addRequiredParam< Real >("ref_pe_rate", "Reference plastic strain rate parameter for rate dependent plasticity (Overstress model)");
  params.addRequiredParam< Real >("exponent", "Exponent for rate dependent plasticity (Perzyna)");

  return params;
}

FiniteStrainRatePlasticMaterial::FiniteStrainRatePlasticMaterial(const std::string & name,
                                             InputParameters parameters)
    : FiniteStrainPlasticMaterial(name, parameters),
      _ref_pe_rate(getParam<Real>("ref_pe_rate")),//Read from input file
      _exponent(getParam<Real>("exponent"))//Read from input file

{
}

void FiniteStrainRatePlasticMaterial::initQpStatefulProperties()
{
  _elastic_strain[_qp].zero();
  _stress[_qp].zero();
  _plastic_strain[_qp].zero();
  _plastic_strain_old[_qp].zero();
  _eqv_plastic_strain[_qp]=0.0;


}

void FiniteStrainRatePlasticMaterial::computeQpStress()
{

  RankTwoTensor dp,sig;


  //In elastic problem, all the strain is elastic
  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp];

  //Obtain previous plastic rate of deformation tensor
  dp=_plastic_strain_old[_qp];

  //Solve J2 plastic constitutive equations based on current strain increment
  //Returns current  stress and plastic rate of deformation tensor

  solveStressResid(_stress_old[_qp],_strain_increment[_qp],_elasticity_tensor[_qp],&dp,&sig);
  _stress[_qp]=sig;

  //Updates current plastic rate of deformation tensor
  _plastic_strain[_qp]=dp;

  //Evaluate and update current equivalent plastic strain
  _eqv_plastic_strain[_qp]=pow(2.0/3.0,0.5)*dp.L2norm();


  //Rotate the stress to the current configuration
  _stress[_qp] = _rotation_increment[_qp]*_stress[_qp]*_rotation_increment[_qp].transpose();

  //Rotate to plastic rate of deformation tensor the current configuration
  _plastic_strain[_qp]=_rotation_increment[_qp]*_plastic_strain[_qp]*_rotation_increment[_qp].transpose();


}


/*
 *Solves for incremental plastic rate of deformation tensor and stress in unrotated frame.
 *Input: Strain incrment, 4th order elasticity tensor, stress tensor in previous incrmenent and
 *plastic rate of deformation tensor.
 */
void
FiniteStrainRatePlasticMaterial::solveStressResid(RankTwoTensor sig_old,RankTwoTensor delta_d,RankFourTensor E_ijkl, RankTwoTensor *dp, RankTwoTensor *sig)
{

  RankTwoTensor sig_new,delta_dp,dpn;
  RankTwoTensor flow_tensor, flow_dirn;
  RankTwoTensor resid,ddsig;
  RankFourTensor dr_dsig,dr_dsig_inv;
  Real /*sig_eqv,*/flow_incr,flow_incr_tmp;
  Real err1,err3,tol1,tol3;
  unsigned int iterisohard,iter,maxiterisohard=20,maxiter=50;
  Real eqvpstrain;
  Real yield_stress,yield_stress_prev;




  tol1=1e-10;
  tol3=1e-6;

  iterisohard=0;
  eqvpstrain=pow(2.0/3.0,0.5)*dp->L2norm();
  yield_stress=getYieldStress(eqvpstrain);

  err3=1.1*tol3;

  while(err3 > tol3 && iterisohard < maxiterisohard)//Hardness update iteration
  {

    iterisohard++;


    iter=0;
    delta_dp.zero();

    sig_new=sig_old+E_ijkl*delta_d;
    flow_incr=_ref_pe_rate*_dt*pow(macaulayBracket(getSigEqv(sig_new)/yield_stress-1.0),_exponent);

    getFlowTensor(sig_new,yield_stress,&flow_tensor);
    flow_dirn=flow_tensor;


    resid=flow_dirn*flow_incr-delta_dp;
    err1=resid.L2norm();

//    printf("resid=%f\n",err1);


    while(err1 > tol1  && iter < maxiter )//Stress update iteration (hardness fixed)
    {

      iter++;

      getJac(sig_new,E_ijkl,flow_incr,yield_stress,&dr_dsig);//Jacobian
      dr_dsig_inv=dr_dsig.invSymm();

      ddsig=-dr_dsig_inv*resid;

      sig_new+=ddsig;//Update stress
      delta_dp-=E_ijkl.invSymm()*ddsig;//Update plastic rate of deformation tensor

      flow_incr_tmp=_ref_pe_rate*_dt*pow(macaulayBracket(getSigEqv(sig_new)/yield_stress-1.0),_exponent);

      if(flow_incr_tmp < 0.0)//negative flow increment not allowed
        mooseError("Constitutive Error-Negative flow increment: Reduce time increment.");

      flow_incr=flow_incr_tmp;

      getFlowTensor(sig_new,yield_stress,&flow_tensor);
      flow_dirn=flow_tensor;

      resid=flow_dirn*flow_incr-delta_dp;//Residual


      err1=resid.L2norm();

//      printf("iter=%d resid=%f %f\n",iter,err1,flow_incr_tmp);
    }

    if(iter>=maxiter)//Convergence failure
    {
//      printf("flow_incr=%f\n",flow_incr);
      mooseError("Constitutive Error-Too many iterations: Reduce time increment.\n");//Convergence failure
    }


    dpn=*dp+delta_dp;
    eqvpstrain=pow(2.0/3.0,0.5)*dpn.L2norm();

    yield_stress_prev=yield_stress;
    yield_stress=getYieldStress(eqvpstrain);

    err3=fabs(yield_stress-yield_stress_prev);

  }

  if(iterisohard>=maxiterisohard)
  {
//    printf("flow_incr=%f %f %f %f\n",flow_incr,err3,yield_stress,eqvpstrain);
    mooseError("Constitutive Error-Too many iterations in Hardness Update:Reduce time increment.\n");//Convergence failure
  }

  *dp=dpn;//Plastic rate of deformation tensor in unrotated configuration

  *sig=sig_new;
}

//Obtain derivative of flow potential w.r.t. stress (plastic flow direction)
void
FiniteStrainRatePlasticMaterial::getFlowTensor(RankTwoTensor sig,Real /*yield_stress*/,RankTwoTensor* flow_tensor)
{

  RankTwoTensor sig_dev;
  Real sig_eqv,val;

  sig_eqv=getSigEqv(sig);
  sig_dev=getSigDev(sig);

  val=0.0;
  if(sig_eqv > 1e-8)
    val=3.0/(2.0*sig_eqv);
  *flow_tensor=sig_dev*val;
}



//Jacobian for stress update algorithm
void
FiniteStrainRatePlasticMaterial::getJac(RankTwoTensor sig, RankFourTensor E_ijkl, Real flow_incr, Real yield_stress,RankFourTensor* dresid_dsig)
{

  RankTwoTensor sig_dev, flow_tensor, flow_dirn,fij;
  RankTwoTensor dfi_dft;
  RankFourTensor dft_dsig,dfd_dft,dfd_dsig,dfi_dsig;
  Real sig_eqv/*,val*/;
  Real f1,f2,f3;
  Real dfi_dseqv;

  sig_dev=getSigDev(sig);
  sig_eqv=getSigEqv(sig);

  getFlowTensor(sig,yield_stress,&flow_tensor);

  flow_dirn=flow_tensor;
  dfi_dseqv=_ref_pe_rate*_dt*_exponent*pow(macaulayBracket(sig_eqv/yield_stress-1.0),_exponent-1.0)/yield_stress;


  for(int i=0;i<3;i++)
    for(int j=0;j<3;j++)
      for(int k=0;k<3;k++)
        for(int l=0;l<3;l++)
          dfi_dsig(i,j,k,l)=flow_dirn(i,j)*flow_dirn(k,l)*dfi_dseqv;//d_flow_increment/d_sig

  f1=0.0;
  f2=0.0;
  f3=0.0;

  if(sig_eqv > 1e-8)
  {
    f1=3.0/(2.0*sig_eqv);
    f2=f1/3.0;
    f3=9.0/(4.0*pow(sig_eqv,3.0));
  }


  for(int i=0;i<3;i++)
    for(int j=0;j<3;j++)
      for(int k=0;k<3;k++)
        for(int l=0;l<3;l++)
          dft_dsig(i,j,k,l)=f1*deltaFunc(i,k)*deltaFunc(j,l)-f2*deltaFunc(i,j)*deltaFunc(k,l)-f3*sig_dev(i,j)*sig_dev(k,l);//d_flow_dirn/d_sig - 2nd part

  dfd_dsig=dft_dsig;//d_flow_dirn/d_sig
  *dresid_dsig=E_ijkl.invSymm()+dfd_dsig*flow_incr+dfi_dsig;//Jacobian


}

//Macaulay Bracket used in Perzyna Model
Real
FiniteStrainRatePlasticMaterial::macaulayBracket(Real val)
{
  if(val > 0.0)
    return val;
  else
    return 0.0;

}


