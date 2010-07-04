/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                   http://mrpt.sourceforge.net/                            |
   |                                                                           |
   |   Copyright (C) 2005-2010  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */
#ifndef CFeature_H
#define CFeature_H

#include <mrpt/utils/CImage.h>
#include <mrpt/utils/stl_extensions.h>
#include <mrpt/math/CMatrix.h>
#include <mrpt/math/ops_matrices.h>
#include <mrpt/math/KDTreeCapable.h>

#include <mrpt/vision/link_pragmas.h>

namespace mrpt
{
	namespace vision
	{
		using namespace mrpt::utils;
		using namespace mrpt::math;

		class CFeatureList;
		class CMatchedFeatureList;

		/** Definition of a feature ID
		*/
		typedef uint64_t TFeatureID;

		/** Types of features - This means that the point has been detected with this algorithm, which is independent of additional descriptors a feature may also have
		*/
		enum TFeatureType
		{
			featNotDefined = -1,	//!< Non-defined feature (also used for Occupancy features)
			featKLT = 0,			//!< Kanade-Lucas-Tomasi feature [SHI'94]
			featHarris,				//!< Harris border and corner detector [HARRIS]
			featBCD,				//!< Binary corder detector
			featSIFT,				//!< Scale Invariant Feature Transform [LOWE'04]
			featSURF,				//!< Speeded Up Robust Feature [BAY'06]
			featBeacon,				//!< A especial case: this is not an image feature, but a 2D/3D beacon (used for range-only SLAM from mrpt::slam::CLandmark)
			featFAST				//!< FAST feature detector ("Faster and better: A machine learning approach to corner detection", E. Rosten, R. Porter and T. Drummond, PAMI, 2009).
		};

		/** The bitwise OR combination of values of TDescriptorType are used in CFeatureExtraction::computeDescriptors to indicate which descriptors are to be computed for features.
		  */
		enum TDescriptorType
		{
			descAny				= 0,  //!< Used in some methods to mean "any of the present descriptors"
			descSIFT            = 1,  //!< SIFT descriptors
			descSURF			= 2,  //!< SURF descriptors
			descSpinImages      = 4,  //!< Intensity-domain spin image descriptors
			descPolarImages     = 8,  //!< Polar image descriptor
			descLogPolarImages	= 16  //!< Log-Polar image descriptor
		};

		enum TFeatureTrackStatus
		{
			// Init value
			status_IDLE 	= 0,	//!< Inactive (right after detection, and before being tried to track)
			
			// Ok:
			status_TRACKED 	= 5,	//!< Feature correctly tracked

			// Bad:
			status_OOB		= 1,	//!< Feature felt Out Of Bounds
			status_LOST 	= 10,	//!< Unable to track this feature

			// KLT specific:
			statusKLT_IDLE 	= 0,	//!< Inactive
			statusKLT_OOB	= 1,	//!< Out Of Bounds	(Value identical to status_OOB)
			statusKLT_SMALL_DET	= 2,	//!< Determinant of the matrix too small
			statusKLT_LARGE_RESIDUE = 3,	//!< Error too big
			statusKLT_MAX_RESIDUE	= 4,
			statusKLT_TRACKED 	= 5,	//!< Feature correctly tracked (Value identical to status_TRACKED)
			statusKLT_MAX_ITERATIONS	= 6	//!< Iteration maximum reached
		};

		typedef TFeatureTrackStatus TKLTFeatureStatus; //!< For backward compatibility

		/****************************************************
						Class CFEATURE
		*****************************************************/
		DEFINE_SERIALIZABLE_PRE_CUSTOM_BASE_LINKAGE( CFeature, mrpt::utils::CSerializable, VISION_IMPEXP )

		/** A generic 2D feature from an image, extracted with \a CFeatureExtraction
		  * Each feature may have one or more descriptors (see \a descriptors), in addition to an image patch.
		  * The (Euclidean) distance between descriptors in a pair of features can be computed with  descriptorDistanceTo,
		  *  while the similarity of the patches is given by patchCorrelationTo.
		  */
		class VISION_IMPEXP CFeature : public mrpt::utils::CSerializable
		{
			friend class CFeatureList;
			friend class CMatchedFeatureList;

			DEFINE_SERIALIZABLE( CFeature )

		public:
			float				x,y;			//!< Coordinates in the image
			TFeatureID			ID;				//!< ID of the feature
			CImage				patch;			//!< A patch of the image surrounding the feature
			uint16_t			patchSize;		//!< Size of the patch (patchSize x patchSize) (it must be an odd number)
			TFeatureType		type;			//!< Type of the feature: featNotDefined, featSIFT, featKLT,	featHarris, featSURF, featBeacon
			TFeatureTrackStatus	track_status;	//!< Status of the feature tracking process (old name: KLT_status)
			float				response;		//!< A measure of the "goodness" of the feature (old name: KLT_val)
			float				orientation;	//!< Main orientation of the feature
			float				scale;			//!< Feature scale into the scale space
			uint8_t				IDSourceImage;	//!< ID of the image from which the feature was extracted.

