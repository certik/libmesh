#ifndef __assembly_h__
#define __assembly_h__

#include "sparse_matrix.h"
#include "numeric_vector.h"
#include "dense_matrix.h"
#include "dense_vector.h"
#include "fe.h"
#include "fe_interface.h"
#include "fe_base.h"
#include "elem_assembly.h"
#include "quadrature_gauss.h"
#include "boundary_info.h"

// rbOOmit includes
#include "rb_theta.h"
#include "rb_assembly_expansion.h"
#include "rb_parametrized_function.h"
#include "rb_eim_construction.h"
#include "rb_eim_theta.h"

// Bring in bits from the libMesh namespace.
// Just the bits we're using, since this is a header.
using libMesh::ElemAssembly;
using libMesh::FEInterface;
using libMesh::FEMContext;
using libMesh::Number;
using libMesh::Point;
using libMesh::RBTheta;
using libMesh::Real;
using libMesh::RealGradient;

struct ElemAssemblyWithConstruction : ElemAssembly
{
  RBConstruction* rb_con;
};

// The "x component" of the function we're approximating with EIM
struct Gx : public RBParametrizedFunction
{
  virtual Number evaluate(const RBParameters& mu,
                          const Point& p)
  {
    Real curvature = mu.get_value("curvature");
    return 1. + curvature*p(0);
  }
};

// The "y component" of the function we're approximating with EIM
struct Gy : public RBParametrizedFunction
{
  virtual Number evaluate(const RBParameters& ,
                          const Point& )
  {
    return 1.;
  }
};

// The "z component" of the function we're approximating with EIM
struct Gz : public RBParametrizedFunction
{
  virtual Number evaluate(const RBParameters& mu,
                          const Point& p)
  {
    Real curvature = mu.get_value("curvature");
    return 1./(1. + curvature*p(0));
  }
};

struct ThetaA0 : RBTheta {
virtual Number evaluate(const RBParameters& mu)
{
  return mu.get_value("kappa") * mu.get_value("Bi");
}
};
struct AssemblyA0 : ElemAssemblyWithConstruction
{
  virtual void boundary_assembly(FEMContext &c)
  {
    short int bc_id = rb_con->get_mesh().boundary_info->boundary_id (c.elem,c.side);
    if( bc_id == 1 || bc_id == 2 || bc_id == 3 || bc_id == 4 )
    {
      const unsigned int u_var = 0;

      const std::vector<Real> &JxW_side =
        c.side_fe_var[u_var]->get_JxW();

      const std::vector<std::vector<Real> >& phi_side =
        c.side_fe_var[u_var]->get_phi();

      // The number of local degrees of freedom in each variable
      const unsigned int n_u_dofs = c.dof_indices_var[u_var].size();

      // Now we will build the affine operator
      unsigned int n_sidepoints = c.side_qrule->n_points();

      for (unsigned int qp=0; qp != n_sidepoints; qp++)
        for (unsigned int i=0; i != n_u_dofs; i++)
          for (unsigned int j=0; j != n_u_dofs; j++)
            c.elem_jacobian(i,j) += JxW_side[qp] * phi_side[j][qp]*phi_side[i][qp];
    }
  }
};

struct ThetaA1 : RBTheta {
virtual Number evaluate(const RBParameters& mu)
{
  return mu.get_value("kappa") * mu.get_value("Bi") * mu.get_value("curvature");
}
};
struct AssemblyA1 : ElemAssemblyWithConstruction
{
  virtual void boundary_assembly(FEMContext &c)
  {
    short int bc_id = rb_con->get_mesh().boundary_info->boundary_id (c.elem,c.side);
    if( bc_id == 2 || bc_id == 4 )
    //if( bc_id == 1 || bc_id == 3 ) MAKE SURE THE BC_IDs ARE CORRECT HERE!
    {
      const unsigned int u_var = 0;

      const std::vector<Real> &JxW_side =
        c.side_fe_var[u_var]->get_JxW();

      const std::vector<std::vector<Real> >& phi_side =
        c.side_fe_var[u_var]->get_phi();
        
      const std::vector<Point>& xyz =
        c.side_fe_var[u_var]->get_xyz();

      // The number of local degrees of freedom in each variable
      const unsigned int n_u_dofs = c.dof_indices_var[u_var].size();

      // Now we will build the affine operator
      unsigned int n_sidepoints = c.side_qrule->n_points();

      for (unsigned int qp=0; qp != n_sidepoints; qp++)
      {
        Real x_hat = xyz[qp](0);
        
        for (unsigned int i=0; i != n_u_dofs; i++)
          for (unsigned int j=0; j != n_u_dofs; j++)
            c.elem_jacobian(i,j) += JxW_side[qp] * x_hat * phi_side[j][qp]*phi_side[i][qp];
      }
    }
  }
};

