/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkAdvancedImageMomentsCalculator_h
#define itkAdvancedImageMomentsCalculator_h

#include "itkInPlaceImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkAffineTransform.h"
#include "itkImage.h"
#include "itkSpatialObject.h"
#include "itkImageGridSampler.h"
#include "itkImageFullSampler.h"

#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/vnl_diag_matrix.h"

#include "itkPlatformMultiThreader.h"

namespace itk
{
/** \class AdvancedImageMomentsCalculator
 * \brief Compute moments of an n-dimensional image.
 *
 * This class provides methods for computing the moments and related
 * properties of a single-echo image.  Computing the (non-central)
 * moments of a large image can easily take a million times longer
 * than computing the various other values derived from them, so we
 * compute the moments only on explicit request, and save their values
 * (in an AdvancedImageMomentsCalculator object) for later retrieval by the user.
 *
 * The non-central moments computed by this class are not really
 * intended for general use and are therefore in index coordinates;
 * that is, we pretend that the index that selects a particular
 * pixel also equals its physical coordinates.  The center of gravity,
 * central moments, principal moments and principal axes are all
 * more generally useful and are computed in the physical coordinates
 * defined by the Origin and Spacing parameters of the image.
 *
 * The methods that return values return the values themselves rather
 * than references because the cost is small compared to the cost of
 * computing the moments and doing so simplifies memory management for
 * the caller.
 *
 * \ingroup Operators
 *
 * \todo It's not yet clear how multi-echo images should be handled here.
 * \ingroup ITKImageStatistics
 */
template< typename TImage >
class AdvancedImageMomentsCalculator : public Object
{
public:
  /** Standard class typedefs. */
  typedef AdvancedImageMomentsCalculator< TImage > Self;
  typedef Object                           Superclass;
  typedef SmartPointer< Self >             Pointer;
  typedef SmartPointer< const Self >       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AdvancedImageMomentsCalculator, Object);

  /** Extract the dimension of the image. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TImage::ImageDimension);

  /** Standard scalar type within this class. */
  typedef double ScalarType;

  /** Standard vector type within this class. */
  typedef Vector< ScalarType, itkGetStaticConstMacro(ImageDimension) > VectorType;

  /** Spatial Object type within this class. */
  typedef SpatialObject< itkGetStaticConstMacro(ImageDimension) > SpatialObjectType;

  /** Spatial Object member types used within this class. */
  typedef typename SpatialObjectType::Pointer      SpatialObjectPointer;
  typedef typename SpatialObjectType::ConstPointer SpatialObjectConstPointer;

  /** Standard matrix type within this class. */
  typedef Matrix< ScalarType,
                  itkGetStaticConstMacro(ImageDimension),
                  itkGetStaticConstMacro(ImageDimension) >   MatrixType;

  /** Standard image type within this class. */
  typedef TImage ImageType;

  /** Standard image type pointer within this class. */
  typedef typename ImageType::Pointer      ImagePointer;
  typedef typename ImageType::ConstPointer ImageConstPointer;

  /** Affine transform for mapping to and from principal axis */
  typedef AffineTransform< double, itkGetStaticConstMacro(ImageDimension) > AffineTransformType;
  typedef typename AffineTransformType::Pointer                             AffineTransformPointer;

  /** Set the input image. */
  virtual void SetImage(const ImageType *image)
  {
    if ( m_Image != image )
      {
      m_Image = image;
      this->Modified();
      m_Valid = false;
      }
  }

  /** Set the spatial object mask. */
  virtual void SetSpatialObjectMask(const SpatialObject< itkGetStaticConstMacro(ImageDimension) > *so)
  {
    if ( m_SpatialObjectMask != so )
      {
      m_SpatialObjectMask = so;
      this->Modified();
      m_Valid = false;
      }
  }

  /** Compute moments of a new or modified image.
   * This method computes the moments of the image given as a
   * parameter and stores them in the object.  The values of these
   * moments and related parameters can then be retrieved by using
   * other methods of this object. */
  /** The multi-threading implementation. */
  void Compute();

  /** The main functions that performs the single thread computation. */
  void ComputeSingleThreaded();
  /** Return the total mass (or zeroth moment) of an image.
   * This method returns the sum of pixel intensities (also known as
   * the zeroth moment or the total mass) of the image whose moments
   * were last computed by this object. */
  ScalarType GetTotalMass() const;

  /** Return first moments about origin, in index coordinates.
   * This method returns the first moments around the origin of the
   * image whose moments were last computed by this object.  For
   * simplicity, these moments are computed in index coordinates
   * rather than physical coordinates. */
  VectorType GetFirstMoments() const;

  /** Return second moments about origin, in index coordinates.
   * This method returns the second moments around the origin
   * of the image whose moments were last computed by this object.
   * For simplicity, these moments are computed in index coordinates
   * rather than physical coordinates. */
  MatrixType GetSecondMoments() const;

  /** Return center of gravity, in physical coordinates.
   * This method returns the center of gravity of the image whose
   * moments were last computed by this object.  The center of
   * gravity is computed in physical coordinates. */
  VectorType GetCenterOfGravity() const;

  /** Return second central moments, in physical coordinates.
   * This method returns the central second moments of the image
   * whose moments were last computed by this object.  The central
   * moments are computed in physical coordinates. */
  MatrixType GetCentralMoments() const;

  /** Return principal moments, in physical coordinates.
   * This method returns the principal moments of the image whose
   * moments were last computed by this object.  The moments are
   * returned as a vector, with the principal moments ordered from
   * smallest to largest.  The moments are computed in physical
   * coordinates.   */
  VectorType GetPrincipalMoments() const;