			bool isPointFeature() const;		//!< Return false only for Blob detectors (SIFT, SURF)

			/** All the possible descriptors this feature may have */
			struct VISION_IMPEXP TDescriptors
			{
				TDescriptors();  // Initialization

				std::vector<unsigned char>	SIFT;			//!< Feature descriptor
				std::vector<float>			SURF;			//!< Feature descriptor
				std::vector<float>			SpinImg;		//!< The 2D histogram as a single row
				uint16_t					SpinImg_range_rows;  //!< The number of rows (corresponding to range bins in the 2D histogram) of the original matrix from which SpinImg was extracted as a vector.
				mrpt::math::CMatrix			PolarImg;		//!< A polar image centered at the interest point
				mrpt::math::CMatrix			LogPolarImg;	//!< A log-polar image centered at the interest point
				bool						polarImgsNoRotation; //!< If set to true (manually, default=false) the call to "descriptorDistanceTo" will not consider all the rotations between polar image descriptors (PolarImg, LogPolarImg)

				bool hasDescriptorSIFT() const { return !SIFT.empty(); };          //!< Whether this feature has this kind of descriptor
				bool hasDescriptorSURF() const { return !SURF.empty(); }          //!< Whether this feature has this kind of descriptor
				bool hasDescriptorSpinImg() const { return !SpinImg.empty(); };       //!< Whether this feature has this kind of descriptor
				bool hasDescriptorPolarImg() const { return size(PolarImg,1)>0; } ;      //!< Whether this feature has this kind of descriptor
				bool hasDescriptorLogPolarImg() const { return size(LogPolarImg,1)>0; } ;      //!< Whether this feature has this kind of descriptor
			}
			descriptors;

			/** Return the first found descriptor, as a matrix.
			  * \return false on error, i.e. there is no valid descriptor.
			  */
			bool getFirstDescriptorAsMatrix(mrpt::math::CMatrixFloat &desc) const;

			/** Computes the normalized cross-correlation between the patches of this and another feature (normalized in the range [0,1], such as 0=best, 1=worst).
			  *  \note If this or the other features does not have patches or they are of different sizes, an exception will be raised.
			  * \sa descriptorDistanceTo
			  */
			float patchCorrelationTo( const CFeature &oFeature) const;

			/** Computes the Euclidean Distance between this feature's and other feature's descriptors, using the given descriptor or the first present one.
			  *  \note If descriptorToUse is not descAny and that descriptor is not present in one of the features, an exception will be raised.
			  * \sa patchCorrelationTo
			  */
			float descriptorDistanceTo( const CFeature &oFeature,  TDescriptorType descriptorToUse = descAny, bool normalize_distances = true ) const;

			/** Computes the Euclidean Distance between "this" and the "other" descriptors */
			float descriptorSIFTDistanceTo( const CFeature &oFeature, bool normalize_distances = true ) const;

			/** Computes the Euclidean Distance between "this" and the "other" descriptors */
			float descriptorSURFDistanceTo( const CFeature &oFeature, bool normalize_distances = true  ) const;

			/** Computes the Euclidean Distance between "this" and the "other" descriptors */
			float descriptorSpinImgDistanceTo( const CFeature &oFeature, bool normalize_distances = true ) const;

			/** Returns the minimum Euclidean Distance between "this" and the "other" polar image descriptor, for the best shift in orientation.
			  * \param oFeature The other feature to compare with.
			  * \param minDistAngle The placeholder for the angle at which the smallest distance is found.
			  * \return The distance for the best orientation (minimum distance).
			  */
			float descriptorPolarImgDistanceTo(
				const CFeature &oFeature,
				float &minDistAngle,
				bool normalize_distances = true ) const;

			/** Returns the minimum Euclidean Distance between "this" and the "other" log-polar image descriptor, for the best shift in orientation.
			  * \param oFeature The other feature to compare with.
			  * \param minDistAngle The placeholder for the angle at which the smallest distance is found.
			  * \return The distance for the best orientation (minimum distance).
			  */
			float descriptorLogPolarImgDistanceTo(
				const CFeature &oFeature,
				float &minDistAngle,
				bool normalize_distances = true ) const;

			/** Get the type of the feature
			*/
			TFeatureType get_type() const { return type; }

			/** Constructor
			*/
			CFeature();

			/** Virtual destructor */
			virtual ~CFeature() {}


