/**
 @file    linear_inverted_pendulum.cc
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Dec 5, 2016
 @brief   Brief description
 */

#include <xpp/opt/linear_inverted_pendulum.h>
#include <vector>
#include <xpp/opt/variables/base_motion.h>

namespace xpp {
namespace opt {

LinearInvertedPendulum::LinearInvertedPendulum ()
{
}

LinearInvertedPendulum::~LinearInvertedPendulum ()
{
}

void
LinearInvertedPendulum::SetCurrent (const Vector3d& com_pos,
                                    const EELoad& ee_load,
                                    const EEPos& ee_pos)
{
  pos_     = com_pos.topRows<kDim2d>();
  h_       = com_pos.z();
  ee_load_ = ee_load;
  ee_pos_  = ee_pos;
}

LinearInvertedPendulum::ComAcc
LinearInvertedPendulum::GetAcceleration () const
{
  Cop u = CalculateCop();
  ComAcc acc_zmp = kGravity/h_*(pos_- u);

  return acc_zmp;
}

LinearInvertedPendulum::JacobianRow
LinearInvertedPendulum::GetJacobianOfAccWrtBase (
    const BaseMotion& com_motion, double t, Coords3D dim) const
{
  JacobianRow com_jac     = com_motion.GetJacobian(t, kPos, dim);
  JacobianRow jac_wrt_com = kGravity/h_*com_jac;

  return jac_wrt_com;
}

double
LinearInvertedPendulum::GetDerivativeOfAccWrtLoad (EndeffectorID ee,
                                                   d2::Coords dim) const
{
  double cop_wrt_load = GetDerivativeOfCopWrtLoad(ee)(dim);
  return kGravity/h_ * (-1* cop_wrt_load);
}

double
LinearInvertedPendulum::GetDerivativeOfAccWrtEEPos (EndeffectorID ee) const
{
  double cop_wrt_ee = GetDerivativeOfCopWrtEEPos(ee);
  return kGravity/h_ * (-1* cop_wrt_ee);
}

double
LinearInvertedPendulum::GetDerivativeOfCopWrtEEPos (EndeffectorID ee) const
{
  return ee_load_.At(ee)/GetLoadSum();
}

LinearInvertedPendulum::Cop
LinearInvertedPendulum::GetDerivativeOfCopWrtLoad (EndeffectorID ee) const
{
  Vector2d p = ee_pos_.At(ee).topRows<kDim2d>();
  Vector2d u = CalculateCop();

  return (p - u)/GetLoadSum();
}

LinearInvertedPendulum::Cop
LinearInvertedPendulum::CalculateCop () const
{
  Cop cop = Cop::Zero();

  double sum = GetLoadSum();

  for (auto ee : ee_pos_.GetEEsOrdered()) {
    double load_percent = ee_load_.At(ee)/sum;
    cop += load_percent*ee_pos_.At(ee).topRows<kDim2d>();
  }

  return cop;
}

double
LinearInvertedPendulum::GetLoadSum () const
{
  double sum = 0.0;
  for (double load : ee_load_.ToImpl())
    sum += load;

  assert(sum > 0); // while using inverted pendulum, this must be the case.

  return sum;
}


} /* namespace opt */
} /* namespace xpp */