struct ThetaA2 : RBTheta {
virtual Number evaluate(const RBParameters& mu)
{
  return 0.4*mu.get_value("kappa") * mu.get_value("Bi") * mu.get_value("curvature");
}
};
struct AssemblyA2 : ElemAssemblyWithConstruction
{
  virtual void boundary_assembly(FEMContext &c)
  {
    short int bc_id = rb_con->get_mesh().boundary_info->boundary_id (c.elem,c.side);
    if( bc_id == 3 ) // MAKE SURE THE BC_IDs ARE CORRECT HERE!
    {
      const unsigned int u_var = 0;

      const std::vector<Real> &JxW_side =
        c.side_fe_var[u_var]->get_JxW();

      const std::vector<std::vector<Real> >& phi_side =
        c.side_fe_var[u_var]->get_phi();

      // The number of local degrees of freedom in each variable
      const unsigned int n_u_dofs = c.dof_indices_var[u_var].size();

      // Now we will build the affine operator
      unsigned int n_sidepoints = c.side_qrule->n_points();

      for (unsigned int qp=0; qp != n_sidepoints; qp++)
        for (unsigned int i=0; i != n_u_dofs; i++)
          for (unsigned int j=0; j != n_u_dofs; j++)
            c.elem_jacobian(i,j) += JxW_side[qp] * phi_side[j][qp]*phi_side[i][qp];
    }
  }
};

struct ThetaEIM : RBEIMTheta {

ThetaEIM(RBEIMEvaluation& rb_eim_eval_in, unsigned int index_in)
:
RBEIMTheta(rb_eim_eval_in, index_in)
{}

virtual Number evaluate(const RBParameters& mu)
{
  return mu.get_value("kappa") * RBEIMTheta::evaluate(mu);
}
};
struct AssemblyEIM : RBEIMAssembly
{

  AssemblyEIM(RBEIMConstruction& rb_eim_con_in,
              unsigned int basis_function_index_in)
  : RBEIMAssembly(rb_eim_con_in,
                  basis_function_index_in)
  {}

  virtual void interior_assembly(FEMContext &c)
  {
    // PDE variable numbers
    const unsigned int u_var = 0;
    
    // EIM variable numbers
    const unsigned int Gx_var = 0;
    const unsigned int Gy_var = 1;
    const unsigned int Gz_var = 2;

    const std::vector<Real> &JxW =
      c.element_fe_var[u_var]->get_JxW();

    const std::vector<std::vector<RealGradient> >& dphi =
      c.element_fe_var[u_var]->get_dphi();

    const std::vector<Point>& qpoints =
      c.element_fe_var[u_var]->get_xyz();

    // The number of local degrees of freedom in each variable
    const unsigned int n_u_dofs = c.dof_indices_var[u_var].size();

    // Now we will build the affine operator
    unsigned int n_qpoints = (c.get_element_qrule())->n_points();

    std::vector<Number> eim_values_Gx;
    evaluate_basis_function(Gx_var,
                            *c.elem,
                            qpoints,
                            eim_values_Gx);

    std::vector<Number> eim_values_Gy;
    evaluate_basis_function(Gy_var,
                            *c.elem,
                            qpoints,
                            eim_values_Gy);

    std::vector<Number> eim_values_Gz;
    evaluate_basis_function(Gz_var,
                            *c.elem,
                            qpoints,
                            eim_values_Gz);

    for (unsigned int qp=0; qp != n_qpoints; qp++)
    {
      for (unsigned int i=0; i != n_u_dofs; i++)
        for (unsigned int j=0; j != n_u_dofs; j++)
        {
          c.elem_jacobian(i,j) += JxW[qp] * ( eim_values_Gx[qp]*dphi[i][qp](0)*dphi[j][qp](0) + 
                                              eim_values_Gy[qp]*dphi[i][qp](1)*dphi[j][qp](1) + 
                                              eim_values_Gz[qp]*dphi[i][qp](2)*dphi[j][qp](2) );
        }
    }
  }

};


struct ThetaF0 : RBTheta { virtual Number evaluate(const RBParameters&   ) { return 1.; } };
struct AssemblyF0 : ElemAssembly
{

  virtual void interior_assembly(FEMContext &c)
  {
    const unsigned int u_var = 0;

    const std::vector<Real> &JxW =
      c.element_fe_var[u_var]->get_JxW();

    const std::vector<std::vector<Real> >& phi =
      c.element_fe_var[u_var]->get_phi();

    // The number of local degrees of freedom in each variable
    const unsigned int n_u_dofs = c.dof_indices_var[u_var].size();

    // Now we will build the affine operator
    unsigned int n_qpoints = (c.get_element_qrule())->n_points();

    for (unsigned int qp=0; qp != n_qpoints; qp++)
      for (unsigned int i=0; i != n_u_dofs; i++)
        c.elem_residual(i) += JxW[qp] * ( 1.*phi[i][qp] );
  }
};

struct ThetaF1 : RBTheta { virtual Number evaluate(const RBParameters& mu) { return mu.get_value("curvature"); } };
struct AssemblyF1 : ElemAssembly
{

