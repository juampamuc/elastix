#ifndef __elxElastixTemplate_hxx
#define __elxElastixTemplate_hxx

#include "elxElastixTemplate.h"

namespace elastix
{
	using namespace itk;
	using namespace xl;
	
	
	/**
	 * ********************* Constructor ****************************
	 */
	
	template <class TFixedImage, class TMovingImage>
	ElastixTemplate<TFixedImage, TMovingImage>
	::ElastixTemplate()
	{
		/** Initialize CallBack commands. */
		this->m_BeforeEachResolutionCommand = 0;
		this->m_AfterEachIterationCommand = 0;

		/** Create timers. */
		this->m_Timer0 = TimerType::New();
		this->m_IterationTimer = TimerType::New();
		this->m_ResolutionTimer = TimerType::New();

		/** Initialize the this->m_IterationCounter. */
		this->m_IterationCounter = 0;

		/** Initialize CurrentTransformParameterFileName. */
		this->m_CurrentTransformParameterFileName = "";
		
	} // end Constructor
	
		
	/**
	 * ********************** GetFixedImage *************************
	 */
	
	template <class TFixedImage, class TMovingImage>
  typename ElastixTemplate<TFixedImage, TMovingImage>::FixedImageType *
  ElastixTemplate<TFixedImage, TMovingImage>
	::GetFixedImage( unsigned int idx ) const
	{
		if ( idx < this->GetNumberOfFixedImages() ) 
    { 
      return dynamic_cast< FixedImageType *>( 
        this->GetFixedImageContainer()->ElementAt(idx).GetPointer() );
    } 
    return 0; 		
	} // end SetFixedImage


	/**
	 * ********************** GetMovingImage *************************
	 */
	
	template <class TFixedImage, class TMovingImage>
  typename ElastixTemplate<TFixedImage, TMovingImage>::MovingImageType *
  ElastixTemplate<TFixedImage, TMovingImage>
	::GetMovingImage( unsigned int idx ) const
	{
		if ( idx < this->GetNumberOfMovingImages() ) 
    { 
      return dynamic_cast< MovingImageType *>( 
        this->GetMovingImageContainer()->ElementAt(idx).GetPointer() );
    } 
    return 0; 		
	} // end SetMovingImage


  /**
	 * ********************** GetFixedMask *************************
	 */
	
	template <class TFixedImage, class TMovingImage>
  typename ElastixTemplate<TFixedImage, TMovingImage>::FixedMaskType *
  ElastixTemplate<TFixedImage, TMovingImage>
	::GetFixedMask( unsigned int idx ) const
	{
		if ( idx < this->GetNumberOfFixedMasks() ) 
    { 
      return dynamic_cast< FixedMaskType *>( 
        this->GetFixedMaskContainer()->ElementAt(idx).GetPointer() );
    } 
    return 0; 		
	} // end SetFixedMask


	/**
	 * ********************** GetMovingMask *************************
	 */
	
	template <class TFixedImage, class TMovingImage>
  typename ElastixTemplate<TFixedImage, TMovingImage>::MovingMaskType *
  ElastixTemplate<TFixedImage, TMovingImage>
	::GetMovingMask( unsigned int idx ) const
	{
		if ( idx < this->GetNumberOfMovingMasks() ) 
    { 
      return dynamic_cast< MovingMaskType *>( 
        this->GetMovingMaskContainer()->ElementAt(idx).GetPointer() );
    } 
    return 0; 		
	} // end SetMovingMask
	

	/**
	 * **************************** Run *****************************
	 */
	
