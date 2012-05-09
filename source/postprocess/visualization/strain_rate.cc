/*
  Copyright (C) 2011, 2012 by the authors of the ASPECT code.

  This file is part of ASPECT.

  ASPECT is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  ASPECT is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ASPECT; see the file doc/COPYING.  If not see
  <http://www.gnu.org/licenses/>.
*/
/*  $Id$  */


#include <aspect/postprocess/visualization.h>
#include <aspect/simulator.h>

#include <deal.II/numerics/data_out.h>


namespace aspect
{
  namespace Postprocess
  {
    namespace VisualizationPostprocessors
    {
      /**
       * A class derived from DataPostprocessor that takes an output vector and
       * computes a variable that represents the strain rate at every point. The
       * scalar strain rate is defined as $\\sqrt{
       *    (\\varepsilon - \\tfrac 13 \\textrm{trace}\\ \\varepsilon \\mathbf 1)
       *  : \\varepsilon - \\tfrac 13 \\textrm{trace}\\ \\varepsilon \\mathbf 1}$.
       *
       * The member functions are all implementations of those declared in the base
       * class. See there for their meaning.
       */
      template <int dim>
      class StrainRate
        : public DataPostprocessorScalar<dim>,
          public SimulatorAccess<dim>,
          public Interface<dim>
      {
        public:
          StrainRate ();

          virtual
          void
          compute_derived_quantities_vector (const std::vector<Vector<double> >              &uh,
                                             const std::vector<std::vector<Tensor<1,dim> > > &duh,
                                             const std::vector<std::vector<Tensor<2,dim> > > &dduh,
                                             const std::vector<Point<dim> >                  &normals,
                                             const std::vector<Point<dim> >                  &evaluation_points,
                                             std::vector<Vector<double> >                    &computed_quantities) const;
      };


      template <int dim>
      StrainRate<dim>::
      StrainRate ()
        :
        DataPostprocessorScalar<dim> ("strain_rate",
                                      update_gradients | update_q_points)
      {}



      template <int dim>
      void
      StrainRate<dim>::
      compute_derived_quantities_vector (const std::vector<Vector<double> >              &uh,
                                         const std::vector<std::vector<Tensor<1,dim> > > &duh,
                                         const std::vector<std::vector<Tensor<2,dim> > > &dduh,
                                         const std::vector<Point<dim> >                  &normals,
                                         const std::vector<Point<dim> >                  &evaluation_points,
                                         std::vector<Vector<double> >                    &computed_quantities) const
      {
        const unsigned int n_quadrature_points = uh.size();
        Assert (computed_quantities.size() == n_quadrature_points,  ExcInternalError());
        Assert (computed_quantities[0].size() == 1,                 ExcInternalError());
        Assert (uh[0].size() == dim+2,                              ExcInternalError());
        Assert (duh[0].size() == dim+2,                             ExcInternalError());

        for (unsigned int q=0; q<n_quadrature_points; ++q)
          {
            // extract the primal variables
            Tensor<2,dim> grad_u;
            for (unsigned int d=0; d<dim; ++d)
              grad_u[d] = duh[q][d];

            const SymmetricTensor<2,dim> strain_rate = symmetrize (grad_u);
            const SymmetricTensor<2,dim> compressible_strain_rate
              = (this->get_material_model().is_compressible()
                 ?
                 strain_rate - 1./3 * trace(strain_rate) * unit_symmetric_tensor<dim>()
                 :
                 strain_rate);
            computed_quantities[q](0) = std::sqrt(compressible_strain_rate *
                                                  compressible_strain_rate);
          }
      }
    }
  }
}


// explicit instantiations
namespace aspect
{
  namespace Postprocess
  {
    namespace VisualizationPostprocessors
    {
      ASPECT_REGISTER_VISUALIZATION_POSTPROCESSOR(StrainRate,
                                                  "strain rate",
                                                  "A visualization output object that generates output "
                                                  "for the norm of the strain rate, i.e., for the quantity "
                                                  "$\\sqrt{\\varepsilon(\\mathbf u):\\varepsilon(\\mathbf u)}$ "
                                                  "in the incompressible case and "
                                                  "$\\sqrt{[\\varepsilon(\\mathbf u)-\\tfrac 13(\\textrm{tr}\\;\\varepsilon(\\mathbf u)\\mathbf I]:"
                                                  "[\\varepsilon(\\mathbf u)-\\tfrac 13(\\textrm{tr}\\;\\varepsilon(\\mathbf u)\\mathbf I]}$ "
                                                  "in the compressible case.")
    }
  }
}