  /** Return principal axes, in physical coordinates.
   * This method returns the principal axes of the image whose
   * moments were last computed by this object.  The moments are
   * returned as an orthogonal matrix, each row of which corresponds
   * to one principal moment; for example, the principal axis
   * corresponding to the smallest principal moment is the vector
   * m[0], where m is the value returned by this method.  The matrix
   * of principal axes is guaranteed to be a proper rotation; that
   * is, to have determinant +1 and to preserve parity.  (Unless you
   * have foolishly made one or more of the spacing values negative;
   * in that case, _you_ get to figure out the consequences.)  The
   * moments are computed in physical coordinates. */
  MatrixType GetPrincipalAxes() const;

  /** Get the affine transform from principal axes to physical axes
   * This method returns an affine transform which transforms from
   * the principal axes coordinate system to physical coordinates. */
  AffineTransformPointer GetPrincipalAxesToPhysicalAxesTransform() const;

  /** Get the affine transform from physical axes to principal axes
   * This method returns an affine transform which transforms from
   * the physical coordinate system to the principal axes coordinate
   * system. */
  AffineTransformPointer GetPhysicalAxesToPrincipalAxesTransform() const;

  /** Set the number of threads. */
  void SetNumberOfWorkUnits(ThreadIdType numberOfThreads)
  {
    this->m_Threader->SetNumberOfWorkUnits(numberOfThreads);
  }

  virtual void BeforeThreadedCompute( void );

  virtual void AfterThreadedCompute( void );

  typedef itk::ImageGridSampler< ImageType >     ImageGridSamplerType;
//  typedef itk::ImageFullSampler< ImageType >     ImageGridSamplerType;
  typedef typename ImageGridSamplerType::Pointer ImageGridSamplerPointer;
  typedef typename ImageGridSamplerType
    ::ImageSampleContainerType                   ImageSampleContainerType;
  typedef typename ImageSampleContainerType::Pointer ImageSampleContainerPointer;

  virtual void SampleImage(ImageSampleContainerPointer & sampleContainer);

  typedef itk::BinaryThresholdImageFilter < TImage, TImage >             BinaryThresholdImageFilterType;
  typedef typename TImage::PixelType  InputPixelType;

  /** Set some parameters. */
  itkSetMacro( NumberOfSamplesForCenteredTransformInitialization, SizeValueType );
  itkSetMacro( LowerThresholdForCenterGravity, InputPixelType );
  itkSetMacro( CenterOfGravityUsesLowerThreshold, bool );

protected:
  AdvancedImageMomentsCalculator();
  ~AdvancedImageMomentsCalculator() override;
  void PrintSelf(std::ostream & os, Indent indent) const override;

  /** Typedefs for multi-threading. */
  typedef itk::PlatformMultiThreader             ThreaderType;
  typedef ThreaderType::ThreadInfoStruct ThreadInfoType;
  ThreaderType::Pointer                   m_Threader;

  /** Launch MultiThread Compute. */
  void LaunchComputeThreaderCallback(void) const;

  /** Compute threader callback function. */
  static ITK_THREAD_RETURN_TYPE ComputeThreaderCallback(void * arg);

  /** The threaded implementation of Compute(). */
  virtual inline void ThreadedCompute(ThreadIdType threadID);

  /** Initialize some multi-threading related parameters. */
  virtual void InitializeThreadingParameters(void);

  /** To give the threads access to all member variables and functions. */
  struct MultiThreaderParameterType
  {
    Self * st_Self;
  };
  mutable MultiThreaderParameterType m_ThreaderParameters;

  struct ComputePerThreadStruct
  {
    /**  Used for accumulating variables. */
    ScalarType st_M0;                   // Zeroth moment for threading
    VectorType st_M1;                   // First moments about origin for threading
    MatrixType st_M2;                   // Second moments about origin for threading
    VectorType st_Cg;                   // Center of gravity (physical units) for threading
    MatrixType st_Cm;                   // Second central moments (physical) for threading
    SizeValueType st_NumberOfPixelsCounted;
  };
  itkPadStruct(ITK_CACHE_LINE_ALIGNMENT, ComputePerThreadStruct,
    PaddedComputePerThreadStruct);
  itkAlignedTypedef(ITK_CACHE_LINE_ALIGNMENT, PaddedComputePerThreadStruct,
    AlignedComputePerThreadStruct);
  mutable AlignedComputePerThreadStruct * m_ComputePerThreadVariables;
  mutable ThreadIdType                    m_ComputePerThreadVariablesSize;
  bool                        m_UseMultiThread;
  SizeValueType               m_NumberOfPixelsCounted;

  /** The type of region used for multithreading */
  typedef typename ImageType::RegionType ThreadRegionType;

  SizeValueType  m_NumberOfSamplesForCenteredTransformInitialization;
  InputPixelType m_LowerThresholdForCenterGravity;
  bool           m_CenterOfGravityUsesLowerThreshold;
  ImageSampleContainerPointer m_SampleContainer;

private:
  AdvancedImageMomentsCalculator( const Self & );
  void operator = ( const Self & );

  bool       m_Valid;                // Have moments been computed yet?
  ScalarType m_M0;                   // Zeroth moment
  VectorType m_M1;                   // First moments about origin
  MatrixType m_M2;                   // Second moments about origin
  VectorType m_Cg;                   // Center of gravity (physical units)
  MatrixType m_Cm;                   // Second central moments (physical)
  VectorType m_Pm;                   // Principal moments (physical)
  MatrixType m_Pa;                   // Principal axes (physical)

  ImageConstPointer         m_Image;
  SpatialObjectConstPointer m_SpatialObjectMask;

};  // class AdvancedImageMomentsCalculator
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAdvancedImageMomentsCalculator.hxx"
#endif

#endif /* itkAdvancedImageMomentsCalculator_h */