  virtual void interior_assembly(FEMContext &c)
  {
    const unsigned int u_var = 0;

    const std::vector<Real> &JxW =
      c.element_fe_var[u_var]->get_JxW();

    const std::vector<std::vector<Real> >& phi =
      c.element_fe_var[u_var]->get_phi();
    
    const std::vector<Point>& xyz =
      c.element_fe_var[u_var]->get_xyz();

    // The number of local degrees of freedom in each variable
    const unsigned int n_u_dofs = c.dof_indices_var[u_var].size();

    // Now we will build the affine operator
    unsigned int n_qpoints = (c.get_element_qrule())->n_points();

    for (unsigned int qp=0; qp != n_qpoints; qp++)
    {
      Real x_hat = xyz[qp](0);
      
      for (unsigned int i=0; i != n_u_dofs; i++)
        c.elem_residual(i) += JxW[qp] * ( 1.*x_hat*phi[i][qp] );
    }
  }
};

struct Ex6InnerProduct : ElemAssembly
{
  // Assemble the Laplacian operator
  virtual void interior_assembly(FEMContext &c)
  {
    const unsigned int u_var = 0;

    const std::vector<Real> &JxW =
      c.element_fe_var[u_var]->get_JxW();

    // The velocity shape function gradients at interior
    // quadrature points.
    const std::vector<std::vector<RealGradient> >& dphi =
      c.element_fe_var[u_var]->get_dphi();

    // The number of local degrees of freedom in each variable
    const unsigned int n_u_dofs = c.dof_indices_var[u_var].size();

    // Now we will build the affine operator
    unsigned int n_qpoints = (c.get_element_qrule())->n_points();

    for (unsigned int qp=0; qp != n_qpoints; qp++)
      for (unsigned int i=0; i != n_u_dofs; i++)
        for (unsigned int j=0; j != n_u_dofs; j++)
          c.elem_jacobian(i,j) += JxW[qp] * dphi[j][qp]*dphi[i][qp];
  }
};

struct Ex6EIMInnerProduct : ElemAssembly
{

  // Use the L2 norm to find the best fit
  virtual void interior_assembly(FEMContext &c)
  {
    const unsigned int Gx_var = 0;
    const unsigned int Gy_var = 1;
    const unsigned int Gz_var = 2;

    const std::vector<Real> &JxW =
      c.element_fe_var[Gx_var]->get_JxW();

    const std::vector<std::vector<Real> >& phi =
      c.element_fe_var[Gx_var]->get_phi();

    const unsigned int n_u_dofs = c.dof_indices_var[Gx_var].size();

    unsigned int n_qpoints = (c.get_element_qrule())->n_points();
    
    DenseSubMatrix<Number>& Kxx = *c.elem_subjacobians[Gx_var][Gx_var];
    DenseSubMatrix<Number>& Kyy = *c.elem_subjacobians[Gy_var][Gy_var];
    DenseSubMatrix<Number>& Kzz = *c.elem_subjacobians[Gz_var][Gz_var];

    for (unsigned int qp=0; qp != n_qpoints; qp++)
      for (unsigned int i=0; i != n_u_dofs; i++)
        for (unsigned int j=0; j != n_u_dofs; j++)
        {
          Kxx(i,j) += JxW[qp] * phi[j][qp]*phi[i][qp];
          Kyy(i,j) += JxW[qp] * phi[j][qp]*phi[i][qp];
          Kzz(i,j) += JxW[qp] * phi[j][qp]*phi[i][qp];
        }
  }
};

// Define an RBThetaExpansion class for this PDE
// The A terms depend on EIM, so we deal with them later
struct Ex6ThetaExpansion : RBThetaExpansion
{

  /**
   * Constructor.
   */
  Ex6ThetaExpansion()
  {
    attach_A_theta(&theta_a0);
    attach_A_theta(&theta_a1);
    attach_A_theta(&theta_a2);
    attach_F_theta(&theta_f0); // Attach the rhs theta
    attach_F_theta(&theta_f1);
  }

  // The RBTheta member variables
  ThetaA0 theta_a0;
  ThetaA1 theta_a1;
  ThetaA2 theta_a2;
  ThetaF0 theta_f0;
  ThetaF1 theta_f1;
};

// Define an RBAssemblyExpansion class for this PDE
// The A terms depend on EIM, so we deal with them later
struct Ex6AssemblyExpansion : RBAssemblyExpansion
{

  /**
   * Constructor.
   */
  Ex6AssemblyExpansion(RBConstruction& rb_con)
  {
    // Point to the RBConstruction object
    assembly_a0.rb_con = &rb_con;
    assembly_a1.rb_con = &rb_con;
    assembly_a2.rb_con = &rb_con;
    
    attach_A_assembly(&assembly_a0);
    attach_A_assembly(&assembly_a1);
    attach_A_assembly(&assembly_a2);
    attach_F_assembly(&assembly_f0); // Attach the rhs assembly
    attach_F_assembly(&assembly_f1);
  }

  // The ElemAssembly objects
  AssemblyA0 assembly_a0;
  AssemblyA1 assembly_a1;
  AssemblyA2 assembly_a2;
  AssemblyF0 assembly_f0;
  AssemblyF1 assembly_f1;
};

#endif


