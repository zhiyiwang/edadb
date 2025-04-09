//lefiVia.hpp
class lefiViaLayer {
protected:
  char* name_;
  int* rectColorMask_;
  int* polyColorMask_;
  int numRects_;
  int rectsAllocated_;
  double* xl_;
  double* yl_;
  double* xh_;
  double* yh_;

  int numPolys_;
  int polysAllocated_;
  lefiGeomPolygon** polygons_;
};

class lefiVia {

protected:
  char* name_;
  int nameSize_;

  int hasDefault_;
  int hasGenerated_;
  int hasResistance_;
  int hasForeignPnt_;
  int hasTopOfStack_;

  int numProps_;
  int propsAllocated_;
  char** propName_;
  // The prop value is stored in the propValue_ or the propDValue_.
  // If it is a string it is in propValue_.  If it is a number,
  // then propValue_ is NULL and it is stored in propDValue_;
  char** propValue_;
  double* propDValue_;
  char*   propType_;

  int numLayers_;
  int layersAllocated_;
  lefiViaLayer** layers_;

  double resistance_;

  char* foreign_;
  double foreignX_;
  double foreignY_;
  int foreignOrient_;

  char* viaRuleName_;          // 5.6
  double xSize_;               // 5.6
  double ySize_;               // 5.6
  char* botLayer_;             // 5.6
  char* cutLayer_;             // 5.6
  char* topLayer_;             // 5.6
  double xSpacing_;            // 5.6
  double ySpacing_;            // 5.6
  double xBotEnc_;             // 5.6
  double yBotEnc_;             // 5.6
  double xTopEnc_;             // 5.6
  double yTopEnc_;             // 5.6
  int numRows_;                // 5.6
  int numCols_;                // 5.6
  double xOffset_;             // 5.6
  double yOffset_;             // 5.6
  double xBotOs_;              // 5.6
  double yBotOs_;              // 5.6
  double xTopOs_;              // 5.6
  double yTopOs_;              // 5.6
  char* cutPattern_;           // 5.6
};


//lefiArray.hpp
class lefiArrayFloorPlan {
  public:
    void Init(const char* name);
    void Destroy();
    void addSitePattern(const char* typ, lefiSitePattern* s);
  
    int numPatterns() const;
    lefiSitePattern* pattern(int index) const;
    char* typ(int index) const;
    const char* name() const;
  
  protected:
    int numPatterns_;
    int patternsAllocated_;
    lefiSitePattern** patterns_;
    char** types_;
    char* name_;
  };
  
class lefiArray {
  public:
  protected:
    int nameSize_;
    char* name_;
  
    int patternsAllocated_;
    int numPatterns_;
    lefiSitePattern** pattern_;
  
    int canAllocated_;
    int numCan_;
    lefiSitePattern** canPlace_;
  
    int cannotAllocated_;
    int numCannot_;
    lefiSitePattern** cannotOccupy_;
  
    int tracksAllocated_;
    int numTracks_;
    lefiTrackPattern** track_;
  
    int gAllocated_;
    int numG_;
    lefiGcellPattern** gcell_;
  
    int hasDefault_;
    int tableSize_;
    int numDefault_;
    int defaultAllocated_;
    int* minPins_;
    double* caps_;
  
    int numFloorPlans_;
    int floorPlansAllocated_;
    lefiArrayFloorPlan** floors_;
  };

//lefiCrossTalk.hpp
struct lefiNoiseMargin {
  double high;
  double low;
};

class lefiNoiseVictim {
public:

protected:
  double length_;

  int numNoises_;
  int noisesAllocated_;
  double* noises_;
};

class lefiNoiseResistance {
public:
protected:
  int numNums_;
  int numsAllocated_;
  double* nums_;

  int numVictims_;
  int victimsAllocated_;
  lefiNoiseVictim** victims_;
};

class lefiNoiseEdge {
public:
protected:
  double edge_;

  int numResistances_;
  int resistancesAllocated_;
  lefiNoiseResistance** resistances_;
};

class lefiNoiseTable {
public:

protected:
  int num_;

  int numEdges_;
  int edgesAllocated_;
  lefiNoiseEdge** edges_;
};

class lefiCorrectionVictim {
public:
protected:
  double length_;

  int numCorrections_;
  int correctionsAllocated_;
  double* corrections_;
};

class lefiCorrectionResistance {
public:
protected:
  int numNums_;
  int numsAllocated_;
  double* nums_;

  int numVictims_;
  int victimsAllocated_;
  lefiCorrectionVictim** victims_;
};

class lefiCorrectionEdge {
public:
protected:
  double edge_;
 
  int numResistances_;
  int resistancesAllocated_;
  lefiCorrectionResistance** resistances_;
};

class lefiCorrectionTable {
public:
protected:
  int num_;

  int numEdges_;
  int edgesAllocated_;
  lefiCorrectionEdge** edges_;
};

//lefiDebug.hpp 疑似不需要
//lefiDefs.hpp
typedef struct point lefPOINT;