	template <class TFixedImage, class TMovingImage>
	int ElastixTemplate<TFixedImage, TMovingImage>
	::Run(void)
	{
		/** Tell all components where to find the ElastixTemplate and 
     * set there ComponentLabel. */
    this->ConfigureComponents( this );

		/** Call BeforeAll to do some checking. */
		int dummy = this->BeforeAll();
		if ( dummy != 0 ) return dummy;

		/** Setup Callbacks. This makes sure that the BeforeEachResolution()
		 * and AfterEachIteration() functions are called.
     *
     * NB: it is not yet clear what should happen when multiple registration
     * or optimizer components are used simultaneously. We won't use this
     * in the near future anyway, probably. */

		this->m_BeforeEachResolutionCommand = BeforeEachResolutionCommandType::New();
		this->m_AfterEachResolutionCommand = AfterEachResolutionCommandType::New();
		this->m_AfterEachIterationCommand = AfterEachIterationCommandType::New();
		
		this->m_BeforeEachResolutionCommand->SetCallbackFunction( this, &Self::BeforeEachResolution );
		this->m_AfterEachResolutionCommand->SetCallbackFunction( this, &Self::AfterEachResolution );
		this->m_AfterEachIterationCommand->SetCallbackFunction( this, &Self::AfterEachIteration );
		
		this->GetElxRegistrationBase()->GetAsITKBaseType()->
      AddObserver( itk::IterationEvent(), this->m_BeforeEachResolutionCommand );
		this->GetElxOptimizerBase()->GetAsITKBaseType()->
      AddObserver( itk::IterationEvent(), this->m_AfterEachIterationCommand );
		this->GetElxOptimizerBase()->GetAsITKBaseType()->
      AddObserver( itk::EndEvent(), this->m_AfterEachResolutionCommand );
	
		/** Start the timer for reading images. */
		this->m_Timer0->StartTimer();
		elxout << "\nReading images..." << std::endl;

    /** Read images and masks, if not set already. */
    if ( this->GetFixedImage() == 0 )
    {
      this->SetFixedImageContainer( 
        FixedImageLoaderType::GenerateImageContainer(
          this->GetFixedImageFileNameContainer(), "Fixed Image" )  );
    }
    if ( this->GetMovingImage() == 0 )
    {
      this->SetMovingImageContainer( 
        MovingImageLoaderType::GenerateImageContainer(
          this->GetMovingImageFileNameContainer(), "Moving Image" )  );
    }
    if ( this->GetFixedMask() == 0 )
    {
      this->SetFixedMaskContainer( 
        FixedMaskLoaderType::GenerateImageContainer(
          this->GetFixedMaskFileNameContainer(), "Fixed Mask" )  );
    }
    if ( this->GetMovingMask() == 0 )
    {
      this->SetMovingMaskContainer( 
        MovingMaskLoaderType::GenerateImageContainer(
          this->GetMovingMaskFileNameContainer(), "Moving Mask" )  );
    }
				
		/** Print the time spent on reading images. */
		this->m_Timer0->StopTimer();
		elxout << "Reading images took " <<	static_cast<unsigned long>(
			this->m_Timer0->GetElapsedClockSec() * 1000 ) << " ms.\n" << std::endl;

		/** Give all components the opportunity to do some initialization. */
		this->BeforeRegistration();
	
		/** START! */
		try
		{
			( this->GetElxRegistrationBase()->GetAsITKBaseType() )->StartRegistration();
		}
		catch( itk::ExceptionObject & excp )
		{
			/** Add information to the exception. */
			excp.SetLocation( "ElastixTemplate - Run()" );
			std::string err_str = excp.GetDescription();
			err_str += "\nError occured during actual registration.\n";
			excp.SetDescription( err_str );
			/** Pass the exception to an higher level. */
			throw excp;
		}
		
		/** Save, show results etc. */
		this->AfterRegistration();
		
		/** Make sure that the transform has stored the final parameters. 
		 *
		 * The transform may be used as a transform in a next elastixLevel;
		 * We need to be sure that it has the final parameters set. 
		 * In the AfterRegistration-method of TransformBase, this method is
		 * already called, but some other component may change the parameters
		 * again in its AfterRegistration-method.
		 *
		 * For now we leave it commented, since there is only Resampler, which 
		 * already calls this method. Calling it again would just take time.
		 */
		//this->GetElxTransformBase()->SetFinalParameters();

    /** Set the first transform as the final transform. This means that 
     * the other transforms should be set as an initial transform of this
     * transforms. However, up to now, multiple transforms are not really
     * supported yet. */
    this->SetFinalTransform( this->GetTransformContainer()->ElementAt(0) );

		/** Decouple the components from Elastix. This increases the chance that
		 * some memory is released. */				
		this->ConfigureComponents( 0 );

		/** Return a value. */
		return 0;
		
	} // end Run
	
	
	/**
	 * ************************ ApplyTransform **********************
	 */
	
