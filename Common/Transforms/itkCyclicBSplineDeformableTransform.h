/*=========================================================================
 *
 *  Copyright UMC Utrecht and contributors
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkCyclicBSplineDeformableTransform_h
#define itkCyclicBSplineDeformableTransform_h

#include "itkAdvancedTransform.h"
#include "itkImage.h"
#include "itkImageRegion.h"
#include "itkBSplineInterpolationWeightFunction2.h"
#include "itkBSplineInterpolationDerivativeWeightFunction.h"
#include "itkBSplineInterpolationSecondOrderDerivativeWeightFunction.h"
#include "itkAdvancedBSplineDeformableTransform.h"
#include "itkCyclicBSplineDeformableTransform.h"

namespace itk
{

/** \class CyclicBSplineDeformableTransform
 *
 * \brief Deformable transform using a B-spline representation in which the
 *   B-spline grid is formulated in a cyclic way.
 *
 * \ingroup Transforms
 */
template <class TScalarType = double,   // Data type for scalars
          unsigned int NDimensions = 3, // Number of dimensions
          unsigned int VSplineOrder = 3>
// Spline order
class ITK_TEMPLATE_EXPORT CyclicBSplineDeformableTransform
  : public AdvancedBSplineDeformableTransform<TScalarType, NDimensions, VSplineOrder>
{
public:
  /** Standard class typedefs. */
  typedef CyclicBSplineDeformableTransform                                           Self;
  typedef AdvancedBSplineDeformableTransform<TScalarType, NDimensions, VSplineOrder> Superclass;
  typedef SmartPointer<Self>                                                         Pointer;
  typedef SmartPointer<const Self>                                                   ConstPointer;

  /** New macro for creation of through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(CyclicBSplineDeformableTransform, AdvancedBSplineDeformableTransform);

  /** Dimension of the domain space. */
  itkStaticConstMacro(SpaceDimension, unsigned int, NDimensions);

  /** The B-spline order. */
  itkStaticConstMacro(SplineOrder, unsigned int, VSplineOrder);

  using typename Superclass::JacobianType;
  using typename Superclass::SpatialJacobianType;
  using typename Superclass::NonZeroJacobianIndicesType;
  using typename Superclass::JacobianOfSpatialJacobianType;
  using typename Superclass::SpatialHessianType;
  using typename Superclass::JacobianOfSpatialHessianType;
  using typename Superclass::InternalMatrixType;
  using typename Superclass::ParametersType;
  using typename Superclass::NumberOfParametersType;

  /** Parameters as SpaceDimension number of images. */
  typedef typename ParametersType::ValueType     PixelType;
  typedef Image<PixelType, Self::SpaceDimension> ImageType;
  typedef typename ImageType::Pointer            ImagePointer;

  using typename Superclass::RegionType;
  typedef typename RegionType::IndexType    IndexType;
  typedef typename RegionType::SizeType     SizeType;
  typedef typename ImageType::SpacingType   SpacingType;
  typedef typename ImageType::DirectionType DirectionType;
  typedef typename ImageType::PointType     OriginType;
  typedef typename RegionType::IndexType    GridOffsetType;
  using typename Superclass::InputPointType;
  using typename Superclass::OutputPointType;
  using typename Superclass::WeightsType;
  using typename Superclass::ParameterIndexArrayType;
  using typename Superclass::ContinuousIndexType;
  using typename Superclass::ScalarType;
  using typename Superclass::JacobianImageType;
  using typename Superclass::JacobianPixelType;
  using typename Superclass::WeightsFunctionType;
  typedef BSplineInterpolationWeightFunction2<ScalarType, Self::SpaceDimension - 1, Self::SplineOrder>
                                                               RedWeightsFunctionType;
  typedef typename RedWeightsFunctionType::ContinuousIndexType RedContinuousIndexType;

  /** This method specifies the region over which the grid resides. */
  void
  SetGridRegion(const RegionType & region) override;

  /** Transform points by a B-spline deformable transformation.
   * On return, weights contains the interpolation weights used to compute the
   * deformation and indices of the x (zeroth) dimension coefficient parameters
   * in the support region used to compute the deformation.
   * Parameter indices for the i-th dimension can be obtained by adding
   * ( i * this->GetNumberOfParametersPerDimension() ) to the indices array.
   */
  void
  TransformPoint(const InputPointType &    inputPoint,
                 OutputPointType &         outputPoint,
                 WeightsType &             weights,
                 ParameterIndexArrayType & indices,
                 bool &                    inside) const override;

  /** Compute the Jacobian of the transformation. */
  virtual void
  GetJacobian(const InputPointType & ipp, WeightsType & weights, ParameterIndexArrayType & indices) const;

  /** Compute the spatial Jacobian of the transformation. */
  void
  GetSpatialJacobian(const InputPointType & ipp, SpatialJacobianType & sj) const override;

protected:
  CyclicBSplineDeformableTransform();
  ~CyclicBSplineDeformableTransform() override = default;

  void
  ComputeNonZeroJacobianIndices(NonZeroJacobianIndicesType & nonZeroJacobianIndices,
                                const RegionType &           supportRegion) const override;

  /** Check if a continuous index is inside the valid region. */
  bool
  InsideValidRegion(const ContinuousIndexType & index) const override;

  /** Split an image region into two regions based on the last dimension. */
  virtual void
  SplitRegion(const RegionType & imageRegion,
              const RegionType & inRegion,
              RegionType &       outRegion1,
              RegionType &       outRegion2) const;

private:
  CyclicBSplineDeformableTransform(const Self &) = delete;
  void
  operator=(const Self &) = delete;
};

} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkCyclicBSplineDeformableTransform.hxx"
#endif

#endif /* itkCyclicBSplineDeformableTransform_h */