struct point {
    double x;
    double y;
    };

typedef struct rect lefRECT;

struct rect {
    lefPOINT ll,ur;
    };

typedef struct token lefTOKEN;

struct token {
    lefTOKEN *next;
    int what;
    int data;
    lefPOINT pt;
    };

#define START_LIST 10001
#define POINT_SPEC 10002
#define VIA_SPEC   10003
#define WIDTH_SPEC 10004
#define LAYER_SPEC 10005
#define SHAPE_SPEC 10006

#ifndef MAXINT
#define MAXINT 0x7FFFFFFF
#endif
#ifndef MININT
#define MININT 0x80000000
#endif

#ifndef    MIN
#define MIN(x,y) ((x) < (y)? (x) : (y))
#endif

#ifndef    MIN
#define MAX(x,y) ((x) > (y)? (x) : (y))
#endif

#define ROUND(x) ((x) >= 0 ? (int)((x)+0.5) : (int)((x)-0.5))

typedef int lefiBoolean;


typedef enum
{
  // decrease likelihood of accidentally correct values by starting
  // at an unusual number 
  lefiInvalidObject = 41713, 
  lefiUnknownObject // void * 
} lefiObjectType_e;

typedef enum
{
  // decrease likelihood of accidentally correct values by starting
  // at an unusual number 
  lefiInvalidMemoryPolicy = 23950,
  lefiPrivateSubObjects,      // deep copy + delete
  lefiReferencedSubObjects,   // shallow copy, no delete
  lefiOrphanSubObjects,       // deep copy, no delete
  lefiAdoptedSubObjects       // shallow copy + delete
} lefiMemoryPolicy_e;

#define lefiUserData void *
#define lefiUserDataHandle void **

//lefiEncryptInt.hpp
extern void lefrEnableReadEncrypted();

//lefiKRDefs.hpp

//lefiLayer.hpp

typedef enum lefiAntennaEnum {
  lefiAntennaAR,
  lefiAntennaDAR,
  lefiAntennaCAR,
  lefiAntennaCDAR,
  lefiAntennaAF,
  lefiAntennaSAR,
  lefiAntennaDSAR,
  lefiAntennaCSAR,
  lefiAntennaCDSAR,
  lefiAntennaSAF,
  lefiAntennaO,
  lefiAntennaADR
} lefiAntennaEnum;

class lefiAntennaPWL {
public:
protected:
  int numAlloc_;
  int numPWL_;
  double* d_;
  double* r_;
};

class lefiLayerDensity {
public:
protected:
  char* type_;
  double oneEntry_;
  int numFrequency_;
  double* frequency_;
  int numWidths_;
  double* widths_;
  int numTableEntries_;
  double* tableEntries_;
  int numCutareas_;
  double* cutareas_;
};

// 5.5
class lefiParallel {
public:
protected:
  int numLength_;
  int numWidth_;
  int numWidthAllocated_;
  double* length_;
  double* width_;
  double* widthSpacing_;
};

// 5.5
class lefiInfluence {
public:
protected:
  int numAllocated_;
  int numWidth_;
  int numDistance_;
  int numSpacing_;
  double* width_;
  double* distance_;
  double* spacing_;
};

// 5.7
class lefiTwoWidths {
public:
protected:
  int numWidth_;
  int numWidthAllocated_;
  double* width_;
  double* prl_;
  int*    hasPRL_;
  int*    numWidthSpacing_;   // each slot contains number of spacing of slot
  double* widthSpacing_;
  int*    atNsp_;             // accumulate total number of spacing
};

// 5.5
class lefiSpacingTable {
public:
protected:
  int hasInfluence_;
  lefiInfluence*   influence_;
  lefiParallel*    parallel_;
  lefiTwoWidths*   twoWidths_;               // 5.7
};

// 5.7
class lefiOrthogonal {
public:
protected:
  int numAllocated_;
  int numCutOrtho_;
  double* cutWithin_;
  double* ortho_;
};

// 5.5
class lefiAntennaModel {
public:
protected:
  int hasAntennaAreaRatio_;
  int hasAntennaDiffAreaRatio_;
  int hasAntennaDiffAreaRatioPWL_;
  int hasAntennaCumAreaRatio_;
  int hasAntennaCumDiffAreaRatio_;
  int hasAntennaCumDiffAreaRatioPWL_;
  int hasAntennaAreaFactor_;
  int hasAntennaAreaFactorDUO_;
  int hasAntennaSideAreaRatio_;
  int hasAntennaDiffSideAreaRatio_;
  int hasAntennaDiffSideAreaRatioPWL_;
  int hasAntennaCumSideAreaRatio_;
  int hasAntennaCumDiffSideAreaRatio_;
  int hasAntennaCumDiffSideAreaRatioPWL_;
  int hasAntennaSideAreaFactor_;
  int hasAntennaSideAreaFactorDUO_;
  int hasAntennaCumRoutingPlusCut_;        // 5.7
  int hasAntennaGatePlusDiff_;             // 5.7
  int hasAntennaAreaMinusDiff_;            // 5.7