	template <class TFixedImage, class TMovingImage>
	int ElastixTemplate<TFixedImage, TMovingImage>
	::ApplyTransform(void)
	{
		/** Timer. */
		tmr::Timer::Pointer timer = tmr::Timer::New();

		/** Tell all components where to find the ElastixTemplate. */
    this->ConfigureComponents( this );

		/** Call BeforeAllTransformix to do some checking. */
		int dummy = this->BeforeAllTransformix();
		if ( dummy != 0 ) return dummy;

    /** Set the inputImage (=movingImage).
     * If "-in" was given or an input image was given in some other way, load the image. */
    if ( (this->GetNumberOfMovingImageFileNames() > 0) || (this->GetMovingImage() != 0) )
		{
			/** Timer. */
			timer->StartTimer();

			/** Tell the user. */
      elxout << std::endl << "Reading input image ..." << std::endl;
			
			/** Load the image from disk, if it wasn't set already by the user */
      if ( this->GetMovingImage() == 0 )
      {
        this->SetMovingImageContainer( 
          MovingImageLoaderType::GenerateImageContainer(
            this->GetMovingImageFileNameContainer(), "Input Image" )  );
      } // end if !moving image

			/** Tell the user. */
			timer->StopTimer();
			elxout << "  Reading input image took "
				<< timer->PrintElapsedTimeSec()
				<< " s" << std::endl;

		} // end if inputImageFileName

		/** Call all the ReadFromFile() functions. */
		timer->StartTimer();
    elxout << "Calling all ReadFromFile()'s ..." << std::endl;
		this->GetElxResampleInterpolatorBase()->ReadFromFile();		
		this->GetElxResamplerBase()->ReadFromFile();
		this->GetElxTransformBase()->ReadFromFile();

		/** Tell the user. */
		timer->StopTimer();
		elxout << "  Calling all ReadFromFile()'s took "
			<< timer->PrintElapsedTimeSec()
			<< " s" << std::endl;
		timer->StartTimer();
		elxout << "Transforming points ..." << std::endl;

		/** Call TransformPoints.
     * Actually we could loop over all transforms.
     * But for now, there seems to be no use yet for that.  */
		try
		{
      this->GetElxTransformBase()->TransformPoints();
		}
		catch( itk::ExceptionObject & excp )
		{
			xout["error"] << excp << std::endl;
			xout["error"] << "However, transformix continues anyway with resampling." << std::endl;
		}
		timer->StopTimer();
		elxout << "  Transforming points done, it took "
			<< timer->PrintElapsedTimeSec()
			<< " s" << std::endl;

		/** Resample the image. */
		if ( this->GetMovingImage() != 0 )
		{
			timer->StartTimer();
      elxout << "Resampling image and writing to disk ..." << std::endl;
			
			/** Create a name for the final result. */
			std::string resultImageFormat = "mhd";
			this->GetConfiguration()->ReadParameter(	resultImageFormat, "ResultImageFormat", 0, true );
			std::ostringstream makeFileName("");
			makeFileName << this->GetConfiguration()->GetCommandLineArgument( "-out" )
				<< "result." << resultImageFormat;
			
			/** Write the resampled image to disk. 
       * Actually we could loop over all resamplers.
       * But for now, there seems to be no use yet for that.  */
			this->GetElxResamplerBase()->WriteResultImage( makeFileName.str().c_str() );

			/** Tell the user. */
			timer->StopTimer();
			elxout << "  Resampling took "
				<< timer->PrintElapsedTimeSec()
				<< " s" << std::endl;
		}

		/** Return a value. */
		return 0;

	} // end ApplyTransform
	

	/**
	 * ************************ BeforeAll ***************************
	 */
	
	template <class TFixedImage, class TMovingImage>
	int ElastixTemplate<TFixedImage, TMovingImage>
	::BeforeAll(void)
	{
		/** Declare the return value and initialize it. */
		int returndummy = 0;
		
		/** Call all the BeforeRegistration() functions. */
		returndummy |= this->BeforeAllBase();
		returndummy |= CallInEachComponentInt( &BaseComponentType::BeforeAllBase );
		returndummy |= CallInEachComponentInt( &BaseComponentType::BeforeAll );

		/** Return a value. */
		return returndummy;
		
	} // end BeforeAll


	/**
	 * ******************** BeforeAllTransformix ********************
	 */
	
