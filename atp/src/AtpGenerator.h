//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************
#ifndef AtpGeneratorBase_H_
#define AtpGeneratorBase_H_

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageChain.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimAnnotationSource.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/reg/Image.h>
#include "AtpTileSource.h"
#include "AutoTiePoint.h"
#include "AtpAnnotatedImage.h"
#include <vector>
#include <memory>

namespace ATP
{

/**
 * Base class for OSSIM-based ATP generators.
 * TODO: Currently, ATP generation only works with pair-wise matching. Eventually it should
 * support N-way matching with an arbitrary number of input images. The multiple image case is
 * presently handled by the AutoTiePointService.
 */
class AtpGenerator : public std::enable_shared_from_this<AtpGenerator>
{
   friend class AtpTileSource;

public:
   enum Algorithm { ALGO_UNASSIGNED=0, CROSSCORR, DESCRIPTOR, NASA };

   AtpGenerator(Algorithm algo);

   virtual ~AtpGenerator();

   void setRefImage(std::shared_ptr<ossim::Image> ref_image);
   void setCmpImage(std::shared_ptr<ossim::Image> cmp_image);

   /** Needs to be called after ref and cmp images are set */
   virtual void initialize();

   /**
    * When the input images are multiband, the bands must be combined into a single-band image.
    * @param weights Vector of normalized weights (0-1) applied to the input bands for combining.
    */
   void setBandWeights(const std::vector<double> weights) { m_bandWeights = weights; }

   /**
    * This is the main workhorse method. It launches the auto-tie-point generation process given
    * the REF and CMP images.
    * TODO: Currently, ATP generation only works with pair-wise matching. Eventually it should
    * support N-way matching with an arbitrary number of input images.
    */
   virtual bool generateTiePointList(ossim::TiePointList& tpList);

   /**
    * For engineering use. Renders relevant tiepoint data to disk file(s). Optionally
    * accepts an LUT file for raster-based remapping.
    * @param outBaseName This is the base name of possible multiple outputs, as needed by derived
    * generator.
    * @param lutFile Optional LUT file formatted as specified in ossimIndexToRgbLutFilter.
    * @return True if successful.
    */
//   virtual bool renderTiePointDataAsImages(const ossimFilename& outBaseName,
//                                           const ossimFilename& lutFile=ossimFilename("")) = 0;

   /**
    * For engineering use. Convenience method for outputting tiepoint list to output stream.
    */
   static void writeTiePointList(std::ostream& out, const AtpList& tpList);

   ossimRefPtr<ossimImageViewProjectionTransform> getRefIVT() { return m_refIVT; }
   ossimRefPtr<ossimImageViewProjectionTransform> getCmpIVT() { return m_cmpIVT; }
   ossimRefPtr<ossimImageChain> getRefChain() { return m_refChain; }
   ossimRefPtr<ossimImageChain> getCmpChain() { return m_cmpChain; }

   std::string getRefImageID();
   std::string getCmpImageID();
   std::string getRefFilename();
   std::string getCmpFilename();

   ossimRefPtr<AtpTileSource> getAtpTileSource() { return m_atpTileSource; } // For engineering use
   ossimRefPtr<AtpAnnotatedImage> m_annotatedRefImage; // For engineering use
   ossimRefPtr<AtpAnnotatedImage> m_annotatedCmpImage; // For engineering use

protected:
   AtpGenerator() : m_algorithm (ALGO_UNASSIGNED), m_refEllipHgt(0) {}

   /**
    * Constructs the processing chain for the input image according to the needs of the generator.
    * As a convenience, the image-to-common view IVT is initialized
    */
   virtual ossimRefPtr<ossimImageChain>
   constructChain(std::shared_ptr<ossim::Image> image,
                  ossimRefPtr<ossimImageViewProjectionTransform>& ivt);

   //! Establishes valid image vertices in view space for later computing overlap.
   //  @return true if successful
   bool getValidVertices(ossimRefPtr<ossimImageChain> chain,
                         ossimRefPtr<ossimImageViewProjectionTransform>& ivt,
                         std::vector<ossimDpt>& validVertices);

   //! Finds optimum layout of patches within the intersect area for feature search.
   void layoutSearchTileRects(ossimPolygon& overlapPoly);

   ossimImageHandler* getImageHandler(ossimRefPtr<ossimImageChain>& chain);

   Algorithm m_algorithm;
   std::shared_ptr<ossim::Image> m_refImage;
   std::shared_ptr<ossim::Image> m_cmpImage;
   ossimRefPtr<AtpTileSource> m_atpTileSource;
   ossimRefPtr<ossimImageChain> m_refChain;
   ossimRefPtr<ossimImageChain> m_cmpChain;
   ossimRefPtr<ossimImageViewProjectionTransform> m_refIVT;
   ossimRefPtr<ossimImageViewProjectionTransform> m_cmpIVT;
   ossimRefPtr<ossimImageGeometry> m_viewGeom;
   ossimIrect m_aoiView;
   std::vector<double> m_bandWeights;
   double m_refEllipHgt;
   std::vector<ossimIrect> m_searchTileRects;
   static std::shared_ptr<AutoTiePoint> s_referenceATP;

};
}
#endif /* AtpGeneratorBase_H_ */