  char*  oxide_;
  double antennaAreaRatio_;
  double antennaDiffAreaRatio_;
  lefiAntennaPWL* antennaDiffAreaRatioPWL_;
  double antennaCumAreaRatio_;
  double antennaCumDiffAreaRatio_;
  lefiAntennaPWL* antennaCumDiffAreaRatioPWL_;
  double antennaAreaFactor_;
  double antennaSideAreaRatio_;
  double antennaDiffSideAreaRatio_;
  lefiAntennaPWL* antennaDiffSideAreaRatioPWL_;
  double antennaCumSideAreaRatio_;
  double antennaCumDiffSideAreaRatio_;
  lefiAntennaPWL* antennaCumDiffSideAreaRatioPWL_;
  double antennaSideAreaFactor_;
  double antennaGatePlusDiff_;                  // 5.7
  double antennaAreaMinusDiff_;                 // 5.7
  lefiAntennaPWL* antennaAreaDiffReducePWL_;    // 5.7
};

class lefiLayer {
public:
private:
  void parseSpacing(int index);
  void parseMaxFloating(int index);
  void parseArraySpacing(int index);
  void parseMinstep(int index);
  void parseAntennaCumRouting(int index);
  void parseAntennaGatePlus(int index);
  void parseAntennaAreaMinus(int index);
  void parseAntennaAreaDiff(int index);

  void parseLayerType(int index);         // 5.8
  void parseLayerEnclosure(int index);    // 5.8
  void parseLayerWidthTable(int indxe);   // 5.8

protected:
  char* name_;
  int nameSize_;
  char* type_;
  int typeSize_;
  char* layerType_;   // 5.8 - POLYROUTING, MIMCAP, TSV, PASSIVATION, NWELL

  int hasPitch_;
  int hasMask_;                       // 5.8 native
  int hasOffset_;
  int hasWidth_;            
  int hasArea_;
  int hasSpacing_;
  int hasDiagPitch_;                  // 5.6
  int hasDiagWidth_;                  // 5.6
  int hasDiagSpacing_;                // 5.6
  int* hasSpacingName_;               // 5.5
  int* hasSpacingLayerStack_;         // 5.7
  int* hasSpacingAdjacent_;           // 5.5
  int* hasSpacingRange_;              // pcr 409334
  int* hasSpacingUseLengthThreshold_; // pcr 282799, due to mult. spacing allow
  int* hasSpacingLengthThreshold_;    // pcr 409334
  int* hasSpacingCenterToCenter_;     // 5.6
  int* hasSpacingParallelOverlap_;    // 5.7
  int* hasSpacingCutArea_;            // 5.7
  int* hasSpacingEndOfLine_;          // 5.7
  int* hasSpacingParellelEdge_;       // 5.7
  int* hasSpacingTwoEdges_;           // 5.7
  int* hasSpacingAdjacentExcept_;     // 5.7
  int* hasSpacingSamenet_;            // 5.7
  int* hasSpacingSamenetPGonly_;      // 5.7
  int hasArraySpacing_;               // 5.7
  int hasDirection_;
  int hasResistance_;
  int hasCapacitance_;
  int hasHeight_;
  int hasWireExtension_;
  int hasThickness_;
  int hasShrinkage_;
  int hasCapMultiplier_;
  int hasEdgeCap_;
  int hasAntennaArea_;
  int hasAntennaLength_;
  int hasCurrentDensityPoint_;

  double currentDensity_;
  double pitchX_;                     // 5.6
  double pitchY_;                     // 5.6
  double offsetX_;                    // 5.6
  double offsetY_;                    // 5.6
  double diagPitchX_;                 // 5.6
  double diagPitchY_;                 // 5.6
  double diagWidth_;                  // 5.6
  double diagSpacing_;                // 5.6
  double width_;
  double area_;
  double wireExtension_;
  int numSpacings_;
  int spacingsAllocated_;
  int maskNumber_;                     // 5.8
  double* spacing_;          // for Cut & routing Layer, spacing is multiple
  char**  spacingName_;
  int*    spacingAdjacentCuts_;    // 5.5
  double* spacingAdjacentWithin_;  // 5.5
  double* spacingCutArea_;         // 5.7
  double* rangeMin_;         // pcr 282799 & 408930, due to mult spacing allow
  double* rangeMax_;         // pcr 282799 & 408930, due to mult spacing allow
  double* rangeInfluence_;   // pcr 282799 & 408930, due to mult spacing allow
  double* rangeInfluenceRangeMin_;          // pcr 388183 & 408930
  double* rangeInfluenceRangeMax_;          // pcr 388183 & 408930
  double* rangeRangeMin_;                   // pcr 408930
  double* rangeRangeMax_;                   // pcr 408930
  double* lengthThreshold_;                 // pcr 408930
  double* lengthThresholdRangeMin_;         // pcr 408930
  double* lengthThresholdRangeMax_;         // pcr 408930