	template <class TFixedImage, class TMovingImage>
	int ElastixTemplate<TFixedImage, TMovingImage>
	::BeforeAllTransformix(void)
	{
		/** Declare the return value and initialize it. */
		int returndummy = 0;

    /** Call the BeforeAllTransformixBase function in ElastixBase.
     * It checks most of the parameters. For now, it is the only
     * component that has a BeforeAllTranformixBase() method. */
		returndummy |= this->BeforeAllTransformixBase();
		
		/** Call all the BeforeAllTransformix() functions. 
     * Actually we could loop over all resample interpolators, resamplers,
     * and transforms etc. But for now, there seems to be no use yet for that.  */
		returndummy |= this->GetElxResampleInterpolatorBase()->BeforeAllTransformix();		
		returndummy |= this->GetElxResamplerBase()->BeforeAllTransformix();
		returndummy |= this->GetElxTransformBase()->BeforeAllTransformix();

		/** The GetConfiguration also has a BeforeAllTransformix, 
     * It print the Transform Parameter file to the log file. That's
     * why we call it after the other components. */
    returndummy |= this->GetConfiguration()->BeforeAllTransformix();
    
		/** Return a value. */
		return returndummy;

	} // end BeforeAllTransformix

	
	/**
	 * **************** BeforeRegistration Callback *****************
	 */
	
	template <class TFixedImage, class TMovingImage>
	void ElastixTemplate<TFixedImage, TMovingImage>
	::BeforeRegistration(void)
	{
		/** Start timer for initializing all components. */
		this->m_Timer0->StartTimer();
		
		/** Call all the BeforeRegistration() functions. */
		this->BeforeRegistrationBase();
		CallInEachComponent( &BaseComponentType::BeforeRegistrationBase );
		CallInEachComponent( &BaseComponentType::BeforeRegistration );

		/** Add a column to iteration with the iteration number. */
		xout["iteration"].AddTargetCell("1:ItNr");

		/** Add a column to iteration with timing information. */
		xout["iteration"].AddTargetCell("Time[ms]");

		/** Print time for initializing. */
		this->m_Timer0->StopTimer();
		elxout << "Initialization of all components (before registration) took: "
			<< static_cast<unsigned long>( this->m_Timer0->GetElapsedClockSec() * 1000 ) << " ms.\n";

		/** Start Timer0 here, to make it possible to measure the time needed for 
		 * preparation of the first resolution.
		 */
		this->m_Timer0->StartTimer();

	} // end BeforeRegistration Callback


	/**
	 * ************** BeforeEachResolution Callback *****************
	 */
	
	template <class TFixedImage, class TMovingImage>
	void ElastixTemplate<TFixedImage, TMovingImage>
	::BeforeEachResolution(void)
	{
		/** Get current resolution level. */
		unsigned long level =
			this->GetElxRegistrationBase()->GetAsITKBaseType()->GetCurrentLevel();

		if ( level == 0 )
		{
			this->m_Timer0->StopTimer();
			elxout << "Preparation of the image pyramids took: "
				<< static_cast<unsigned long>( this->m_Timer0->GetElapsedClockSec() * 1000 ) << " ms.\n";
			this->m_Timer0->StartTimer();
		}
		
		/** Reset the this->m_IterationCounter. */
		this->m_IterationCounter = 0;


		/** Print the current resolution. */
		elxout << "\nResolution: " <<	level	<< std::endl;

		this->OpenIterationInfoFile();

		/** Call all the BeforeEachResolution() functions. */
		this->BeforeEachResolutionBase();
		CallInEachComponent( &BaseComponentType::BeforeEachResolutionBase );
		CallInEachComponent( &BaseComponentType::BeforeEachResolution );
				
		/** Print the extra preparation time needed for this resolution. */
		this->m_Timer0->StopTimer();
		elxout << "Elastix initialization of all components (for this resolution) took: "
			<< static_cast<unsigned long>( this->m_Timer0->GetElapsedClockSec() * 1000 ) << " ms.\n";

		/** Start ResolutionTimer, which measures the total iteration time in this resolution. */
		this->m_ResolutionTimer->StartTimer();

		/** Start IterationTimer here, to make it possible to measure the time
		 * of the first iteration.
		 */
		this->m_IterationTimer->StartTimer();


	} // end BeforeEachResolution Callback


	/**
	 * ************** AfterEachResolution Callback *****************
	 */
	