		protected:

			/** Internal function used by "descriptorLogPolarImgDistanceTo" and "descriptorPolarImgDistanceTo"
			  */
			static float internal_distanceBetweenPolarImages(
				const mrpt::math::CMatrix &desc1,
				const mrpt::math::CMatrix &desc2,
				float &minDistAngle,
				bool normalize_distances,
				bool dont_shift_angle );

		}; // end of class


		/****************************************************
						Class CFEATURELIST
		*****************************************************/
		/** A list of visual features, to be used as output by detectors, as input/output by trackers, etc.
		  */
		class VISION_IMPEXP CFeatureList : public mrpt::math::KDTreeCapable  //public std::deque<CFeaturePtr>
		{
		protected:
			typedef std::deque<CFeaturePtr> TInternalFeatList;

			TInternalFeatList  m_feats; //!< The actual container with the list of features

		public:
			/** The type of the first feature in the list */
			inline TFeatureType get_type() const { return empty() ? featNotDefined : (*begin())->get_type(); }

			/** Save feature list to a text file */
			void saveToTextFile( const std::string &fileName, bool APPEND = false );

			/** Save feature list to a text file */
			void loadFromTextFile( const std::string &fileName );

			/** Get the maximum ID into the list */
			TFeatureID getMaxID() const;

			/** Get a reference to a Feature from its ID */
			CFeaturePtr getByID( TFeatureID ID ) const;

			/** Get a reference to the nearest feature to the a given 2D point (version returning distance to closest feature in "max_dist")
			*   \param x [IN] The query point x-coordinate
			*   \param y [IN] The query point y-coordinate
			*   \param max_dist [IN/OUT] At input: The maximum distance to search for. At output: The actual distance to the feature.
			*  \return A reference to the found feature, or a NULL smart pointer if none found.
			*  \note See also all the available KD-tree search methods, listed in mrpt::math::KDTreeCapable
			*/
			CFeaturePtr nearest( const float x, const float y, double &max_dist ) const;

			/** Constructor */
			CFeatureList();

			/** Virtual destructor */
			virtual ~CFeatureList();

			/** @name Method and datatypes to emulate a STL container
			    @{ */
			typedef TInternalFeatList::iterator iterator;
			typedef TInternalFeatList::const_iterator const_iterator;

			typedef TInternalFeatList::reverse_iterator reverse_iterator;
			typedef TInternalFeatList::const_reverse_iterator const_reverse_iterator;

			inline iterator begin() { return m_feats.begin(); }
			inline iterator end() { return m_feats.end(); }
			inline const_iterator begin() const { return m_feats.begin(); }
			inline const_iterator end() const { return m_feats.end(); }

			inline iterator erase(const iterator it)  { return m_feats.erase(it); }

			inline bool empty() const  { return m_feats.empty(); }
			inline size_t size() const { return m_feats.size(); }

			inline void clear() { m_feats.clear(); }
			inline void resize(size_t N) { m_feats.resize(N); }

			inline void push_front(const CFeaturePtr &f) { m_feats.push_front(f); }
			inline void push_back(const CFeaturePtr &f) { m_feats.push_back(f); }

			inline CFeaturePtr & operator [](const unsigned int index) { return m_feats[index]; }
			inline const CFeaturePtr & operator [](const unsigned int index) const  { return m_feats[index]; }

			/** @} */

			/** @name Virtual methods that MUST be implemented by children classes of KDTreeCapable
			    @{ */

			/** Must return the number of data points */
			virtual size_t kdtree_get_point_count() const { return size(); }

			/** Must fill out the data points in "data", such as the i'th point will be stored in (data[i][0],...,data[i][nDims-1]). */
			virtual void kdtree_fill_point_data(ANNpointArray &data, const int nDims) const;
			/** @} */


		}; // end of class

		/****************************************************
					Class CMATCHEDFEATURELIST
		*****************************************************/
		/** A list of features
		*/
		class VISION_IMPEXP CMatchedFeatureList : public std::deque< std::pair<CFeaturePtr,CFeaturePtr> >
		{
		public:
			/** The type of the first feature in the list */
			inline TFeatureType get_type() const { return empty() ? featNotDefined : (begin()->first)->get_type(); }

			/** Save list of matched features to a text file */
			void saveToTextFile(const std::string &fileName);

			/** Constructor */
			CMatchedFeatureList();

			/** Virtual destructor */
			virtual ~CMatchedFeatureList();
		}; // end of class

	} // end of namespace

	namespace utils
	{
		using namespace ::mrpt::vision;
		// Specialization must occur in the same namespace
		MRPT_DECLARE_TTYPENAME_PTR(CFeature)
	}


} // end of namespace

#endif