  // 5.5
  int     numMinimumcut_;
  int     minimumcutAllocated_;
  int*    minimumcut_;                       // pcr 409334
  double* minimumcutWidth_;                  // pcr 409334
  int*    hasMinimumcutWithin_;              // 5.7
  double* minimumcutWithin_;                 // 5.7
  int*    hasMinimumcutConnection_;
  char**  minimumcutConnection_;
  int*    hasMinimumcutNumCuts_;
  double* minimumcutLength_;
  double* minimumcutDistance_;

  double  maxwidth_;                          // 5.5
  double  minwidth_;                          // 5.5
  int     numMinenclosedarea_;                // 5.5
  int     minenclosedareaAllocated_;          // 5.5
  double* minenclosedarea_;                   // 5.5
  double* minenclosedareaWidth_;              // 5.5
  double  protrusionWidth1_;                  // 5.5
  double  protrusionLength_;                  // 5.5
  double  protrusionWidth2_;                  // 5.5

  int     numMinstep_;                        // 5.6
  int     numMinstepAlloc_;                   // 5.6
  double* minstep_;                           // 5.6, switch to multiple
  char**  minstepType_;                       // INSIDECORNER|OUTSIDECORNER|STEP
  double* minstepLengthsum_; 
  int*    minstepMaxEdges_;                   // 5.7
  double* minstepMinAdjLength_;               // 5.7
  double* minstepMinBetLength_;               // 5.7
  int*    minstepXSameCorners_;               // 5.7

  char*  direction_;
  double resistance_;
  double capacitance_;
  double height_;
  double thickness_;
  double shrinkage_;
  double capMultiplier_;
  double edgeCap_;
  double antennaArea_;
  double antennaLength_;

  int numCurrentPoints_;
  int currentPointsAllocated_;
  double* currentWidths_;
  double* current_;

  int numCapacitancePoints_;
  int capacitancePointsAllocated_;
  double* capacitanceWidths_;
  double* capacitances_;

  int numResistancePoints_;
  int resistancePointsAllocated_;
  double* resistanceWidths_;
  double* resistances_;

  int numProps_;
  int propsAllocated_;
  char**  names_;
  char**  values_;
  double* dvalues_;
  char*   types_;                     // I: integer, R: real, S:string
                                      // Q: quotedstring
  int numAccurrents_;                 // number of ACCURRENTDENSITY
  int accurrentAllocated_;
  lefiLayerDensity** accurrents_;
  int numDccurrents_;                 // number of DCCURRENTDENSITY
  int dccurrentAllocated_;
  lefiLayerDensity** dccurrents_;
  int numNums_;
  int numAllocated_;
  double* nums_;

  // 3/23/2000 - Wanda da Rosa.  The following is for 5.4 ANTENNA.
  //             Either 5.4 or 5.3 are allowed, not both
  int hasAntennaAreaRatio_;
  int hasAntennaDiffAreaRatio_;
  int hasAntennaDiffAreaRatioPWL_;
  int hasAntennaCumAreaRatio_;
  int hasAntennaCumDiffAreaRatio_;
  int hasAntennaCumDiffAreaRatioPWL_;
  int hasAntennaAreaFactor_;
  int hasAntennaAreaFactorDUO_;
  int hasAntennaSideAreaRatio_;
  int hasAntennaDiffSideAreaRatio_;
  int hasAntennaDiffSideAreaRatioPWL_;
  int hasAntennaCumSideAreaRatio_;
  int hasAntennaCumDiffSideAreaRatio_;
  int hasAntennaCumDiffSideAreaRatioPWL_;
  int hasAntennaSideAreaFactor_;
  int hasAntennaSideAreaFactorDUO_;

  // 5.5 AntennaModel
  lefiAntennaModel* currentAntennaModel_;
  int numAntennaModel_;
  int antennaModelAllocated_;
  lefiAntennaModel** antennaModel_;

  // 8/29/2001 - Wanda da Rosa.  The following is for 5.4 enhancements.
  int hasSlotWireWidth_;
  int hasSlotWireLength_;
  int hasSlotWidth_;
  int hasSlotLength_;
  int hasMaxAdjacentSlotSpacing_;
  int hasMaxCoaxialSlotSpacing_;
  int hasMaxEdgeSlotSpacing_;
  int hasSplitWireWidth_;
  int hasMinimumDensity_;
  int hasMaximumDensity_;
  int hasDensityCheckWindow_;
  int hasDensityCheckStep_;
  int hasFillActiveSpacing_;
  int hasTwoWidthPRL_;

  double slotWireWidth_; 
  double slotWireLength_; 
  double slotWidth_; 
  double slotLength_; 
  double maxAdjacentSlotSpacing_; 
  double maxCoaxialSlotSpacing_; 
  double maxEdgeSlotSpacing_; 
  double splitWireWidth_; 
  double minimumDensity_; 
  double maximumDensity_; 
  double densityCheckWindowLength_; 
  double densityCheckWindowWidth_; 
  double densityCheckStep_; 
  double fillActiveSpacing_; 

