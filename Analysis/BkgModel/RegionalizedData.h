/* Copyright (C) 2012 Ion Torrent Systems, Inc. All Rights Reserved */
#ifndef REGIONALIZEDDATA_H
#define REGIONALIZEDDATA_H


#include <stdio.h>
#include <vector>
#include <map>

#include "Image.h"
#include "Region.h"
#include "FlowBuffer.h"

#include "Utils.h"
#include "BeadParams.h"
#include "BeadTracker.h"
#include "RegionParams.h"
#include "RegionTracker.h"
#include "EmphasisVector.h"

#include "TimeCompression.h"

#include "BkgTrace.h"
#include "EmptyTraceTracker.h"
#include "BeadScratch.h"
#include "Serialization.h"
#include "SampleClonality.h"
#include <stddef.h>

class GlobalDefaultsForBkgModel;

class RegionalizedData
{
public:
  int fitters_applied;
  // the subregion that contains regionalized data
  Region *region;
  bool isBestRegion;
  bool isRegionCenter(int ibd);
  bool isLinePoint(int ibd, int nSamples);
  bool isGridPoint(int ibd, int nSamples);
  //int outputCount;

    TimeCompression time_c; // region specific
// local region emphasis vectors - may vary across chip by trace
    EmphasisClass emphasis_data; // region specific, effectively a parameter
    EmphasisClass std_time_comp_emphasis; // generate emphasis for standard time compression
    
    BkgTrace my_trace;  // initialized and populated by this object
    EmptyTraceTracker *emptyTraceTracker;
    EmptyTrace *emptytrace;

  // The things we're applying optimizers to fit:
  BeadTracker my_beads;
  RegionTracker my_regions;

  // initial guesses for nuc rise parameters
  float   sigma_start;
  float   t_mid_nuc_start;
  int   t0_frame;

  // space for processing current bead in optimization (multiflow levmar)
  // recycled for use in other optimizers
  // possibly should have more than one instance
  BeadScratchSpace my_scratch; // @TODO: transfer to >fitters<

  void regParamsToJson(Json::Value &regparams_json);
  void LoadRestartRegParamsFromJson(const Json::Value &regparams_json);
  void DumpRegionParameters (float cur_avg_resid);
  void DumpTimeAndEmphasisByRegion (FILE *my_fp);
  void NoData();

  const Region *get_region()
  {
    return region;
  }
  int get_region_col()
  {
    return region->col;
  }
  int get_region_row()
  {
    return region->row;
  }
  int GetNumLiveBeads() const
  {
    return my_beads.numLBeads;
  }

  int GetNumFrames()
  {
    return time_c.npts();
  }


  int GetNumHighPPF() const
  {
    return my_beads.NumHighPPF();
  }

  int GetNumPolyclonal() const
  {
    return my_beads.NumPolyclonal();
  }

  int GetNumBadKey() const
  {
    return my_beads.NumBadKey();
  }
  BeadParams& GetParams (int iWell)
  {
    return my_beads.params_nn[iWell];
  }
  void GetParams (int iWell,struct BeadParams *pOut)
  {
    if ( (iWell >= 0) && (iWell < my_beads.numLBeads) && (pOut != NULL))
    {
      memcpy (pOut,&my_beads.params_nn[iWell],sizeof (struct BeadParams));
    }
  }
  void DumpEmptyTrace (FILE *my_fp, int flow_block_size);
  void DumpInitValues (FILE *my_fp)
  {
    if (region!=NULL)
      fprintf (my_fp, "%d\t%d\t%f\t%f\n",region->col,region->row, t_mid_nuc_start, sigma_start);
  }

  RegionalizedData();
  RegionalizedData( const CommandLineOpts * inception_state );
  ~RegionalizedData();

  void AllocTraceBuffers(int flow_block_size);
  void AllocFitBuffers(int flow_block_size);
  void SetTimeAndEmphasis (GlobalDefaultsForBkgModel &global_defaults, float tmid, float t0_offset);
  void SetupTimeAndBuffers (GlobalDefaultsForBkgModel &global_defaults,float sigma_guess,
                            float t_mid_nuc_guess,
                            float t0_offset, int flow_block_size,
                            int global_flow_max );
  // set states of emphasis for the fit
  void LimitedEmphasis();
  void AdaptiveEmphasis();
  void SetCrudeEmphasisVectors();
  void SetFineEmphasisVectors();
  void GenerateFineEmphasisForStdTimeCompression();


  // don't know if this is correct, again may want coprocessor instances
  // loading the data into the regionalized structure
  void AddOneFlowToBuffer (GlobalDefaultsForBkgModel &global_defaults, 
                    FlowBufferInfo & my_flow, int flow);
  void UpdateTracesFromImage (Image *img, FlowBufferInfo &my_flow, int flow, int flow_block_size);