	template <class TFixedImage, class TMovingImage>
	void ElastixTemplate<TFixedImage, TMovingImage>
	::AfterEachResolution(void)
	{
		/** Get current resolution level. */
		unsigned long level =
			this->GetElxRegistrationBase()->GetAsITKBaseType()->GetCurrentLevel();

    /** Print the total iteration time. */
		elxout << std::setprecision(3);
		this->m_ResolutionTimer->StopTimer();
		elxout 
			<< "Time spent in resolution " 
			<< ( level )
			<< " (ITK initialisation and iterating): "
			<< this->m_ResolutionTimer->GetElapsedClockSec()
			<< " s.\n";
		elxout << std::setprecision( this->GetDefaultOutputPrecision() );
		
		/** Call all the AfterEachResolution() functions. */
		this->AfterEachResolutionBase();
		CallInEachComponent( &BaseComponentType::AfterEachResolutionBase );
		CallInEachComponent( &BaseComponentType::AfterEachResolution );

		/** Create a TransformParameter-file for the current resolution. */
    bool writeTransformParameterEachResolution = false;
    this->GetConfiguration()->ReadParameter( writeTransformParameterEachResolution,
      "WriteTransformParametersEachResolution", 0, true );
		if ( writeTransformParameterEachResolution )
		{
			/** Create the TransformParameters filename for this resolution. */
			std::ostringstream makeFileName("");
			makeFileName << this->m_Configuration->GetCommandLineArgument( "-out" )
				<< "TransformParameters."
				<< this->GetConfiguration()->GetElastixLevel()
				<< ".R" << this->GetElxRegistrationBase()->GetAsITKBaseType()->GetCurrentLevel()
				<< ".txt";
			std::string FileName = makeFileName.str();
			
			/** Create a TransformParameterFile for this iteration. */
			this->CreateTransformParameterFile( FileName, false );
		}

		/** Start Timer0 here, to make it possible to measure the time needed for:
		 *    - executing the BeforeEachResolution methods (if this was not the last resolution)
		 *		- executing the AfterRegistration methods (if this was the last resolution)
		 */
		this->m_Timer0->StartTimer();
	
	} // end AfterEachResolution Callback
	

	/**
	 * ************** AfterEachIteration Callback *******************
	 */
	
	template <class TFixedImage, class TMovingImage>
	void ElastixTemplate<TFixedImage, TMovingImage>
	::AfterEachIteration(void)
	{
		/** Write the headers of the colums that are printed each iteration. */
		if ( this->m_IterationCounter == 0 )
		{
			xout["iteration"]["WriteHeaders"];
		}

		/** Call all the AfterEachIteration() functions. */
		this->AfterEachIterationBase();
		CallInEachComponent( &BaseComponentType::AfterEachIterationBase );
		CallInEachComponent( &BaseComponentType::AfterEachIteration );

		/** Write the iteration number to the table. */
		xout["iteration"]["1:ItNr"] << m_IterationCounter;

		/** Time in this iteration. */
		this->m_IterationTimer->StopTimer();
		xout["iteration"]["Time[ms]"]
			<< static_cast<unsigned long>( this->m_IterationTimer->GetElapsedClockSec() *1000 );

		/** Write the iteration info of this iteration. */
		xout["iteration"].WriteBufferedData();

    /** Create a TransformParameter-file for the current iteration. */
    bool writeTansformParametersThisIteration = false;
    this->GetConfiguration()->ReadParameter( writeTansformParametersThisIteration,
      "WriteTransformParametersEachIteration", 0, true );
    //this->GetConfiguration()->ReadParameter( writeTansformParametersThisIteration,
      //"WriteTransformParametersEachIteration", level, true );
		if ( writeTansformParametersThisIteration )
		{
			/** Add zeros to the number of iterations, to make sure 
			 * it always consists of 7 digits. 
       * \todo: use sprintf for this. it's much easier. or a formatting string for the 
       * ostringstream, if that's possible somehow.  */
			std::ostringstream makeIterationString("");
			unsigned int border = 1000000;
			while ( border > 1 )
			{
				if ( this->m_IterationCounter < border )
				{
					makeIterationString << "0";
					border /= 10;
				}
				else
				{
					/** Stop. */
					border = 1;
				}
			}
			makeIterationString << this->m_IterationCounter;

			/** Create the TransformParameters filename for this iteration. */
			std::ostringstream makeFileName("");
			makeFileName << this->GetConfiguration()->GetCommandLineArgument( "-out" )
				<< "TransformParameters."
				<< this->GetConfiguration()->GetElastixLevel()
				<< ".R" << this->GetElxRegistrationBase()->GetAsITKBaseType()->GetCurrentLevel()
				<< ".It" << makeIterationString.str()
				<< ".txt";
			std::string tpFileName = makeFileName.str();
			
			/** Create a TransformParameterFile for this iteration. */
			this->CreateTransformParameterFile( tpFileName, false );
		}
		
		/** Count the number of iterations. */
		this->m_IterationCounter++;

		/** Start timer for next iteration. */
		this->m_IterationTimer->StartTimer();
		
	} // end AfterEachIteration Callback
	
	
	/**
	 * ************** AfterRegistration Callback *******************
	 */
	