  // 5.5 SPACINGTABLE
  int numSpacingTable_;
  int spacingTableAllocated_;
  lefiSpacingTable** spacingTable_;

  // 5.6
  int numEnclosure_;
  int enclosureAllocated_;
  char** enclosureRules_;
  double* overhang1_;
  double* overhang2_;
  double* encminWidth_;
  double* cutWithin_;
  double* minLength_;
  int numPreferEnclosure_;
  int preferEnclosureAllocated_;
  char** preferEnclosureRules_;
  double* preferOverhang1_;
  double* preferOverhang2_;
  double* preferMinWidth_;
  double  resPerCut_;
  double  diagMinEdgeLength_;
  int numMinSize_;
  double* minSizeWidth_;
  double* minSizeLength_;

  // 5.7
  double* eolWidth_;
  double* eolWithin_;
  double* parSpace_;
  double* parWithin_;
  double  maxArea_;
  int     hasLongArray_;
  double  viaWidth_;
  double  cutSpacing_;
  int     numArrayCuts_;
  int     arrayCutsAllocated_;
  int*    arrayCuts_;
  double* arraySpacings_;
  int     hasSpacingTableOrtho_;
  lefiOrthogonal* spacingTableOrtho_;
  double* notchLength_;
  double* endOfNotchWidth_;
  double* minNotchSpacing_;
  double* eonotchLength_;

  int     lef58WidthTableOrthoValues_;
  int     lef58WidthTableWrongDirValues_;
  double* lef58WidthTableOrtho_;
  double* lef58WidthTableWrongDir_;
};

//lefiMacro.hpp
class lefiObstruction {
public:

protected:

  lefiGeometries* geometries_;
};

// 5.5
class lefiPinAntennaModel {
public:

protected:
  char* oxide_;
  int   hasReturn_;

  int numAntennaGateArea_;
  int antennaGateAreaAllocated_;
  double* antennaGateArea_;
  char** antennaGateAreaLayer_;

  int numAntennaMaxAreaCar_;
  int antennaMaxAreaCarAllocated_;
  double* antennaMaxAreaCar_;
  char** antennaMaxAreaCarLayer_;

  int numAntennaMaxSideAreaCar_;
  int antennaMaxSideAreaCarAllocated_;
  double* antennaMaxSideAreaCar_;
  char** antennaMaxSideAreaCarLayer_;

  int numAntennaMaxCutCar_;
  int antennaMaxCutCarAllocated_;
  double* antennaMaxCutCar_;
  char** antennaMaxCutCarLayer_;
};

class lefiPin {
public:

protected:
  int   nameSize_;
  char* name_;

  char hasLEQ_;
  char hasDirection_;
  char hasUse_;
  char hasShape_;
  char hasMustjoin_;
  char hasOutMargin_;
  char hasOutResistance_;
  char hasInMargin_;
  char hasPower_;
  char hasLeakage_;
  char hasMaxload_;
  char hasMaxdelay_;
  char hasCapacitance_;
  char hasResistance_;
  char hasPulldownres_;
  char hasTieoffr_;
  char hasVHI_; 
  char hasVLO_;
  char hasRiseVoltage_;
  char hasFallVoltage_;
  char hasRiseThresh_;
  char hasFallThresh_;
  char hasRiseSatcur_;
  char hasFallSatcur_;
  char hasCurrentSource_;
  char hasTables_;
  char hasAntennasize_;
  char hasRiseSlewLimit_;
  char hasFallSlewLimit_;

  int     numForeigns_;
  int     foreignAllocated_;
  int*    hasForeignOrient_;
  int*    hasForeignPoint_;
  int*    foreignOrient_;
  double* foreignX_;
  double* foreignY_;
  char**  foreign_;

  int    LEQSize_;
  char*  LEQ_;
  int    mustjoinSize_;
  char*  mustjoin_;
  double outMarginH_;
  double outMarginL_;
  double outResistanceH_;
  double outResistanceL_;
  double inMarginH_;
  double inMarginL_;
  double power_;
  double leakage_;
  double maxload_;
  double maxdelay_;
  double capacitance_;
  double resistance_;
  double pulldownres_;
  double tieoffr_;
  double VHI_;
  double VLO_;
  double riseVoltage_;
  double fallVoltage_;
  double riseThresh_;
  double fallThresh_;
  double riseSatcur_;
  double fallSatcur_;
  int lowTableSize_;
  char* lowTable_;
  int highTableSize_;
  char* highTable_;
  double riseSlewLimit_;
  double fallSlewLimit_;

  // 5.5 AntennaModel
  int numAntennaModel_;
  int antennaModelAllocated_;
  int curAntennaModelIndex_;     // save the current index of the antenna
  lefiPinAntennaModel** pinAntennaModel_;

  int numAntennaSize_;
  int antennaSizeAllocated_;
  double* antennaSize_;
  char** antennaSizeLayer_;