  bool LoadOneFlow (Image *img, GlobalDefaultsForBkgModel &global_defaults, 
                    FlowBufferInfo & my_flow, int flow, int flow_block_size);
  bool PrepareLoadOneFlowGPU (Image *img, GlobalDefaultsForBkgModel &global_defaults, 
                    FlowBufferInfo & my_flow, int flow);
  bool FinalizeLoadOneFlowGPU ( FlowBufferInfo & my_flow, int flow_block_size );

  // technically trivial fitters that fit the dc_offset model
  // so we may think about doing this on a coprocessor
  void RezeroByCurrentTiming(int flow_block_size);
  void RezeroTraces (float t_start, float t_mid_nuc, float t_offset_beads, float t_offset_empty, int flow_buffer_index, int flow_block_size );
  void RezeroTracesAllFlows (float t_start, float t_mid_nuc, float t_offset_beads, float t_offset_empty, int flow_block_size);

  void PickRepresentativeHighQualityWells (float stringent_filter, int min_beads,  int max_rank, bool revert_regional_sampling, int flow_block_size);

  void ZeromerAndOnemerAllBeads(Clonality& sc, size_t const t0_ix, size_t const t_end_ix, std::vector<float>& key_zeromer, std::vector<float>& key_onemer, std::vector<int>& keyLen, int flow_block_size);
  void ZeromerAndOnemerOneBead(std::vector<int> const& key, int const keyLen, std::vector<float> const& signal, float& key_zeromer, float& key_onemer);
  void CalculateFirstBlockClonalPenalty(float nuc_flow_frame_width, std::vector<float>& penalty, const int penalty_type, int flow_block_size);

private:
  // Serialization section
  friend class boost::serialization::access;
  template<typename Archive>
  void save(Archive& ar, const unsigned version) const
  {
    //fprintf(stdout, "Serialization: save RegionalizedData...");
    ar &
        region &
        time_c &
        emphasis_data &
        std_time_comp_emphasis &
        my_trace &
        emptyTraceTracker &
        emptytrace &
        my_beads &
        my_regions &
        sigma_start &
        t_mid_nuc_start &
        t0_frame &
        my_scratch &
        region_nSamples;

     //fprintf(stdout, "done with RegionalizedData\n");
  }
  template<typename Archive>
  void load(Archive& ar, const unsigned version)
  {
    // fprintf(stdout, "Serialization: load RegionalizedData...");
    ar &
        region &
        time_c &
        emphasis_data &
        std_time_comp_emphasis &
        my_trace &
        emptyTraceTracker &
        emptytrace &
        my_beads &
        my_regions &
        sigma_start &
        t_mid_nuc_start &
        t0_frame &
        my_scratch &
        region_nSamples;

    SetCrudeEmphasisVectors();

    // TODO Oh, this is awful, but on restart, I don't have a better place to get this.
    if (my_scratch.npts > 0)
      AllocFitBuffers( my_scratch.bead_flow_t / my_scratch.npts );

    //fprintf(stdout, "done with RegionalizedData\n");
  }

  // Need to set up emphasis object for recompression of raw traces when exponential tail fit is on
  void SetUpEmphasisForStandardCompression(GlobalDefaultsForBkgModel &global_defaults);

  BOOST_SERIALIZATION_SPLIT_MEMBER()

  public:
    int get_region_nSamples(void) { return (region_nSamples); }
    int get_sampleIndex(int ibd);
    int assign_sampleIndex(void); // return (nAssigned)
    bool isRegionSample(int ibd);
  private:
	int assign_sampleIndex_even(void);
	int assign_sampleIndex_even_random(void); // no uniqueness problem
 	int assign_sampleIndex_random(void); // uniqueness problem, not robust yet w/ infinite loop
	int assign_sampleIndex_random_shuffle(void); // no uniqueness problem
	void my_random_shuffle(vector<int>&cards, unsigned int nCards=52);
	int random_in_range(int range) {return (range>1 ? (rand()%range): 0);}
    std::map<int,int> regionSampleIndex;
	bool sampleIndex_assigned;
	// number of sample beads to trace.h5
	int region_nSamples; // assigned in the constructor: region_nSamples = inception_state->bkg_control.pest_control.bkgDebug_nSamples;
	int nAssigned;
	//bool betterUseEvenSamples(float fraction=0.05) {return (false);} // debugging
	bool betterUseEvenSamples(float fraction=0.05) {int nBeads=GetNumLiveBeads(); return (nBeads<100 ? true: (region_nSamples>nBeads*fraction ? true:false));}
};

#endif // REGIONALIZEDDATA_H