	template <class TFixedImage, class TMovingImage>
	void ElastixTemplate<TFixedImage, TMovingImage>
	::AfterRegistration(void)
	{
		/** A white line */
    elxout << std::endl;
		
		/** Create the final TransformParameters filename. */
		std::ostringstream makeFileName("");
		makeFileName << this->GetConfiguration()->GetCommandLineArgument( "-out" )
			<< "TransformParameters."
			<< this->GetConfiguration()->GetElastixLevel() << ".txt";
		std::string FileName = makeFileName.str();

		/** Create a final TransformParameterFile. */
		this->CreateTransformParameterFile( FileName, true );

		/** Call all the AfterRegistration() functions. */
		this->AfterRegistrationBase();
		CallInEachComponent( &BaseComponentType::AfterRegistrationBase );
		CallInEachComponent( &BaseComponentType::AfterRegistration );

		/** Print the time spent on things after the registration. */
		this->m_Timer0->StopTimer();
		elxout << "Time spent on saving the results, applying the final transform etc.: "
			<< static_cast<unsigned long>( this->m_Timer0->GetElapsedClockSec() * 1000 ) << " ms.\n";
		
	} // end AfterRegistration Callback
	

	/**
	 * ************** CreateTransformParameterFile ******************
	 *
	 * Setup the xout transform parameter file, which will
	 * contain the final transform parameters.
	 */
	
	template <class TFixedImage, class TMovingImage>
		void ElastixTemplate<TFixedImage, TMovingImage>
		::CreateTransformParameterFile( std::string FileName, bool ToLog )
	{
		using namespace xl;

		/** Store CurrentTransformParameterFileName. */
		this->m_CurrentTransformParameterFileName = FileName;

		/** Create transformParameterFile and xout["transpar"]. */
		xoutsimple_type		transformationParameterInfo;
		std::ofstream			transformParameterFile;
		
		/** Set up the "TransformationParameters" writing field. */
		transformationParameterInfo.SetOutputs( xout.GetCOutputs() );
		transformationParameterInfo.SetOutputs( xout.GetXOutputs() );
		
		xout.AddTargetCell( "transpar", &transformationParameterInfo );
		
		/** Set it in the Transform, for later use. */
		this->GetElxTransformBase()->SetTransformParametersFileName( FileName.c_str() );
		
		/** Open the TransformParameter file. */
		transformParameterFile.open( FileName.c_str() );
		if ( !transformParameterFile.is_open() )
		{
			xout["error"] << "ERROR: File \"" << FileName << "\" could not be opened!" << std::endl;
		}
		
		/** This xout["transpar"] writes to the log and to the TransformParameter file. */
		transformationParameterInfo.RemoveOutput( "cout" );
		transformationParameterInfo.AddOutput( "tpf", &transformParameterFile );
		if ( !ToLog )
		{
			transformationParameterInfo.RemoveOutput( "log" );
		}
		
		/** Format specifiers of the transformation parameter file. */
		xout["transpar"] << std::showpoint;
		xout["transpar"] << std::fixed;
		xout["transpar"] << std::setprecision( this->GetDefaultOutputPrecision() );
		
		/** Separate clearly in log-file. */
		if ( ToLog )
		{
			xout["logonly"] <<
				"\n=============== start of TransformParameterFile ==============="	<< std::endl;
		}

		/** Call all the WriteToFile() functions. 
     * Actually we could loop over all resample interpolators, resamplers,
     * and transforms etc. But for now, there seems to be no use yet for that.  */
		this->GetElxTransformBase()->WriteToFile(
			this->GetElxOptimizerBase()->GetAsITKBaseType()->GetCurrentPosition() );
		this->GetElxResampleInterpolatorBase()->WriteToFile();
		this->GetElxResamplerBase()->WriteToFile();
		
		/** Separate clearly in log-file. */
		if ( ToLog )
		{
			xout["logonly"] << 
				"\n=============== end of TransformParameterFile ===============" << std::endl;
		}

		/** Remove the "transpar" writing field. */
		xout.RemoveTargetCell( "transpar" );
		
	} // end CreateTransformParameterFile