  int numAntennaMetalArea_;
  int antennaMetalAreaAllocated_;
  double* antennaMetalArea_;
  char** antennaMetalAreaLayer_;

  int numAntennaMetalLength_;
  int antennaMetalLengthAllocated_;
  double* antennaMetalLength_;
  char** antennaMetalLengthLayer_;

  int numAntennaPartialMetalArea_;
  int antennaPartialMetalAreaAllocated_;
  double* antennaPartialMetalArea_;
  char** antennaPartialMetalAreaLayer_;

  int numAntennaPartialMetalSideArea_;
  int antennaPartialMetalSideAreaAllocated_;
  double* antennaPartialMetalSideArea_;
  char** antennaPartialMetalSideAreaLayer_;

  int numAntennaPartialCutArea_;
  int antennaPartialCutAreaAllocated_;
  double* antennaPartialCutArea_;
  char** antennaPartialCutAreaLayer_;

  int numAntennaDiffArea_;
  int antennaDiffAreaAllocated_;
  double* antennaDiffArea_;
  char** antennaDiffAreaLayer_;

  char* taperRule_;

  char* netEpxr_;
  char* ssPinName_;
  char* gsPinName_;

  char direction_[32];
  char use_[12];
  char shape_[12];
  char currentSource_[12];

  int numProperties_;
  int propertiesAllocated_;
  char** propNames_;
  char** propValues_;
  double* propNums_;
  char*  propTypes_;

  int numPorts_;
  int portsAllocated_;
  lefiGeometries** ports_;
};

// 5.6
class lefiDensity {
public:

protected:
  int    numLayers_;
  int    layersAllocated_;
  char** layerName_;
  int*   numRects_;
  int*   rectsAllocated_;
  struct lefiGeomRect** rects_;
  double**       densityValue_;
};

class lefiMacro {
public:

protected:
  int nameSize_;
  char* name_;
  char macroClass_[32];
  char source_[12];

  int generatorSize_;
  char* generator_;

  char hasClass_;
  char hasGenerator_;
  char hasGenerate_;
  char hasPower_;
  char hasOrigin_;
  char hasSource_;
  char hasEEQ_;
  char hasLEQ_;
  char hasSymmetry_;  // X=1  Y=2  R90=4  (can be combined)
  char hasSiteName_;
  char hasSize_;
  char hasClockType_;
  char isBuffer_;
  char isInverter_;

  char* EEQ_;
  int EEQSize_;
  char* LEQ_;
  int LEQSize_;
  char* gen1_;
  int gen1Size_;
  char* gen2_;
  int gen2Size_;
  double power_;
  double originX_;
  double originY_;
  double sizeX_;
  double sizeY_;

  int numSites_;
  int sitesAllocated_;
  lefiSitePattern** pattern_;

  int numForeigns_;
  int foreignAllocated_;
  int*  hasForeignOrigin_;
  int*  hasForeignPoint_;
  int*  foreignOrient_;
  double* foreignX_;
  double* foreignY_;
  char** foreign_;

  int siteNameSize_;
  char* siteName_;

  char* clockType_;
  int clockTypeSize_;

  int numProperties_;
  int propertiesAllocated_;
  char** propNames_;
  char** propValues_;
  double* propNums_;
  char*  propTypes_;

  int isFixedMask_;
};

class lefiTiming {
public:
protected:
  int numFrom_;
  char** from_;
  int fromAllocated_;
  int numTo_;
  char** to_;
  int toAllocated_;

  int hasTransition_;
  int hasDelay_;
  int hasRiseSlew_;
  int hasRiseSlew2_;
  int hasFallSlew_;
  int hasFallSlew2_;
  int hasRiseIntrinsic_;
  int hasFallIntrinsic_;
  int hasRiseRS_;
  int hasRiseCS_;
  int hasFallRS_;
  int hasFallCS_;
  int hasUnateness_;
  int hasFallAtt1_;
  int hasRiseAtt1_;
  int hasFallTo_;
  int hasRiseTo_;
  int hasStableTiming_;
  int hasSDFonePinTrigger_;
  int hasSDFtwoPinTrigger_;
  int hasSDFcondStart_;
  int hasSDFcondEnd_;
  int hasSDFcond_;
  int nowRise_;

  int numOfAxisNumbers_;
  double* axisNumbers_;
  int axisNumbersAllocated_;

  int numOfTableEntries_;
  int tableEntriesAllocated_;
  double* table_;  // three numbers per entry 

  char* delayRiseOrFall_;
  char* delayUnateness_;
  double delayTableOne_;
  double delayTableTwo_;
  double delayTableThree_;
  char* transitionRiseOrFall_;
  char* transitionUnateness_;
  double transitionTableOne_;
  double transitionTableTwo_;
  double transitionTableThree_;
  double riseIntrinsicOne_;
  double riseIntrinsicTwo_;
  double riseIntrinsicThree_;
  double riseIntrinsicFour_;
  double fallIntrinsicOne_;
  double fallIntrinsicTwo_;
  double fallIntrinsicThree_;
  double fallIntrinsicFour_;
  double riseSlewOne_;
  double riseSlewTwo_;
  double riseSlewThree_;
  double riseSlewFour_;
  double riseSlewFive_;
  double riseSlewSix_;
  double riseSlewSeven_;
  double fallSlewOne_;
  double fallSlewTwo_;
  double fallSlewThree_;
  double fallSlewFour_;
  double fallSlewFive_;
  double fallSlewSix_;
  double fallSlewSeven_;
  double riseRSOne_;
  double riseRSTwo_;
  double riseCSOne_;
  double riseCSTwo_;
  double fallRSOne_;
  double fallRSTwo_;
  double fallCSOne_;
  double fallCSTwo_;
  char* unateness_;
  double riseAtt1One_;
  double riseAtt1Two_;
  double fallAtt1One_;
  double fallAtt1Two_;
  double fallToOne_;
  double fallToTwo_;
  double riseToOne_;
  double riseToTwo_;
  double stableSetup_;
  double stableHold_;
  char* stableRiseFall_;
  char* SDFtriggerType_;
  char* SDFfromTrigger_;
  char* SDFtoTrigger_;
  double SDFtriggerTableOne_;
  double SDFtriggerTableTwo_;
  double SDFtriggerTableThree_;
  char* SDFcondStart_;
  char* SDFcondEnd_;
  char* SDFcond_;
};

// 5.8 
class lefiMacroSite {
public:
protected:
  const char            *siteName_;
  const lefiSitePattern *sitePattern_;
};

class lefiMacroForeign {
public:
protected:
  const char *cellName_;
  int        cellHasPts_;
  double     px_;
  double     py_;
  int        cellHasOrient_;
  int        cellOrient_;
};

//lefiMisc.hpp

struct lefiGeomRect {
      double xl;
      double yl;
      double xh;
      double yh;
      int    colorMask;
};

struct lefiGeomRectIter {
  double xl;
  double yl;
  double xh;
  double yh;
  double xStart;
  double yStart;
  double xStep;
  double yStep;
  int    colorMask;
};

struct lefiGeomPath {
  int     numPoints;
  double* x;
  double* y;
  int     colorMask;
};

struct lefiGeomPathIter {
  int     numPoints;
  double* x;
  double* y;
  double xStart;
  double yStart;
  double xStep;
  double yStep;
  int    colorMask;
};

struct lefiGeomPolygon {
  int     numPoints;
  double* x;
  double* y;
  int     colorMask;
};

struct lefiGeomPolygonIter {
  int numPoints;
  double* x;
  double* y;
  double xStart;
  double yStart;
  double xStep;
  double yStep;
  int    colorMask;
};

enum lefiGeomEnum {
  lefiGeomUnknown = 0,
  lefiGeomLayerE,
  lefiGeomLayerExceptPgNetE,
  lefiGeomLayerMinSpacingE,
  lefiGeomLayerRuleWidthE,
  lefiGeomWidthE,
  lefiGeomPathE,
  lefiGeomPathIterE,
  lefiGeomRectE,
  lefiGeomRectIterE,
  lefiGeomPolygonE,
  lefiGeomPolygonIterE,
  lefiGeomViaE,
  lefiGeomViaIterE,
  lefiGeomClassE,
  lefiGeomEnd
};

struct lefiGeomVia {
  char*  name;
  double x;
  double y;
  int    topMaskNum;
  int    cutMaskNum;
  int    bottomMaskNum;
};

struct lefiGeomViaIter {
  char*  name;
  double x;
  double y;
  double xStart;
  double yStart;
  double xStep;
  double yStep;
  int    topMaskNum;
  int    cutMaskNum;
  int    bottomMaskNum;
};

class lefiGeometries {
public:

protected:

  int numItems_;
  int itemsAllocated_;
  lefiGeomEnum* itemType_;
  void** items_;

  int numPoints_;
  int pointsAllocated_;
  double* x_;
  double* y_;

  double xStart_;
  double yStart_;
  double xStep_;
  double yStep_;
};

class lefiSpacing {
public:

protected:
  int    name1Size_;
  int    name2Size_;
  char*  name1_;
  char*  name2_;
  double distance_;
  int    hasStack_;
};

class lefiIRDrop {
public:

protected:
  int     nameSize_;
  int     value1Size_;
  int     value2Size_;
  int     numValues_;
  int     valuesAllocated_;
  char*   name_;
  double* value1_;
  double* value2_;
};

class lefiMinFeature {
public:

protected:
  double one_;
  double two_;
};

class lefiSite {
public:
protected:
  int    nameSize_;
  char*  name_;
  int    hasClass_;
  char   siteClass_[8];
  double sizeX_;
  double sizeY_;
  int    hasSize_;
  int    symmetry_;   // bit 0-x   bit 1-y   bit 2-90

  int    numRowPattern_;         // 5.6 ROWPATTERN
  int    rowPatternAllocated_;
  char** siteNames_;
  int*   siteOrients_;
};