	/**
	 * ****************** CallInEachComponent ***********************
	 */
	
	template <class TFixedImage, class TMovingImage>
	void ElastixTemplate<TFixedImage, TMovingImage>
	::CallInEachComponent( PtrToMemberFunction func )
	{
		/** Call the memberfunction 'func' of all components. */
		(  ( *(this->GetConfiguration()) ).*func  )();

    for (unsigned int i = 0; i < this->GetNumberOfRegistrations(); ++i)
    {  (  ( *(this->GetElxRegistrationBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfTransforms(); ++i)
    {  (  ( *(this->GetElxTransformBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfImageSamplers(); ++i)
    {  (  ( *(this->GetElxImageSamplerBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfMetrics(); ++i)
    {  (  ( *(this->GetElxMetricBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfInterpolators(); ++i)
    {  (  ( *(this->GetElxInterpolatorBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfOptimizers(); ++i)
    {  (  ( *(this->GetElxOptimizerBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfFixedImagePyramids(); ++i)
    {  (  ( *(this->GetElxFixedImagePyramidBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfMovingImagePyramids(); ++i)
    {  (  ( *(this->GetElxMovingImagePyramidBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfResampleInterpolators(); ++i)
    {  (  ( *(this->GetElxResampleInterpolatorBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfResamplers(); ++i)
    {  (  ( *(this->GetElxResamplerBase(i)) ).*func  )(); }
		
	} // end CallInEachComponent
	

	/**
	 * ****************** CallInEachComponentInt ********************
	 */
	
	template <class TFixedImage, class TMovingImage>
	int ElastixTemplate<TFixedImage, TMovingImage>
	::CallInEachComponentInt( PtrToMemberFunction2 func )
	{
		/** Declare the return value and initialize it. */
		int returndummy = 0;

		/** Call the memberfunction 'func' of all components. */
		returndummy |= (  ( *( this->GetConfiguration()) ).*func  )();		
    
    for (unsigned int i = 0; i < this->GetNumberOfRegistrations(); ++i)
    {  returndummy |= (  ( *(this->GetElxRegistrationBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfTransforms(); ++i)
    {  returndummy |= (  ( *(this->GetElxTransformBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfImageSamplers(); ++i)
    {  returndummy |= (  ( *(this->GetElxImageSamplerBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfMetrics(); ++i)
    {  returndummy |= (  ( *(this->GetElxMetricBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfInterpolators(); ++i)
    {  returndummy |= (  ( *(this->GetElxInterpolatorBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfOptimizers(); ++i)
    {  returndummy |= (  ( *(this->GetElxOptimizerBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfFixedImagePyramids(); ++i)
    {  returndummy |= (  ( *(this->GetElxFixedImagePyramidBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfMovingImagePyramids(); ++i)
    {  returndummy |= (  ( *(this->GetElxMovingImagePyramidBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfResampleInterpolators(); ++i)
    {  returndummy |= (  ( *(this->GetElxResampleInterpolatorBase(i)) ).*func  )(); }
    for (unsigned int i = 0; i < this->GetNumberOfResamplers(); ++i)
    {  returndummy |= (  ( *(this->GetElxResamplerBase(i)) ).*func  )(); }

		/** Return a value. */
		return returndummy;
				
	} // end CallInEachComponent


	/**
	 * ****************** ConfigureComponents *******************
   */
	
	template <class TFixedImage, class TMovingImage>
	void ElastixTemplate<TFixedImage, TMovingImage>
	::ConfigureComponents( Self * This )
	{
    this->GetConfiguration()->SetComponentLabel("Configuration", 0);

    for (unsigned int i = 0; i < this->GetNumberOfRegistrations(); ++i)
    { this->GetElxRegistrationBase(i)->SetComponentLabel( "Registration", i );
      this->GetElxRegistrationBase(i)->SetElastix( This ); }

    for (unsigned int i = 0; i < this->GetNumberOfTransforms(); ++i)
    { this->GetElxTransformBase(i)->SetComponentLabel( "Transform", i );
      this->GetElxTransformBase(i)->SetElastix( This ); }

    for (unsigned int i = 0; i < this->GetNumberOfImageSamplers(); ++i)
    { this->GetElxImageSamplerBase(i)->SetComponentLabel( "ImageSampler", i );
      this->GetElxImageSamplerBase(i)->SetElastix( This ); }

    for (unsigned int i = 0; i < this->GetNumberOfMetrics(); ++i)
    { this->GetElxMetricBase(i)->SetComponentLabel( "Metric", i );
      this->GetElxMetricBase(i)->SetElastix( This ); }

    for (unsigned int i = 0; i < this->GetNumberOfInterpolators(); ++i)
    { this->GetElxInterpolatorBase(i)->SetComponentLabel( "Interpolator", i );
      this->GetElxInterpolatorBase(i)->SetElastix( This ); }

    for (unsigned int i = 0; i < this->GetNumberOfOptimizers(); ++i)
    { this->GetElxOptimizerBase(i)->SetComponentLabel( "Optimizer", i );
      this->GetElxOptimizerBase(i)->SetElastix( This ); }

    for (unsigned int i = 0; i < this->GetNumberOfFixedImagePyramids(); ++i)
    { this->GetElxFixedImagePyramidBase(i)->SetComponentLabel( "FixedImagePyramid", i );
      this->GetElxFixedImagePyramidBase(i)->SetElastix( This ); }

    for (unsigned int i = 0; i < this->GetNumberOfMovingImagePyramids(); ++i)
    { this->GetElxMovingImagePyramidBase(i)->SetComponentLabel( "MovingImagePyramid", i );
      this->GetElxMovingImagePyramidBase(i)->SetElastix( This ); }

    for (unsigned int i = 0; i < this->GetNumberOfResampleInterpolators(); ++i)
    { this->GetElxResampleInterpolatorBase(i)->SetComponentLabel( "ResampleInterpolator", i );
      this->GetElxResampleInterpolatorBase(i)->SetElastix( This ); }

    for (unsigned int i = 0; i < this->GetNumberOfResamplers(); ++i)
    { this->GetElxResamplerBase(i)->SetComponentLabel( "Resampler", i );
      this->GetElxResamplerBase(i)->SetElastix( This ); }
				
	} // end ConfigureComponents


	/**
	 * ************** OpenIterationInfoFile *************************
	 *
	 * Open a file called IterationInfo.<ElastixLevel>.R<Resolution>.txt,
	 * which will contain the iteration info table.
	 */
	
	template <class TFixedImage, class TMovingImage>
		void ElastixTemplate<TFixedImage, TMovingImage>
		::OpenIterationInfoFile( void )
	{
		using namespace xl;
		
		/** Remove the current iteration info output file, if any. */
		xout["iteration"].RemoveOutput( "IterationInfoFile" );
		
		if ( this->m_IterationInfoFile.is_open() )
		{
			this->m_IterationInfoFile.close();
		}

		/** Create the IterationInfo filename for this resolution. */
		std::ostringstream makeFileName("");
		makeFileName << this->m_Configuration->GetCommandLineArgument( "-out" )
			<< "IterationInfo."
			<< this->m_Configuration->GetElastixLevel()
			<< ".R" << this->GetElxRegistrationBase()->GetAsITKBaseType()->GetCurrentLevel()
			<< ".txt";
		std::string FileName = makeFileName.str();

		/** Open the IterationInfoFile. */
		this->m_IterationInfoFile.open( FileName.c_str() );
		if ( !(this->m_IterationInfoFile.is_open()) )
		{
			xout["error"] << "ERROR: File \"" << FileName << "\" could not be opened!" << std::endl;
		}
		else    
		{
			/** Add this file to the list of outputs of xout["iteration"]. */
			xout["iteration"].AddOutput( "IterationInfoFile", &(this->m_IterationInfoFile) );
		}

	} // end of function OpenIterationInfoFile

} // end namespace elastix


#endif // end #ifndef __elxElastixTemplate_hxx