class lefiSitePattern {
public:
protected:
  int    nameSize_;
  char*  name_;
  int    orient_;
  double x_;
  double y_;
  double xStart_;
  double yStart_;
  double xStep_;
  double yStep_;
};

class lefiTrackPattern {
public:

protected:
  int    nameSize_;
  char*  name_;
  double start_;
  int    numTracks_;
  double space_;

  int    numLayers_;
  int    layerAllocated_;
  char** layerNames_;
};

class lefiGcellPattern {
public:

protected:
  int    nameSize_;
  char*  name_;
  double start_;
  int    numCRs_;
  double space_;
};

class lefiUseMinSpacing {
public:

protected:
  char* name_;
  int   value_;
};

// 5.5 for Maximum Stacked-via rule
class lefiMaxStackVia {
public:
protected:
  int   value_;
  int   hasRange_;
  char* bottomLayer_;
  char* topLayer_;
};


//lefiNonDefault.hpp
class lefiNonDefault {
public:
protected:
  int nameSize_;
  char* name_;

  // Layer information
  int numLayers_;
  int layersAllocated_;
  char** layerName_;
  double* width_;
  double* spacing_;
  double* wireExtension_;
  char* hasWidth_;
  char* hasSpacing_;
  char* hasWireExtension_;

  // 5.4
  double* resistance_;
  double* capacitance_;
  double* edgeCap_;
  char*   hasResistance_;
  char*   hasCapacitance_;
  char*   hasEdgeCap_;

  double* diagWidth_;              // 5.6
  char*   hasDiagWidth_;           // 5.6

  int numVias_;
  int allocatedVias_;
  lefiVia** viaRules_;

  int numSpacing_;
  int allocatedSpacing_;
  lefiSpacing** spacingRules_;

  int    hardSpacing_;             // 5.6
  int    numUseVias_;              // 5.6
  int    allocatedUseVias_;        // 5.6
  char** useViaName_;              // 5.6
  int    numUseViaRules_;          // 5.6
  int    allocatedUseViaRules_;    // 5.6
  char** useViaRuleName_;          // 5.6
  int    numMinCuts_;              // 5.6
  int    allocatedMinCuts_;        // 5.6
  char** cutLayerName_;            // 5.6
  int*   numCuts_;                 // 5.6

  int numProps_;
  int propsAllocated_;
  char**  names_;
  char**  values_;
  double* dvalues_;
  char*   types_;
};

//lefiProp.hpp
// Struct holds the data for one property.
class lefiProp {
public:
protected:
  char* propType_;      // "design" "net" "macro" ...
  char* propName_;      // name.
  int nameSize_;        // allocated size of name.
  char hasRange_;       // either 0:NO or 1:YES.
  char hasNumber_;      // either 0:NO or 1:YES.
  char hasNameMapString_;
  char dataType_;       // either I:integer R:real S:string Q:quotedstring.
            // N:property name is not defined.
  char* stringData_;    // if it is a string the data is here.
  int stringLength_;    // allocated size of stringData.
  double left_;
  double right_; // if it has a range the numbers are here.
  double d_;            // if it is a real or int the number is here.
};

//lefiPropType.hpp
class lefiPropType {
public:
protected:
  int    numProperties_;
  int    propertiesAllocated_;
  char** propNames_;      // name.
  char*  propTypes_;      // 'R' == "REAL", 'I' == "INTEGER"
};

//lefiUnits.hpp
class lefiUnits {
  public:
  protected:
    int hasDatabase_;
    int hasCapacitance_;
    int hasResistance_;
    int hasTime_;
    int hasPower_;
    int hasCurrent_;
    int hasVoltage_;
    int hasFrequency_;
    char* databaseName_;
    double databaseNumber_;
    double capacitance_;
    double resistance_;
    double power_;
    double time_;
    double current_;
    double voltage_;
    double frequency_;
  };

//lefiUtil.hpp
struct lefiPoints {
  double x;
  double y;
};

typedef struct lefiPoints lefiNum;

//lefiViaRule.hpp
class lefiViaRuleLayer {
  public:
  protected:
    char* name_;
    char direction_;
    double overhang1_;                                       // 5.5
    double overhang2_;                                       // 5.5
    int hasWidth_;
    int hasResistance_;
    int hasOverhang_;
    int hasMetalOverhang_;
    int hasSpacing_;
    int hasRect_;
    double widthMin_;
    double widthMax_;
    double overhang_;
    double metalOverhang_;
    double resistance_;
    double spacingStepX_;
    double spacingStepY_;
    double xl_, yl_, xh_, yh_;
  };
  
  class lefiViaRule {
  public:
  protected:
    char* name_;
    int nameSize_;
   
    int hasGenerate_;
    int hasDefault_;
  
    int numLayers_;
    lefiViaRuleLayer layers_[3];
  
    int numVias_;
    int viasAllocated_;
    char** vias_;
  
    int numProps_;
    int propsAllocated_;
    char**  names_;
    char**  values_;
    double* dvalues_;
    char*   types_;
  };