
#define VERSION "version  4.5 (April 07, 2004)"

/*----------------------------------------------------------------------
 * 3dVol2Surf - dump ascii dataset values corresponding to a surface
 *
 * This program is used to display AFNI dataset values that correspond to
 * a surface.  The surface points are mapped to xyz coordinates, according
 * to the SURF_VOL (surface volume) AFNI dataset.  These coordinates are
 * then matched to voxels in other AFNI datasets.  So given any other
 * AFNI dataset, this program can output all of the sub-brick values that
 * correspond to each of the suface locations.  The user also has options
 * to mask regions for output.
 *
 * usage:
 *    3dVol2Surf [options] -spec SPEC_FILE -sv SURF_VOL               \
 *                         -grid_par AFNI_DSET -map_func MAPPING_FUNC
 *
 * options:
 *
 * 	-help
 * 	-version
 *
 * 	-grid_par   AFNI_DSET
 * 	-map_func   MAP_FUNCTION
 * 	-spec       SPEC_FILE
 * 	-sv         SURF_VOL
 *
 * 	-cmask      MASK_COMMAND
 * 	-debug      LEVEL
 * 	-dnode      NODE_NUM
 * 	-f_index    INDEX_TYPE
 * 	-f_steps    NUM_STEPS
 * 	-f_p1_mm    DISTANCE
 * 	-f_pn_mm    DISTANCE
 * 	-f_p1_fr    FRACTION
 * 	-f_pn_fr    FRACTION
 * 	-oob_index  INDEX
 * 	-oob_value  VALUE
 * 	-oom_value  VALUE
 * 	-out_1D     OUTPUT_FILE
 * 	-no_headers
 *
 * examples:
 *
 *    3dVol2Surf -spec     SubjA.spec     -sv       SubjA_anat+orig   \ 
 *               -grid_par SubjA_EPI+orig -map_func mask  >  output.txt
 *
 *    3dVol2Surf -spec       SubjectA.spec                              \
 *               -sv         SubjectA_spgr+orig                         \ 
 *               -grid_par   SubjA_EPI+orig                             \
 *               -cmask      '-a SubjA.func+orig[2] -expr step(a-0.6)'  \
 *               -map_func   midpoint					\
 *               -out_1D     SubjA_surf_out.txt
 *
 *----------------------------------------------------------------------
*/

/* define program history for -hist option */

static char g_history[] = 

    "----------------------------------------------------------------------\n"
    "history:\n"
    "\n"
    "1.0  February 10, 2003  [rickr]\n"
    "  - initial release\n"
    "\n"
    "1.1  February 11, 2003  [rickr]\n"
    "  - handle no arguments as with -help\n"
    "  - minor updates to -help\n"
    "\n"
    "1.2  February 13, 2003  [rickr]\n"
    "  - init SUMAg array pointers, check before calling Free_()\n"
    "\n"
    "1.3  February 14, 2003  [rickr]\n"
    "  - optionally enable more SUMA debugging\n"
    "\n"
    "2.0  June 06, 2003  [rickr]\n"
    "  - re-wrote program according to 3dSurf2Vol (which was written\n"
    "    according to this :) - using map functions and node lists\n"
    "  - added midpoint map function\n"
    "\n"
    "2.1  June 10, 2003  [rickr]\n"
    "  - added ave map function (see dump_ave_map)\n"
    "\n"
    "2.2  June 19, 2003  [rickr]\n"
    "  - added -f_index INDEX_TYPE option (to index across nodes, too)\n"
    "  - set the default of -f_steps to 2\n"
    "  - use SMD prefix for macros\n"
    "\n"
    "2.3  July 21, 2003  [rickr]\n"
    "  - fixed problem with nodes outside grid_par dataset\n"
    "  - added min/max distance info\n"
    "\n"
    "3.0  August 05, 2003  [rickr]\n"
    "  - renamed SUMA_3dSurfMaskDump.[ch] to SUMA_3dVol2Surf.[ch]\n"
    "  - all output functions now go through dump_surf_3dt\n"
    "  - dump_surf_3dt() is a generalized function to get an MRI_IMARR for\n"
    "    one or a pair of nodes, by converting to a segment of points\n"
    "  - added v2s_adjust_endpts() to apply segment endpoint modifications\n"
    "  - added segment_imarr() to get the segment of points and fill the\n"
    "    MRI_IMARR list (along with other info)\n"
    "  - filter functions have been taken to v2s_apply_filter()\n"
    "  - added min, max and seg_vals map functions (filters)\n"
    "  - added options of the form -f_pX_XX to adjust segment endpoints\n"
    "  - added -dnode option for specific node debugging\n"
    "  - changed -output option to -out_1D\n"
    "  - added new debug info\n"
    "  - added checking of surface order (process from inner to outer)\n"
    "\n"
    "3.1  September 17, 2003  [rickr]\n"
    "  - fixed the help instructions for '-cmask'\n"
    "\n"
    "3.2  September 20, 2003  [rickr]\n"
    "  - added max_abs mapping function\n"
    "  - added options '-oob_index' and '-oob_value'\n"
    "  - added CHECK_NULL_STR macro\n"
    "\n"
    "3.3  September 23, 2003  [rickr]\n"
    "  - added help for -no_headers option\n"
    "\n"
    "3.4  October 01, 2003  [rickr]\n"
    "  - added -oom_value option\n"
    "  - added additional help example (for -oob and -oom options)\n"
    "\n"
    "3.5  October 20, 2003  [rickr]\n"
    "  - call the new engine function, SUMA_LoadSpec_eng()\n"
    "    (this will restrict the debug output from SUMA_LoadSpec())\n"
    "\n"
    "3.6  October 21, 2003  [rickr]\n"
    "  - finish upates for -f_keep_surf_order option\n"
    "    (help and sopt)\n"
    "\n"
    "3.7  November 04, 2003  [rickr]\n"
    "  - added ENTRY() stuff\n"
    "\n"
    "3.8  December 15, 2003  [rickr]\n"
    "  - added options '-surf_A' and '-surf_B'\n"
    "  - called SUMA_spec_select_surfs() and SUMA_spec_set_map_refs()\n"
    "    to pick requested surfaces from spec file\n"
    "  - removed option '-kso'\n"
    "  - added '-hist' option\n"
    "\n"
    "3.9  January 08, 2004  [rickr]\n"
    "  - added '-use_norms' option (segments come from norms)\n"
    "  - added '-norm_len' option to alter default normal lengths\n"
    "  - added '-keep_norm_dir' option to prevent direction check\n"
    "  - reversed order from '-hist' option (newer at bottom)\n"
    "  - added example with normals to help, along with option descriptions\n"
    "\n"
    "4.0  January 23, 2004  [rickr]\n"
    "  - SUMA_isINHmappable() is depricated, check with AnatCorrect field\n"
    "  - version -> 4.0 to celebrate normals :)\n"
    "\n"
    "4.1  February 10, 2004  [rickr]\n"
    "  - output a little more debug info for !AnatCorrect case\n"
    "  - small updates to help examples\n"
    "\n"
    "4.2  February 18, 2004  [rickr]\n"
    "  - added functionality for mapping functions that require sorting\n"
    "  - added mapping functions: median and mode\n"
    "\n"
    "4.3  February 19, 2004  [rickr]\n"
    "  - track 1dindex sources for new sorting filters (median, mode)\n"
    "    i.e. idindex is accurate, not just defaulting to first node\n"
    "\n"
    "4.4  March 26, 2004  [ziad]\n"
    "  - DsetList added to SUMA_LoadSpec_eng() call\n"
    "\n"
    "4.5  April 07, 2004  [rickr]\n"
    "  - fixed inconsistency in check_norm_dirs(), default dirs reversed\n"
    "---------------------------------------------------------------------\n";

/*----------------------------------------------------------------------
 * todo:
 *   - option to restrict columns for output?
 *   - option to use binary niml format for output
 *----------------------------------------------------------------------
*/

#include "mrilib.h"
#include "SUMA_suma.h"
#include "SUMA_3dVol2Surf.h"

/* --------------------  globals  -------------------- */

/* for all SUMA programs... */
SUMA_SurfaceViewer * SUMAg_SVv = NULL;	/* array of Surf View structs   */
int                  SUMAg_N_SVv = 0;	/* length of SVv array          */
SUMA_DO            * SUMAg_DOv = NULL;	/* array of Displayable Objects */
int                  SUMAg_N_DOv = 0;	/* length of DOv array          */
SUMA_CommonFields  * SUMAg_CF = NULL;	/* info common to all viewers   */

/* this must match smap_nums enum */
char * g_smap_names[] = { "none", "mask", "midpoint", "mask2", "ave",
                          "count", "min", "max", "max_abs", "seg_vals",
			  "median", "mode" };


/* --------------------  AFNI prototype(s)  -------------------- */
extern void machdep( void );

#define MAIN

/*----------------------------------------------------------------------*/

int main( int argc , char * argv[] )
{
    SUMA_SurfSpecFile  spec;
    node_list_t        node_list = {NULL, 0, 0};
    param_t            params;
    smap_opts_t        sopt;
    opts_t             opts;
    int                ret_val;

    mainENTRY("3dVol2Surf main");
    machdep();
    AFNI_logger("3dVol2Surf",argc,argv);

    /* validate inputs and init options structure */
    if ( ( ret_val = init_options(&opts, argc, argv) ) != 0 )
	return ret_val;

    if ( ( ret_val = validate_options(&opts, &params) ) != 0 )
	return ret_val;

    if ( (ret_val = set_smap_opts( &opts, &params, &sopt )) != 0 )
	return ret_val;

    /* read surface files */
    if ( (ret_val = read_surf_files(&opts, &params, &spec)) != 0 )
	return ret_val;

    /*  get node list from surfaces (multiple points per node)
     *  need merge function
     */
    if ( (ret_val = create_node_list( &sopt, &node_list )) != 0 )
	return ret_val;

    if ( (ret_val = write_output( &sopt, &params, &node_list )) != 0 )
	return ret_val;

    /* free memory */
    final_clean_up(&opts, &params, &spec, &node_list);

    return ret_val;
}


/*----------------------------------------------------------------------
 * write_output
 *----------------------------------------------------------------------
*/
int write_output ( smap_opts_t * sopt, param_t * p, node_list_t * N )
{
ENTRY("write_output");

    if ( sopt == NULL || p == NULL || N == NULL )
    {
	fprintf( stderr, "** smd_wo - bad params (%p,%p,%p)\n",
		 sopt, p, N );
	RETURN(-1);
    }

    switch (sopt->map)
    {
	case E_SMAP_AVE:
	case E_SMAP_MASK:
	case E_SMAP_MAX:
	case E_SMAP_MAX_ABS:
	case E_SMAP_MIDPT:
	case E_SMAP_MIN:
	case E_SMAP_SEG_VALS:
	case E_SMAP_MEDIAN:
	case E_SMAP_MODE:
	{
	    dump_surf_3dt( sopt, p, N );
	    break;
	}

	case E_SMAP_COUNT:
	case E_SMAP_MASK2:
	{
	    fprintf( stderr, "** function '%s' coming soon ...\n",
		     g_smap_names[sopt->map] );
	    break;
	}

	default:
	    fprintf( stderr, "** smd_n2v: unknown map %d\n", sopt->map );
	    break;
    }

    RETURN(0);
}


/*----------------------------------------------------------------------
 * dump_surf_3dt - for each node index, get an appropriate node sampling,
 *                 and compute and output results across sub-bricks
 *----------------------------------------------------------------------
*/
int dump_surf_3dt ( smap_opts_t * sopt, param_t * p, node_list_t * N )
{
    range_3dmm_res r3mm_res;
    range_3dmm     r3mm;
    THD_ivec3      i3;				/* for coordinates */
    float          dist, min_dist, max_dist;
    int            sub, subs, nindex, findex = 0, index1d;
    int            oobc, oomc, index, max_index;

ENTRY("dump_surf_3dt");

    if ( sopt == NULL || p == NULL || N == NULL )
    {
	fprintf( stderr, "** smd_dmm : bad params (%p,%p,%p)\n", sopt, p, N );
	RETURN(-1);
    }

    /* one last precaution */
    if ( N->depth < 1 || N->nnodes <= 0 || sopt->f_steps < 1 )
    {
	fprintf( stderr, "** bad setup for mapping (%d,%d,%d)\n",
		 N->depth, N->nnodes, sopt->f_steps );
	RETURN(-1);
    }

    subs = DSET_NVALS(p->gpar);

    if ( ! sopt->no_head )
    {
	if ( vals_over_steps(sopt->map) )
	    print_header(p->outfp, N->labels[0], g_smap_names[sopt->map],
		         sopt->f_steps);
	else 
	    print_header(p->outfp, N->labels[0], g_smap_names[sopt->map], subs);
    }

    min_dist = 9999.9;						/* v2.3 */
    max_dist = -1.0;
    oobc     = 0; 			  /* init out-of-bounds counter */
    oomc     = 0; 			  /* init out-of-mask counter   */

    /* prepare range structs */
    r3mm.dset          = p->gpar;
    r3mm.debug         = 0;

    r3mm_res.ims.num   = 0;
    r3mm_res.ims.nall  = 0;
    r3mm_res.ims.imarr = NULL;
    r3mm_res.i3arr     = NULL;

    /* note, NodeList elements are in dicomm mm orientation */

    for ( nindex = 0; nindex < N->nnodes; nindex++ )
    {
	/* init default max for oob and oom cases */
	max_index = vals_over_steps(sopt->map) ? sopt->f_steps : subs;

	/* note the endpoints */
	if ( sopt->use_norms )
	{
	    /* first apply normals, then transform to current 3dmm */
	    r3mm.p1 = N->nodes[nindex];
	    directed_dist(r3mm.pn.xyz, r3mm.p1.xyz,
		          N->norms[0]+3*nindex, sopt->norm_len);

	    r3mm.p1 = THD_dicomm_to_3dmm(p->gpar, r3mm.p1);
	    r3mm.pn = THD_dicomm_to_3dmm(p->gpar, r3mm.pn);
	}
	else
	{
	    r3mm.p1 = THD_dicomm_to_3dmm(p->gpar, N->nodes[nindex]);

	    if ( N->depth > 1 )
		r3mm.pn = THD_dicomm_to_3dmm(p->gpar,
			  N->nodes[nindex+N->nnodes]);
	    else
		r3mm.pn = r3mm.p1;
	}

	/* make any user-defined ajustments */
	v2s_adjust_endpts( sopt, &r3mm.p1, &r3mm.pn );

	/* if either point is outside our dataset, skip the pair   v2.3 */
	if ( f3mm_out_of_bounds( &r3mm.p1, &p->f3mm_min, &p->f3mm_max ) ||
	     f3mm_out_of_bounds( &r3mm.pn, &p->f3mm_min, &p->f3mm_max ) )
	{
	    oobc++;

	    /* if user requests, display default info */
	    if ( p->oob.show )
		print_default_line( p->outfp, max_index, nindex,
			p->oob.index, p->oob.index, p->oob.index, p->oob.index, 
			p->oob.value );

	    continue;
	}

	dist = dist_f3mm( &r3mm.p1, &r3mm.pn );
	if ( dist < min_dist ) min_dist = dist;
	if ( dist > max_dist ) max_dist = dist;

	if ( sopt->debug > 0 )
	{
	    if ( nindex == sopt->dnode )
	    {
		fprintf(stderr,"-- debug node: %d\n", nindex );
		r3mm.debug = sopt->debug;
	    }
	    else if ( r3mm.debug )
		r3mm.debug = 0;
	}

	if ( segment_imarr( &r3mm_res, &r3mm, sopt ) != 0 )
	    continue;

	if ( r3mm_res.ims.num == 0 )	/* any good voxels in the bunch? */
	{
	    oomc++;

	    /* if user requests, display default info */
	    if ( p->oom.show )
		print_default_line( p->outfp, max_index, nindex,
			r3mm_res.ifirst, r3mm_res.i3first.ijk[0],
			r3mm_res.i3first.ijk[1], r3mm_res.i3first.ijk[2],
			p->oom.value );
	    continue;
	}

	/* get element 0, just for the findex */
	(void)v2s_apply_filter(&r3mm_res, sopt, 0, &findex);

	/* now get 3D and 1D coordinates */
	i3 = r3mm_res.i3arr[findex];
	index1d = i3.ijk[0] + DSET_NX(r3mm.dset) *
	    	 (i3.ijk[1] + DSET_NY(r3mm.dset) * i3.ijk[2] );

	/* output surface, volume, ijk indices and nvoxels*/
	fprintf( p->outfp, "  %8d   %8d   %3d  %3d  %3d     %3d",
	         nindex, index1d,
		 i3.ijk[0], i3.ijk[1], i3.ijk[2], r3mm_res.ims.num);

	/* Hey, these numbers are why I'm writing the program, woohoo! */
	/* Decide to print over steps or sub-bricks.                   */
	max_index = vals_over_steps(sopt->map) ? r3mm_res.ims.num : subs;

	for ( index = 0; index < max_index; index++ )
	    fprintf( p->outfp, "  %10s",
		MV_format_fval(v2s_apply_filter(&r3mm_res,sopt,index,NULL)));

	/* possibly fill line with default if by steps and short */
	if ( vals_over_steps(sopt->map) && (max_index < sopt->f_steps) )
	    for ( index = max_index; index < sopt->f_steps; index++ )
		fprintf(p->outfp, "  %10s", MV_format_fval(0.0));
	fputc( '\n', p->outfp );

	if ( (sopt->debug > 1) && (nindex == sopt->dnode) )
	{
	    fprintf(stderr, "-- nindex, findex, index1d = %d, %d, %d\n",
		    nindex, findex, index1d );
	    if ( sopt->use_norms )
		fprintf(stderr,"-- normal %f, %f, %f\n", N->norms[0][3*nindex],
			N->norms[0][3*nindex+1], N->norms[0][3*nindex+2]);
	    disp_mri_imarr( "++ raw data: ", &r3mm_res.ims );
	}

	/* clean up the MRI_IMARR struct, but don't free imarr */
	for ( sub = 0; sub < r3mm_res.ims.num; sub++ )
	{
	    mri_free(r3mm_res.ims.imarr[sub]);
	    r3mm_res.ims.imarr[sub] = NULL;
	}
	r3mm_res.ims.num = 0;
    }

    if ( sopt->debug > 0 )					/* v2.3 */
    {
	fprintf( stderr, "-- node pair dist (min,max) = (%f,%f)\n",
		 min_dist, max_dist );
	fprintf( stderr, "-- out-of-bounds, o-o-mask counts : %d, %d (of %d)\n",
		 oobc,oomc,N->nnodes);
    }

    /* now we can free the imarr and voxel lists */
    if ( r3mm_res.ims.nall > 0 )
    {
	free(r3mm_res.ims.imarr);
	free(r3mm_res.i3arr);
	r3mm_res.ims.nall = 0;
    }

    RETURN(0);
}


/***********************************************************************
 * print_default_lint	- special case of output
 *
 * return 0 on success
 ***********************************************************************
*/
int print_default_line( FILE * fp, int max_ind, int node_ind,
			int vind, int i, int j, int k, float fval )
{
    int cc;

ENTRY("print_default_line");

    if ( !fp )
	RETURN(-1);

    fprintf( fp, "  %8d   %8d   %3d  %3d  %3d     %3d",
	     node_ind, vind, i, j, k, max_ind );

    for ( cc = 0; cc < max_ind; cc++ )
	fprintf( fp, "  %10s", MV_format_fval(fval));
    fputc( '\n', fp );

    RETURN(0);
}


/***********************************************************************
 * segment_imarr	- get MRI_IMARR for steps over a segment
 *
 * The res->ims structure should be empty, except that it may
 * optionally contain space for pointers in imarr.  Note that nall
 * should be accurate.
 *
 * return 0 on success
 ***********************************************************************
*/
int segment_imarr( range_3dmm_res * res, range_3dmm * R, smap_opts_t * sopt )
{
    THD_fvec3 f3mm;
    THD_ivec3 i3ind;
    float     rat1, ratn;
    int       nx, ny;
    int       step, vindex, prev_ind;

ENTRY("segment_imarr");

    /* check params for validity */
    if ( !R || !sopt || !res || !R->dset )
    {
	fprintf(stderr, "** seg_imarr: invalid params (%p,%p,%p)\n",R,sopt,res);
	if ( R ) disp_range_3dmm("segment_imarr: bad inputs:", R );
	RETURN(-1);
    }

    if ( R->debug > 1 )
	disp_range_3dmm("segment_imarr: ", R );

    /* handle this as an acceptable, trivial case */
    if ( sopt->f_steps < 1 )
    {
	res->ims.num = 0;
	RETURN(0);
    }

    nx = DSET_NX(R->dset);
    ny = DSET_NY(R->dset);

    /* if we don't have enough memory for results, (re)allocate some */
    if ( res->ims.nall < sopt->f_steps )
    {
	if ( R->debug > 1 )
	    fprintf(stderr,"++ realloc of imarr (from %d to %d pointers)\n",
		    res->ims.nall,sopt->f_steps);

	res->ims.nall  = sopt->f_steps;
	res->ims.imarr = realloc(res->ims.imarr,
                                 sopt->f_steps*sizeof(MRI_IMAGE *));
	res->i3arr     = realloc(res->i3arr, sopt->f_steps*sizeof(THD_ivec3));
	if ( !res->ims.imarr || !res->i3arr )
	{
	    fprintf(stderr,"** seg_imarr: no memory for %d MRI_IMAGE ptrs\n",
		    sopt->f_steps);
	    res->ims.nall = res->ims.num = 0;
	    /* one might be good */
	    if ( res->ims.imarr ) free(res->ims.imarr);
	    if ( res->i3arr )     free(res->i3arr);
	    RETURN(-1);
	}
    }

    /* init return structure */
    res->ims.num = 0;
    res->masked  = 0;
    res->i3first = THD_3dmm_to_3dind( R->dset, R->p1 );
    res->ifirst  = res->i3first.ijk[0] +
	           nx * (res->i3first.ijk[1] + ny * res->i3first.ijk[2] );

    prev_ind = -1;			/* in case we want unique voxels */

    for ( step = 0; step < sopt->f_steps; step++ )
    {
	/* set our endpoint ratios */
	if ( sopt->f_steps < 2 )     /* if this is based on a single point */
	    ratn = 0.0;
	else
	    ratn = (float)step / (sopt->f_steps - 1);
	rat1 = 1.0 - ratn;

	f3mm.xyz[0] = rat1 * R->p1.xyz[0] + ratn * R->pn.xyz[0];
	f3mm.xyz[1] = rat1 * R->p1.xyz[1] + ratn * R->pn.xyz[1];
	f3mm.xyz[2] = rat1 * R->p1.xyz[2] + ratn * R->pn.xyz[2];

	i3ind = THD_3dmm_to_3dind( R->dset, f3mm );
	vindex = i3ind.ijk[0] + nx * (i3ind.ijk[1] + ny * i3ind.ijk[2] );

	/* is this voxel masked out? */
	if ( sopt->cmask && !sopt->cmask[vindex] )
	{
	    res->masked++;
	    continue;
	}

	/* is this voxel repeated, and if so, do we skip it? */
	if ( sopt->f_index == V2S_M2_INDEX_VOXEL && vindex == prev_ind )
	    continue;

	/* Huston, we have a good voxel... */

	prev_ind    = vindex;		/* note this for next time  */

	/* now for the big finish, get and insert the actual data */

	res->i3arr    [res->ims.num] = i3ind;	/* store the 3-D indices */
	res->ims.imarr[res->ims.num] = THD_extract_series( vindex, R->dset, 0 );
	res->ims.num++;

	if ( R->debug > 2 )
	    fprintf(stderr, "-- seg step %2d, vindex %d, coords %f %f %f\n",
		    step,vindex,f3mm.xyz[0],f3mm.xyz[1],f3mm.xyz[2]);
    }

    if ( R->debug > 0 )
	disp_range_3dmm_res( "++ i3mm_seg_imarr results: ", res );

    RETURN(0);
}


/*----------------------------------------------------------------------
 * create_node_list	- from surfaces
 *
 *----------------------------------------------------------------------
*/
int create_node_list ( smap_opts_t * sopt, node_list_t * N )
{
    int nsurf = 2;

ENTRY("create_node_list");

    if ( sopt == NULL || N == NULL )
    {
	fprintf( stderr, "** cnl - bad params (%p,%p)\n", sopt, N );
	RETURN(-1);
    }

    if ( (sopt->map == E_SMAP_MASK) || sopt->use_norms )
	nsurf = 1;

    if ( alloc_node_list( sopt, N, nsurf ) )
	RETURN(-1);

    if ( sopt->use_norms )
    {
	if ( ! N->norms[0] )
	{
	    fprintf(stderr,"** failure: surface '%s' has no normal list\n",
		    CHECK_NULL_STR(N->labels[0]));
	    RETURN(-1);
	}

	if ( sopt->debug > 1 )
	    fprintf(stderr,"-- have normals for surface '%s'\n",
		CHECK_NULL_STR(N->labels[0]));

	if ( ! sopt->keep_norm_dir && check_norm_dirs(sopt, N, 0) )
	    RETURN(-1);
    }

    RETURN(0);
}


/*----------------------------------------------------------------------
 * check_norm_dirs	- check to reverse normal directions
 *
 * Count how many of six bounding points point away from the center.
 *     6   : perfect, no change
 *     5   : provide warning, but apply no change
 *     2-4 : cannot determine direction
 *           --> this is a program terminating failure condition
 *     1   : provide warning, and reverse direction
 *     0   : no warning, just reverse direction
 *
 * return   0 : success (cases 0,1,5,6)
 *         -1 : failure (cases 2-4)
 *----------------------------------------------------------------------
*/
int check_norm_dirs ( smap_opts_t * sopt, node_list_t * N, int surf )
{
    THD_fvec3 * coords;
    float     * norms;
    float       fmin[3], fmax[3];
    int         min[3], max[3];
    int         match[6];		/* +/- 1, for direction match */
    int         node, c, ncount;

ENTRY("check_norm_dirs");

    norms  = N->norms[surf];		 /* point to nodes for this surface  */
    coords = N->nodes + surf*N->nnodes;	 /* point to coords for this surface */

    /* init mins and max's from node 0 */
    min[0] = max[0] = min[1] = max[1] = min[2] = max[2] = 0;

    fmin[0] = fmax[0] = coords->xyz[0];
    fmin[1] = fmax[1] = coords->xyz[1];
    fmin[2] = fmax[2] = coords->xyz[2];
    coords++;

    /* now check the rest of them */
    for ( node = 1; node < N->nnodes; node++, coords++ )
    {
	/* for each of x, y and z, check min and max */
	for ( c = 0; c < 3; c++ )
	{
	    if ( fmin[c] > coords->xyz[c] )
	    {
		min[c]  = node;
		fmin[c] = coords->xyz[c];
	    }

	    if ( fmax[c] < coords->xyz[c] )
	    {
		max[c]  = node;
		fmax[c] = coords->xyz[c];
	    }
	}
    }
    coords = N->nodes + surf*N->nnodes;	 /* reset coords to the beginning */

    if ( sopt->debug > 1 )
    {
	fprintf(stderr,"++ normals:\n"
		"      mins       : %d, %d, %d\n"
		"      min coords : %f, %f, %f\n"
		"      mins0      : %f, %f, %f\n"
		"      mins1      : %f, %f, %f\n"
		"      mins2      : %f, %f, %f\n"
		"\n"
		"      maxs       : %d, %d, %d\n"
		"      max coords : %f, %f, %f\n"
		"      maxs0      : %f, %f, %f\n"
		"      maxs1      : %f, %f, %f\n"
		"      maxs2      : %f, %f, %f\n",
		min[0], min[1], min[2], 
		fmin[0], fmin[1], fmin[2], 
		coords[min[0]].xyz[0], coords[min[0]].xyz[1],
				       coords[min[0]].xyz[2], 
		coords[min[1]].xyz[0], coords[min[1]].xyz[1],
				       coords[min[1]].xyz[2], 
		coords[min[2]].xyz[0], coords[min[2]].xyz[1],
				       coords[min[2]].xyz[2], 
		max[0], max[1], max[2], 
		fmax[0], fmax[1], fmax[2], 
		coords[max[0]].xyz[0], coords[max[0]].xyz[1],
				       coords[max[0]].xyz[2], 
		coords[max[1]].xyz[0], coords[max[1]].xyz[1],
				       coords[max[1]].xyz[2], 
		coords[max[2]].xyz[0], coords[max[2]].xyz[1],
				       coords[max[2]].xyz[2] );

    }

    /* now count the number of normals pointing "away from" the center */
    /* fixed directions - inconsistent usage       07 Apr 2004 [rickr] */
    match[0] = norms[3*min[0]  ] < 0;
    match[1] = norms[3*min[1]+1] < 0;
    match[2] = norms[3*min[2]+2] < 0;

    match[3] = norms[3*max[0]  ] > 0;
    match[4] = norms[3*max[1]+1] > 0;
    match[5] = norms[3*max[2]+2] > 0;

    if ( sopt->debug > 1 )
	fprintf(stderr,"-- matches[0..5] = (%d, %d, %d,  %d, %d, %d)\n",
		match[0], match[1], match[2], match[3], match[4], match[5] );

    ncount = 0;
    for ( c = 0; c < 6; c++ )
	if ( match[c] )
	    ncount++;

    /* do we fail? */
    if ( (ncount >= 2) && (ncount <= 4) )
    {
	fprintf(stderr, "** cannot determine directions for normals:\n"
		"   Option '-keep_norm_dir' is required to proceed.\n"
		"   It is ~%d%% likely that you will want to negate the\n"
		"   normal length in '-norm_len'\n",
		(int)(100*ncount/6.0) );
	RETURN(-1);
    }

    /* or do we just warn the user? */
    if ( (ncount == 1) || (ncount == 5) )
	fprintf(stderr,"** warning: only 83%% sure of direction of normals\n");
    
    /* do we need to reverse the direction? */
    if ( ncount < 2 )
    {
	fprintf(stderr,"-- reversing direction of normals\n");
	sopt->norm_len *= -1.0;
    }

    RETURN(0);
}


/*----------------------------------------------------------------------
 * alloc_node_list - create node list for mask mapping
 *
 * Allocate space for node list(s), and fill with xyz coordinates.
 *
 * return -1 : on error
 *         0 : on success
 *----------------------------------------------------------------------
*/
int alloc_node_list ( smap_opts_t * sopt, node_list_t * N, int nsurf )
{
    SUMA_SurfaceObject ** so;
    THD_fvec3          *  fvp;
    float              *  fp, radius[V2S_MAX_SURFS];
    int                   rv, nindex, sindex;

ENTRY("alloc_node_list");

    if ( sopt == NULL || N == NULL || nsurf < 0 )
    {
	fprintf( stderr, "** anl: bad params (%p,%p,%d)\n",
		 sopt, N, nsurf );
	RETURN(-1);
    }

    /* create a temporary list of surface pointers */
    so = (SUMA_SurfaceObject **)calloc(nsurf, sizeof(SUMA_SurfaceObject *));
    if ( so == NULL )
    {
	fprintf( stderr, "** anl: failed to alloc %d surf pointers\n", nsurf );
	RETURN(-1);
    }

    if ( (rv = get_mappable_surfs( so, nsurf, sopt->debug )) != nsurf )
    {
	fprintf( stderr, "** found %d mappable surfaces (but expected %d)\n",
		 rv, nsurf );
	free(so);
	RETURN(-1);
    }

    /* fill node list struct */

    N->depth     = nsurf;
    N->nnodes    = so[0]->N_Node;
    N->center[0] = so[0]->Center[0];
    N->center[1] = so[0]->Center[1];
    N->center[2] = so[0]->Center[2];
    N->nodes = (THD_fvec3 *)malloc(N->depth*N->nnodes*(int)sizeof(THD_fvec3));
    if ( N->nodes == NULL )
    {
	fprintf( stderr, "** cnlm: failed to allocate %d THD_fvec3 structs\n",
		 N->depth * N->nnodes );
	free(so);
	RETURN(-1);
    }

    N->labels = (char **)malloc(N->depth * sizeof(char *));
    if ( N->labels == NULL )
    {
	fprintf(stderr,"** cnlm: failed to allocate for %d labels\n",N->depth);
	free(so);
	free(N->nodes);
	RETURN(-1);
    }

    /* per surf, get label, norms, and copy xyz coords for each node */

    fvp = N->nodes;	/* linear coverage of all nodes */
    for ( sindex = 0; sindex < nsurf; sindex++ )
    {
	if ( so[sindex]->N_Node != N->nnodes )
	{
	    fprintf( stderr, "** surface has %d nodes (but expected %d)\n",
		     so[sindex]->N_Node, N->nnodes );
	    free( N->nodes );  N->nodes = NULL;
	    free(so);
	    RETURN(-1);
	}

	N->labels[sindex] = so[sindex]->Label;
	N->norms [sindex] = so[sindex]->NodeNormList;

	for ( nindex = 0, fp = so[sindex]->NodeList;
	      nindex < N->nnodes;
	      nindex++, fp += 3 )
	{
	    memcpy( fvp->xyz, fp, 3*sizeof(float) );
	    fvp++;
	}

	surf_ave_radius(radius+sindex, so[sindex], sopt->debug);
    }

#if 0		/* with -surf_A and -surf_B, intended order is known   v3.8 */
    if ( nsurf == 2 && !sopt->f_kso )
	verify_2surf_order(radius, N, sopt->debug);
#endif

    if ( sopt->debug > 1 )
	fprintf(stderr, "++ allocated %d x %d (x %d) node list\n",
		N->depth, N->nnodes, sizeof(THD_fvec3));

    free(so);
    RETURN(0);
}


/*----------------------------------------------------------------------
 * get_mappable_surfs - return mappable surface objects
 *
 * return the number of surfaces found
 *----------------------------------------------------------------------
*/
int get_mappable_surfs( SUMA_SurfaceObject ** slist, int how_many, int debug )
{
    SUMA_SurfaceObject * so;
    int			 count, socount = 0;

ENTRY("get_mappable_surfs");

    if ( slist == NULL )
    {
	fprintf( stderr, "** gms: missing slist!\n" );
	RETURN(-1);
    }

    for ( count = 0; count < SUMAg_N_DOv; count++ )
    {
	if ( ! SUMA_isSO(SUMAg_DOv[count]) )
	    continue;

	so = (SUMA_SurfaceObject *)SUMAg_DOv[count].OP;

	if ( ! so->AnatCorrect )
	{
	    if ( debug )
		fprintf(stderr,"-- surf #%d '%s', anat not correct, skipping\n",
			socount, CHECK_NULL_STR(so->Label));
            if ( debug > 1 )
                fprintf(stderr,"** consider adding the following to the "
                               "surface definition in the spec file:\n"
                               "       Anatomical = Y\n");
	    continue;
	}

	if ( debug > 2 )
	{
	    fprintf( stderr, "\n---------- surface #%d '%s' -----------\n",
		     socount, CHECK_NULL_STR(so->Label) );
	    SUMA_Print_Surface_Object( so, stderr );
	}

	if ( socount < how_many )	/* store a good surface */
	    slist[socount] = so;

	socount++;
    }

    if ( debug > 1 )
	fprintf( stderr, "++ found %d mappable surfaces\n", socount );

    RETURN(socount);
}


/*----------------------------------------------------------------------
 * set_smap_opts  - fill smap_opts_t struct
 *
 * return  0 : success
 *        -1 : error condition
 *----------------------------------------------------------------------
*/
int set_smap_opts( opts_t * opts, param_t * p, smap_opts_t * sopt )
{
    int nsurf = 1;

ENTRY("set_smap_opts");

    memset( sopt, 0, sizeof(*sopt) );

    if ( (sopt->map = check_map_func( opts->map_str )) == E_SMAP_INVALID )
	RETURN(-1);

    /* set defaults before checking map type */

    sopt->debug         = opts->debug;
    sopt->dnode         = opts->dnode;
    sopt->no_head       = opts->no_head;
    sopt->use_norms     = opts->use_norms;
    sopt->norm_len      = opts->norm_len;
    sopt->keep_norm_dir = opts->keep_norm_dir;

    sopt->f_index = V2S_M2_INDEX_VOXEL;	     /* default is "voxel" */

    if ( (opts->f_index_str != NULL) &&
	 (!strncmp(opts->f_index_str, "node", 4)) )
	    sopt->f_index = V2S_M2_INDEX_NODE;

    if ( opts->f_steps <= V2S_M2_STEPS_DEFAULT )	/* default is 2    */
	sopt->f_steps = V2S_M2_STEPS_DEFAULT;
    else
	sopt->f_steps = opts->f_steps;

#if 0
    sopt->f_kso = opts->f_kso;
#endif

    sopt->f_p1_fr = opts->f_p1_fr;         /* copy fractions & distances */
    sopt->f_pn_fr = opts->f_pn_fr;
    sopt->f_p1_mm = opts->f_p1_mm;
    sopt->f_pn_mm = opts->f_pn_mm;

    sopt->cmask = p->cmask;

    /* great, now my switch is ineffective - save for later */
    switch (sopt->map)
    {
	default:
	    break;

	case E_SMAP_AVE:
	case E_SMAP_COUNT:
	case E_SMAP_MAX:
	case E_SMAP_MAX_ABS:
	case E_SMAP_MIN:
	case E_SMAP_MASK2:
	case E_SMAP_SEG_VALS:
	case E_SMAP_MEDIAN:
	case E_SMAP_MODE:
	    nsurf = 2;
	    break;

	case E_SMAP_MIDPT:	/* continue to SMAP_MASK */
	    nsurf = 2;

	case E_SMAP_MASK:
	    if (sopt->f_steps != V2S_M2_STEPS_DEFAULT)
	    {
		fprintf(stderr,"** -f_steps option not valid\n");
		RETURN(-1);
	    }

	    /* we will only use the first point in the computation */
	    sopt->f_steps = 1;
	    break;
    }

    if ( (nsurf == 2) && !opts->snames[1] && !opts->use_norms )
    {
	fprintf(stderr, "** function '%s' requires 2 surfaces\n",
		        g_smap_names[sopt->map]);
	RETURN(-1);
    }

    if ( opts->debug > 0 )
	disp_smap_opts_t( "++ smap opts set :", sopt );

    RETURN(0);
}


/*----------------------------------------------------------------------
 * free memory, close output file
 *----------------------------------------------------------------------
*/
int final_clean_up ( opts_t * opts, param_t * p, SUMA_SurfSpecFile * spec,
		     node_list_t * N )
{
    if (N->nodes)   free(N->nodes);
    if (N->labels)  free(N->labels);

    if ( ( SUMAg_DOv != NULL ) &&
	 ( SUMA_Free_Displayable_Object_Vect(SUMAg_DOv, SUMAg_N_DOv) == 0 ) )
	fprintf(stderr, "** failed SUMA_Free_Displayable_Object_Vect()\n" );

    if ( ( SUMAg_SVv != NULL ) &&
	 ( SUMA_Free_SurfaceViewer_Struct_Vect(SUMAg_SVv, SUMAg_N_SVv) == 0 ) )
	fprintf( stderr, "** failed SUMA_Free_SurfaceViewer_Struct_Vect()\n" );

    if ( ( SUMAg_CF != NULL ) && ( SUMA_Free_CommonFields(SUMAg_CF) == 0 ) )
	fprintf( stderr, "** failed SUMA_Free_CommonFields()\n" );

    RETURN(0);
}


/*----------------------------------------------------------------------
 * read surfaces (much stolen from SUMA_suma.c - thanks Ziad!)
 *----------------------------------------------------------------------
*/
int read_surf_files ( opts_t * opts, param_t * p, SUMA_SurfSpecFile * spec )
{
    int debug, rv;					/* v3.5 [rickr] */
    
ENTRY("read_surf_files");

    debug = (opts->debug > 2);

    if ( debug )
	fputs( "-- SUMA_Create_CommonFields()...\n", stderr );

    /* initialize common fields struct */
    SUMAg_CF = SUMA_Create_CommonFields();

    if ( SUMAg_CF == NULL )
    {
	fprintf( stderr, "** failed SUMA_Create_CommonFields(), exiting...\n" );
	RETURN(-1);
    }

    /* for SUMA type notifications */
    if ( opts->debug > 3 )
    {
	SUMAg_CF->MemTrace = 1;

	if ( opts->debug > 4 )
	    SUMAg_CF->InOut_Notify = 1;
    }

    if ( debug )
	fputs( "-- SUMA_Alloc_DisplayObject_Struct()...\n", stderr );

    SUMAg_DOv = SUMA_Alloc_DisplayObject_Struct(SUMA_MAX_DISPLAYABLE_OBJECTS);

    if ( debug )	/* can't SUMA_ShowSpecStruct() yet    - v3.9 */
	fputs( "-- SUMA_Read_SpecFile()...\n", stderr );

    if ( SUMA_Read_SpecFile( opts->spec_file, spec) == 0 )
    {
	fprintf( stderr, "** failed SUMA_Read_SpecFile(), exiting...\n" );
	RETURN(-1);
    }

    rv = SUMA_spec_select_surfs(spec, opts->snames, V2S_MAX_SURFS, opts->debug);
    if ( rv < 1 )
    {
	if ( rv == 0 )
	    fprintf(stderr,"** no named surfaces found in spec file\n");
	RETURN(-1);
    }

    if ( debug )
	SUMA_ShowSpecStruct(spec, stderr, opts->debug > 2 ? 3 : 1);

    if ( SUMA_spec_set_map_refs(spec, opts->debug) != 0 )
	RETURN(-1);

    /* make sure only group was read from spec file */
    if ( spec->N_Groups != 1 )
    {
	fprintf( stderr,"** error: N_Groups <%d> must be 1 in spec file <%s>\n",
		 spec->N_Groups, opts->spec_file );
	RETURN(-1);
    }

    if ( debug )
	fputs( "-- SUMA_LoadSpec_eng()...\n", stderr );

    /* actually load the surface(s) from the spec file */
    if (SUMA_LoadSpec_eng(spec,SUMAg_DOv,&SUMAg_N_DOv,opts->sv_file,debug,
	     SUMAg_CF->DsetList) == 0)     /* DsetList   26 Mar 2004 [ziad] */
    {
	fprintf( stderr, "** error: failed SUMA_LoadSpec_eng(), exiting...\n" );
	RETURN(-1);
    }

    if ( opts->debug > 1 )
	fprintf(stderr, "++ %d surfaces loaded.\n", spec->N_Surfs );

    RETURN(0);
}


/*----------------------------------------------------------------------
 * init_options - fill opts struct, display help
 *----------------------------------------------------------------------
*/
int init_options ( opts_t * opts, int argc, char * argv [] )
{
    int ac, ind;

ENTRY("init_options");

    if ( argc < 2 )
    {
	usage( PROG_NAME, V2S_USE_LONG );
	RETURN(-1);
    }

    /* clear out the options structure */
    memset( opts, 0, sizeof( opts_t) );
    opts->gpar_file   = NULL;
    opts->out_file    = NULL;
    opts->spec_file   = NULL;
    opts->sv_file     = NULL;
    opts->cmask_cmd   = NULL;
    opts->map_str     = NULL;
    opts->snames[0]   = NULL;
    opts->snames[1]   = NULL;
    opts->f_index_str = NULL;

    opts->norm_len    = 1.0;		/* init to 1.0 millimeter    */
    opts->dnode       = -1;		/* init to something invalid */

    for ( ac = 1; ac < argc; ac++ )
    {
	/* alphabetical... */
	if ( ! strncmp(argv[ac], "-cmask", 6) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -cmask COMMAND\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->cmask_cmd = argv[++ac];
	}
	else if ( ! strncmp(argv[ac], "-debug", 6) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -debug LEVEL\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->debug = atoi(argv[++ac]);
	    if ( opts->debug < 0 || opts->debug > V2S_DEBUG_MAX_LEV )
	    {
		fprintf( stderr, "bad debug level <%d>, should be in [0,%d]\n",
			opts->debug, V2S_DEBUG_MAX_LEV );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }
	}
	else if ( ! strncmp(argv[ac], "-dnode", 6) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -dnode NODE_NUM\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->dnode = atoi(argv[++ac]);
	}
	else if ( ! strncmp(argv[ac], "-f_index", 7) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -f_index INDEX_TYPE\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->f_index_str = argv[++ac];
	}
	else if ( ! strncmp(argv[ac], "-f_keep_surf_order", 9) )
	{
	    /*  opts->f_kso = 1;		v3.8 */

	    fprintf(stderr,"** the -f_keep_surf_order option is depreciated\n"
		    "   in favor of -surf_A and -surf_B (version 3.8)\n");
	    RETURN(-1);
	}
	else if ( ! strncmp(argv[ac], "-f_p1_fr", 9) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -f_p1_fr FRACTION\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->f_p1_fr = atof(argv[++ac]);
	}
	else if ( ! strncmp(argv[ac], "-f_pn_fr", 9) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -f_pn_fr FRACTION\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->f_pn_fr = atof(argv[++ac]);
	}
	else if ( ! strncmp(argv[ac], "-f_p1_mm", 9) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -f_p1_mm DISTANCE\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->f_p1_mm = atof(argv[++ac]);
	}
	else if ( ! strncmp(argv[ac], "-f_pn_mm", 9) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -f_pn_mm DISTANCE\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->f_pn_mm = atof(argv[++ac]);
	}
	else if ( ! strncmp(argv[ac], "-f_steps", 7) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -f_steps NUM_STEPS\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->f_steps = atoi(argv[++ac]);
	}
	else if ( ! strncmp(argv[ac], "-help", 5) )
	{
	    usage( PROG_NAME, V2S_USE_LONG );
	    RETURN(-1);
	}
	else if ( ! strncmp(argv[ac], "-hist", 5) )
	{
	    usage( PROG_NAME, V2S_USE_HIST );
	    RETURN(-1);
	}
	else if ( ! strncmp(argv[ac], "-keep_norm_dir", 14) )
	{
	    opts->keep_norm_dir = 1;
	}
	else if ( ! strncmp(argv[ac], "-grid_parent", 5) ||
		  ! strncmp(argv[ac], "-inset", 6)       ||
		  ! strncmp(argv[ac], "-input", 6) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -grid_parent INPUT_DSET\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->gpar_file = argv[++ac];
	}
	else if ( ! strncmp(argv[ac], "-map_func", 4) )  /* mapping function */
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -map_func FUNCTION\n\n", stderr );
		RETURN(-1);
	    }

	    opts->map_str = argv[++ac];	    /* store user string for now */
	}
	else if ( ! strncmp(argv[ac], "-no_headers", 5) )
	{
	    opts->no_head = 1;
	}
	else if ( ! strncmp(argv[ac], "-norm_len", 9) )
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -norm_len LENGTH\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->norm_len = atof(argv[++ac]);
	}
	else if ( ! strncmp(argv[ac], "-oob_index", 8) )
	{
	    if ( (ac+1) >= argc )
            {
		fputs( "option usage: -oob_index INDEX_VALUE\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->oob.show  = 1;
            opts->oob.index = atoi(argv[++ac]);
	}
	else if ( ! strncmp(argv[ac], "-oob_value", 8) )
	{
	    if ( (ac+1) >= argc )
            {
		fputs( "option usage: -oob_value VALUE\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->oob.show  = 1;
            opts->oob.value = atof(argv[++ac]);
	}
	else if ( ! strncmp(argv[ac], "-oom_value", 8) )
	{
	    if ( (ac+1) >= argc )
            {
		fputs( "option usage: -oob_value VALUE\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

	    opts->oom.show  = 1;
            opts->oom.value = atof(argv[++ac]);
	}
	else if ( ! strncmp(argv[ac], "-out_1D", 7) )
	{
	    if ( (ac+1) >= argc )
            {
		fputs( "option usage: -out_1D OUTPUT_FILE\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

            opts->out_file = argv[++ac];
	}
	else if ( ! strncmp(argv[ac], "-spec", 3) )
	{
	    if ( (ac+1) >= argc )
            {
		fputs( "option usage: -spec SPEC_FILE\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

            opts->spec_file = argv[++ac];
	}
	else if ( ! strncmp(argv[ac], "-surf_", 6) )
	{
	    if ( (ac+1) >= argc )
            {
		fputs( "option usage: -surf_X SURF_NAME\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }
	    ind = argv[ac][6] - 'A';
	    if ( (ind < 0) || (ind >= V2S_MAX_SURFS) )
	    {
		fprintf(stderr,"** -surf_X option: '%s' out of range,\n"
			"   use one of '-surf_A' through '-surf_%c'\n",
			argv[ac], 'A'+V2S_MAX_SURFS-1);
		RETURN(-1);
	    }

            opts->snames[ind] = argv[++ac];
	}
	else if ( ! strncmp(argv[ac], "-sv", 3) )
	{
	    if ( (ac+1) >= argc )
            {
		fputs( "option usage: -sv SURFACE_VOLUME\n\n", stderr );
		usage( PROG_NAME, V2S_USE_SHORT );
		RETURN(-1);
	    }

            opts->sv_file = argv[++ac];
	}
	else if ( ! strncmp(argv[ac], "-use_norms", 5) )
	{
	    opts->use_norms = 1;
	}
	else if ( ! strncmp(argv[ac], "-version", 2) )
	{
	    usage( PROG_NAME, V2S_USE_VERSION );
	    RETURN(-1);
	}
	else	 /* invalid option */
	{
	    fprintf( stderr, "invalid option <%s>\n", argv[ac] );
	    usage( PROG_NAME, V2S_USE_SHORT );
	    RETURN(-1);
	}
    }

    RETURN(0);
}

/*----------------------------------------------------------------------
 * validate_options - fill param struct from options
 *
 *     - validate datasets
 *     - validate surface
 *----------------------------------------------------------------------
*/
int validate_options ( opts_t * opts, param_t * p )
{
ENTRY("validate_options");

    memset( p, 0, sizeof(*p) );
    p->gpar  = NULL;
    p->outfp = NULL;
    p->cmask = NULL;

    if ( opts->debug > 0 )
    {
	usage( PROG_NAME, V2S_USE_VERSION );
	disp_opts_t ( "++ opts read: ", opts );
    }

    if ( check_map_func( opts->map_str ) == E_SMAP_INVALID )
	RETURN(-1);

    if ( opts->spec_file == NULL )
    {
	fprintf( stderr, "** missing '-spec_file SPEC_FILE' option\n" );
	RETURN(-1);
    }

    if ( opts->sv_file == NULL )
    {
	fprintf( stderr, "** missing '-sv SURF_VOL' option\n" );
	RETURN(-1);
    }

    if ( opts->snames[0] == NULL )
    {
	fprintf(stderr,"** missing '-surf_A SURF_NAME' option\n");
	RETURN(-1);
    }

    if ( opts->use_norms )
    {
	if ( opts->snames[1] )
	{
	    fprintf(stderr,"** no '-use_norms' option with 2 surfaces\n");
	    RETURN(-1);
	}
    }
    else if ( opts->norm_len != 1.0 || opts->keep_norm_dir )
    {
	fprintf(stderr,"** options for normals requires '-use_norms'\n");
	RETURN(-1);
    }

    if ( set_outfile( opts, p ) != 0 )
	RETURN(-1);

    if ( validate_datasets( opts, p ) != 0 )
	RETURN(-1);

    p->oob = opts->oob;		/* out of bounds info */
    p->oom = opts->oom;		/* out of bounds info */

    if ( p->oom.show && !p->cmask )
    {
	fprintf(stderr,"** '-cmask' option is required with '-oom_value'\n");
	RETURN(-1);
    }

    if ( opts->debug > 1 )
	disp_param_t( "++ opts validated: ", p );

    RETURN(0);
}


/*----------------------------------------------------------------------
 * decide where the output goes
 *----------------------------------------------------------------------
*/
int set_outfile( opts_t * opts, param_t * p )
{
ENTRY("set_outfile");

    if ( opts == NULL || p == NULL )
	RETURN(-1);

    if ( opts->out_file == NULL )
	p->outfp = stdout;
    else if ( strcmp( opts->out_file, "stdout" ) == 0 )
	p->outfp = stdout;
    else if ( strcmp( opts->out_file, "stderr" ) == 0 )
	p->outfp = stderr;
    else
    {
	if ( THD_is_file(opts->out_file) )
	{
	    fprintf( stderr, "** output file '%s' already exists\n",
		     opts->out_file );
	    RETURN(-1);
	}

	p->outfp = fopen( opts->out_file, "w" );
	if ( p->outfp == NULL )
	{
	    fprintf( stderr, "** failure to open '%s' for writing\n",
		     opts->out_file );
	    RETURN(-1);
	}
    }

    RETURN(0);
}


/*----------------------------------------------------------------------
 * check_map_func
 *
 *     - check for map_str
 *     - validate the map type
 *----------------------------------------------------------------------
*/
int check_map_func ( char * map_str )
{
    int map;

ENTRY("check_map_func");

    if ( map_str == NULL )
    {
	fprintf( stderr, "** missing option: '-map_func FUNCTION'\n" );
	RETURN(E_SMAP_INVALID);
    }

    map = smd_map_type( map_str );

    switch ( map )
    {
	default:
	    map = E_SMAP_INVALID;
	    break;

	case E_SMAP_COUNT:
	case E_SMAP_MASK2:
	    fprintf( stderr, "** function '%s' coming soon ...\n",
		     g_smap_names[map] );
	    RETURN(E_SMAP_INVALID);
	    break;

	case E_SMAP_AVE:
	case E_SMAP_MASK:
	case E_SMAP_MAX:
	case E_SMAP_MAX_ABS:
	case E_SMAP_MIN:
	case E_SMAP_MIDPT:
	case E_SMAP_SEG_VALS:
	case E_SMAP_MEDIAN:
	case E_SMAP_MODE:
	    break;
    }

    if ( map == E_SMAP_INVALID )
	fprintf( stderr, "** invalid map string '%s'\n", map_str );

    RETURN(map);
}


/*----------------------------------------------------------------------
 * smd_map_type - return an E_SMAP_XXX code
 *
 * on failure, return -1 (E_SMAP_INVALID)
 * else        return >0 (a valid map code)
 *----------------------------------------------------------------------
*/
int smd_map_type ( char * map_str )
{
    smap_nums map;

ENTRY("smd_map_type");

    if ( map_str == NULL )
    {
	fprintf( stderr, "** smd_map_type: missing map_str parameter\n" );
	RETURN((int)E_SMAP_INVALID);
    }

    if ( sizeof(g_smap_names) / sizeof(char *) != (int)E_SMAP_FINAL )
    {
	fprintf( stderr, "** error:  g_smap_names/smd_map_num mis-match\n");
	RETURN((int)E_SMAP_INVALID);
    }

    for ( map = E_SMAP_INVALID; map < E_SMAP_FINAL; map++ )
	if ( !strcmp( map_str, g_smap_names[map] ) )
	    RETURN((int)map);

    RETURN((int)E_SMAP_INVALID);
}


/*----------------------------------------------------------------------
 * validate_datasets
 *
 * Note that we do not validate the SURFACE_VOLUME AFNI dataset here.
 * That is done in SUMA_LoadSpec().
 *
 * Verify the AFNI dataset used for value output.
 * Check for a cmask dataset and command.
 * Verify that AFNI dataset and the mask have the same size.
 *----------------------------------------------------------------------
*/
int validate_datasets( opts_t * opts, param_t * p )
{
ENTRY("validate_datasets");

    p->gpar = THD_open_dataset( opts->gpar_file );

    if ( !ISVALID_DSET(p->gpar) )
    {
	if ( opts->gpar_file == NULL )
	    fprintf( stderr, "** error: missing '-grid_parent DSET' option\n" );
	else
	    fprintf( stderr, "** error: invalid input dataset '%s'\n",
		     opts->gpar_file);
	RETURN(-1);
    }
    else if ( DSET_BRICK_TYPE(p->gpar, 0) == MRI_complex )
    {
	fprintf(stderr,
		"** failure: cannot deal with complex-valued dataset, '%s'\n",
		opts->gpar_file);
	RETURN(-1);
    }

    p->nvox = DSET_NVOX( p->gpar );
    set_3dmm_bounds( p->gpar, &p->f3mm_min, &p->f3mm_max );

    /* -------------------------------------------------------------------- */
    /* check for cmask - casually stolen from 3dmaskdump.c (thanks, Bob! :) */

    if ( opts->cmask_cmd != NULL )
    {
	int    clen = strlen( opts->cmask_cmd );
	char * cmd;

	/* save original cmask command, as EDT_calcmask() is destructive */
	cmd = (char *)malloc((clen + 1) * sizeof(char));
	strcpy( cmd, opts->cmask_cmd );

	p->cmask = EDT_calcmask( cmd, &p->ncmask );

	free( cmd );			   /* free EDT_calcmask() string */

	if ( p->cmask == NULL )
	{
	    fprintf( stderr, "** failure: cannot compute mask from option:\n"
		     "   -cmask '%s'\n", opts->cmask_cmd );
	    RETURN(-1);
	}
	if ( p->ncmask != p->nvox )
	{
	    fprintf( stderr, "** error: input and cmask datasets do not have "
		     "the same dimensions\n" );
	    RETURN(-1);
	}
	if ( ( p->ccount = THD_countmask( p->ncmask, p->cmask ) ) <= 0 )
	{
	    fprintf( stderr, "** Warning!  No voxels in computed cmask!\n" );
	    /* return -1;   continue, and let the user deal with it...  */
	}
    }

    if ( opts->debug > 0 )
    {
	fprintf( stderr, "++ input dset has nvox = %d, nvals = %d",
		 p->nvox, DSET_NVALS(p->gpar) );
	if ( p->cmask == NULL )
	    fputc( '\n', stderr );
	else
	    fprintf( stderr, " (%d voxels in mask)\n", p->ccount );
    }

    RETURN(0);
}

/*----------------------------------------------------------------------
 * usage  -  output usage information
 *
 * V2S_USE_SHORT	- display brief output
 * V2S_USE_LONG		- display long output
 * V2S_USE_VERSION	- show the VERSION of the program
 *----------------------------------------------------------------------
*/
int usage ( char * prog, int level )
{
ENTRY("usage");

    if ( level == V2S_USE_SHORT )
    {
	fprintf( stderr,
		 "usage: %s [options] -spec SPEC_FILE -sv SURF_VOL "
		                    " -grid_parent AFNI_DSET\n"
		 "usage: %s -help\n",
		 prog, prog );
    }
    else if ( level == V2S_USE_LONG )
    {
	printf(
	    "\n"
	    "%s - map data from a volume domain to a surface domain\n"
	    "\n"
	    "  usage: %s [options] -spec SPEC_FILE -sv SURF_VOL \\\n"
	    "                    -grid_parent AFNI_DSET -map_func MAP_FUNC\n"
	    "\n"
            "This program is used to map data values from an AFNI volume\n"
            "dataset to a surface dataset.  A filter may be applied to the\n"
            "volume data to produce the value(s) for each surface node.\n"
            "\n"
            "The surface and volume domains are spacially matched via the\n"
            "'surface volume' AFNI dataset.  This gives each surface node xyz\n"
            "coordinates, which are then matched to the input 'grid parent'\n"
            "dataset.  This grid parent is an AFNI dataset containing the\n"
            "data values destined for output.\n"
            "\n"
            "Typically, two corresponding surfaces will be input (via the\n"
            "spec file and the '-surf_A' and '-surf_B' options), along with\n"
	    "a mapping function and relevant options.  The mapping function\n"
	    "will act as a filter over the values in the AFNI volume.\n"
            "\n"
            "Note that an alternative to using a second surface with the\n"
            "'-surf_B' option is to define the second surface by using the\n"
	    "normals from the first surface.  By default, the second surface\n"
	    "would be defined at a distance of 1mm along the normals, but the\n"
	    "user may modify the applied distance (and direction).  See the\n"
	    "'-use_norms' and '-norm_len' options for more details.\n"
            "\n"
            "For each pair of corresponding surface nodes, let NA be the node\n"
            "on surface A (such as a white/grey boundary) and NB be the\n"
            "corresponding node on surface B (such as a pial surface).  The\n"
	    "filter is applied to the volume data values along the segment\n"
	    "from NA to NB (consider the average or maximum as examples of\n"
	    "filters).\n"
	    "\n"
	    "Note: if either endpoint of a segment is outside the grid parent\n"
	    "      volume, that node (pair) will be skipped.\n"
            "\n"
            "Note: surface A corresponds to the required '-surf_A' argument,\n"
            "      while surface B corresponds to '-surf_B'.\n"
            "\n"
            "By default, this segment only consists of the endpoints, NA and\n"
            "NB (the actual nodes on the two surfaces).  However the number\n"
            "of evenly spaced points along the segment may be specified with\n"
            "the -f_steps option, and the actual locations of NA and NB may\n"
            "be altered with any of the -f_pX_XX options, covered below.\n"
	    "\n"
	    "As an example, for each node pair, one could output the average\n"
	    "value from some functional dataset along a segment of 10 evenly\n"
	    "spaced points, where the segment endpoints are defined by the\n"
	    "xyz coordinates of the nodes.  This is example 3, below.\n"
	    "\n"
	    "The mapping function (i.e. filter) is a required parameter to\n"
	    "the program.\n"
	    "\n"
	    "Brief descriptions of the current mapping functions are as\n"
	    "follows.  These functions are defined over a segment of points.\n"
	    "\n"
	    "    ave       : output the average of all voxel values along the\n"
	    "                segment\n"
	    "    mask      : output the voxel value for the trivial case of a\n"
	    "                segment - defined by a single surface point\n"
	    "    median    : output the median value from the segment\n"
	    "    midpoint  : output the dataset value at the segment midpoint\n"
	    "    mode      : output the mode of the values along the segment\n"
	    "    max       : output the maximum volume value over the segment\n"
	    "    max_abs   : output the dataset value with max abs over seg\n"
	    "    min       : output the minimum volume value over the segment\n"
	    "    seg_vals  : output _all_ volume values over the segment (one\n"
	    "                sub-brick only)\n"
	    "\n"
	    "  --------------------------------------------------\n"
	    "\n"
	    "  examples:\n"
	    "\n"
	    "    1. Apply a single surface mask to output volume values over\n"
	    "       each surface node.  Output is one value per sub-brick\n"
	    "       (per surface node).\n"
	    "\n"
	    "    %s                                \\\n"
	    "       -spec         fred.spec                \\\n"
	    "       -surf_A       smoothwm                 \\\n"
	    "       -sv           fred_anat+orig           \\\n"
	    "       -grid_parent  fred_anat+orig           \\\n"
	    "       -map_func     mask                     \\\n"
	    "       -out_1D       fred_anat_vals.1D\n"
	    "\n"
	    "    2. Apply a single surface mask to output volume values over\n"
	    "       each surface node.  In this case restrict input to the\n"
	    "       mask implied by the -cmask option.  Supply additional\n"
	    "       debug output, and more for surface node 1874\n"
	    "\n"
	    "    %s                                                \\\n"
	    "       -spec         fred.spec                                \\\n"
	    "       -surf_A       smoothwm                                 \\\n"
	    "       -sv           fred_anat+orig                           \\\n"
	    "       -grid_parent 'fred_epi+orig[0]'                        \\\n"
	    "       -cmask       '-a fred_func+orig[2] -expr step(a-0.6)'  \\\n"
	    "       -map_func     mask                                     \\\n"
	    "       -debug        2                                        \\\n"
	    "       -dnode        1874                                     \\\n"
	    "       -out_1D       fred_epi_vals.1D\n"
	    "\n"
	    "    3. Given a pair of related surfaces, for each node pair,\n"
	    "       break the connected line segment into 10 points, and\n"
	    "       compute the average dataset value over those points.\n"
	    "       Since the index is nodes, each of the 10 points will be\n"
	    "       part of the average.  This could be changed so that only\n"
	    "       values from distinct volume nodes are considered (by\n"
	    "       changing the -f_index from nodes to voxels).  Restrict\n"
	    "       input voxels to those implied by the -cmask option\n"
	    "       Output is one average value per sub-brick (per surface\n"
	    "       node).\n"
	    "\n"
	    "    %s                                                \\\n"
	    "       -spec         fred2.spec                               \\\n"
	    "       -surf_A       smoothwm                                 \\\n"
	    "       -surf_B       pial                                     \\\n"
	    "       -sv           fred_anat+orig                           \\\n"
	    "       -grid_parent  fred_func+orig                           \\\n"
	    "       -cmask        '-a fred_func+orig[2] -expr step(a-0.6)' \\\n"
	    "       -map_func     ave                                      \\\n"
	    "       -f_steps      10                                       \\\n"
	    "       -f_index      nodes                                    \\\n"
	    "       -out_1D       fred_func_ave.1D\n"
	    "\n"
	    "    4. Similar to example 3, but each of the node pair segments\n"
	    "       has grown by 10%% on the inside of the first surface,\n"
	    "       and 20%% on the outside of the second.  This is a 30%%\n"
	    "       increase in the length of each segment.  To shorten the\n"
	    "       node pair segment, use a '+' sign for p1 and a '-' sign\n"
	    "       for pn.\n"
	    "       As an interesting side note, '-f_p1_fr 0.5 -f_pn_fr -0.5'\n"
	    "       would give a zero length vector identical to that of the\n"
	    "       'midpoint' filter.\n"
	    "\n"
	    "    %s                                                \\\n"
	    "       -spec         fred2.spec                               \\\n"
	    "       -surf_A       smoothwm                                 \\\n"
	    "       -surf_B       pial                                     \\\n"
	    "       -sv           fred_anat+orig                           \\\n"
	    "       -grid_parent  fred_func+orig                           \\\n"
	    "       -cmask        '-a fred_func+orig[2] -expr step(a-0.6)' \\\n"
	    "       -map_func     ave                                      \\\n"
	    "       -f_steps      10                                       \\\n"
	    "       -f_index      voxels                                   \\\n"
	    "       -f_p1_fr      -0.1                                     \\\n"
	    "       -f_pn_fr      0.2                                      \\\n"
	    "       -out_1D       fred_func_ave2.1D\n"
	    "\n"
	    "    5. Similar to example 3, instead of computing the average\n"
	    "       across each segment (one average per sub-brick), output\n"
	    "       the volume value at _every_ point across the segment.\n"
	    "       The output here would be 'f_steps' values per node pair,\n"
	    "       though the output could again be restricted to unique\n"
	    "       voxels along each segment with '-f_index voxels'.\n"
	    "       Note that only sub-brick 0 will be considered here.\n"
	    "\n"
	    "    %s                                                \\\n"
	    "       -spec         fred2.spec                               \\\n"
	    "       -surf_A       smoothwm                                 \\\n"
	    "       -surf_B       pial                                     \\\n"
	    "       -sv           fred_anat+orig                           \\\n"
	    "       -grid_parent  fred_func+orig                           \\\n"
	    "       -cmask        '-a fred_func+orig[2] -expr step(a-0.6)' \\\n"
	    "       -map_func     seg_vals                                 \\\n"
	    "       -f_steps      10                                       \\\n"
	    "       -f_index      nodes                                    \\\n"
	    "       -out_1D       fred_func_segvals_10.1D\n"
	    "\n"
            "    6. Similar to example 5, but make sure there is output for\n"
            "       every node pair in the surfaces.  Since it is expected\n"
            "       that some nodes are out of bounds (meaning that they lie\n"
	    "       outside the domain defined by the grid parent dataset),\n"
	    "       the '-oob_value' option is added to include a default\n"
	    "       value of 0.0 in such cases.  And since it is expected\n"
	    "       that some node pairs are \"out of mask\" (meaning that\n"
	    "       their resulting segment lies entirely outside the cmask),\n"
	    "       the '-oom_value' was added to output the same default\n"
	    "       value of 0.0.\n"
	    "\n"
	    "    %s                                                \\\n"
	    "       -spec         fred2.spec                               \\\n"
	    "       -surf_A       smoothwm                                 \\\n"
	    "       -surf_B       pial                                     \\\n"
	    "       -sv           fred_anat+orig                           \\\n"
	    "       -grid_parent  fred_func+orig                           \\\n"
	    "       -cmask        '-a fred_func+orig[2] -expr step(a-0.6)' \\\n"
	    "       -map_func     seg_vals                                 \\\n"
	    "       -f_steps      10                                       \\\n"
	    "       -f_index      nodes                                    \\\n"
	    "       -oob_value    0.0                                      \\\n"
	    "       -oom_value    0.0                                      \\\n"
	    "       -out_1D       fred_func_segvals_10_all.1D\n"
	    "\n"
	    "    7. This is a basic example of calculating the average along\n"
	    "       each segment, but where the segment is produced by only\n"
	    "       one surface, along with its set of surface normals.  The\n"
	    "       segments will be 2.5 mm in length.\n"
	    "\n"
	    "    %s                                                \\\n"
	    "       -spec         fred2.spec                               \\\n"
	    "       -surf_A       smoothwm                                 \\\n"
	    "       -sv           fred_anat+orig                           \\\n"
	    "       -grid_parent  fred_anat+orig                           \\\n"
	    "       -use_norms                                             \\\n"
	    "       -norm_len     2.5                                      \\\n"
	    "       -map_func     ave                                      \\\n"
	    "       -f_steps      10                                       \\\n"
	    "       -f_index      nodes                                    \\\n"
	    "       -out_1D       fred_anat_norm_ave.2.5.1D\n"
	    "\n"
	    "  --------------------------------------------------\n"
	    "\n"
	    "  REQUIRED COMMAND ARGUMENTS:\n"
	    "\n"
	    "    -spec SPEC_FILE        : SUMA spec file\n"
	    "\n"
	    "        e.g. -spec fred.spec\n"
	    "\n"
	    "        The surface specification file contains the list of\n"
	    "        mappable surfaces that are used.\n"
	    "\n"
	    "        See @SUMA_Make_Spec_FS and @SUMA_Make_Spec_SF.\n"
	    "\n"
	    "    -surf_A SURF_NAME      : name of surface A (from spec file)\n"
	    "    -surf_B SURF_NAME      : name of surface B (from spec file)\n"
	    "\n"
	    "        e.g. -surf_A smoothwm\n"
	    "        e.g. -surf_A lh.smoothwm\n"
	    "        e.g. -surf_B lh.pial\n"
	    "\n"
	    "        This is used to specify which surface(s) will be used by\n"
	    "        the program.  The '-surf_A' parameter is required, as it\n"
	    "        specifies the first surface, whereas since '-surf_B' is\n"
	    "        used to specify an optional second surface, it is not\n"
	    "        required.\n"
	    "\n"
	    "        Note that any need for '-surf_B' may be fulfilled using\n"
	    "        the '-use_norms' option.\n"
	    "\n"
	    "        Note that any name provided must be in the spec file,\n"
	    "        uniquely matching the name of a surface node file (such\n"
	    "        as lh.smoothwm.asc, for example).  Note that if both\n"
	    "        hemispheres are represented in the spec file, then there\n"
	    "        may be both lh.pial.asc and rh.pial.asc, for instance.\n"
	    "        In such a case, 'pial' would not uniquely determine a\n"
	    "        a surface, but the name 'lh.pial' would.\n"
	    "\n"
	    "    -sv SURFACE_VOLUME     : AFNI volume dataset\n"
	    "\n"
	    "        e.g. -sv fred_anat+orig\n"
	    "\n"
	    "        This is the AFNI dataset that the surface is mapped to.\n"
	    "        This dataset is used for the initial surface node to xyz\n"
	    "        coordinate mapping, in the Dicom orientation.\n"
	    "\n"
	    "    -grid_parent AFNI_DSET : AFNI volume dataset\n"
	    "\n"
	    "        e.g. -grid_parent fred_function+orig\n"
	    "\n"
	    "        This dataset is used as a grid and orientation master\n"
	    "        for the output (i.e. it defines the volume domain).\n"
	    "        It is also the source of the output data values.\n"
	    "\n"
	    "    -map_func MAP_FUNC     : filter for values along the segment\n"
	    "\n"
	    "        e.g. -map_func ave\n"
	    "        e.g. -map_func ave -f_steps 10\n"
	    "        e.g. -map_func ave -f_steps 10 -f_index nodes\n"
	    "\n"
	    "        The current mapping function for 1 surface is:\n"
	    "\n"
	    "          mask     : For each surface xyz location, output the\n"
	    "                     dataset values of each sub-brick.\n"
	    "\n"
	    "        Most mapping functions are defined for 2 related input\n"
	    "        surfaces (such as white/grey boundary and pial).  For\n"
	    "        each node pair, the function will be performed on the\n"
	    "        values from the 'grid parent dataset', and along the\n"
	    "        segment connecting the nodes.\n"
	    "\n"
	    "          ave      : Output the average of the dataset values\n"
	    "                     along the segment.\n"
	    "\n"
	    "          max      : Output the maximum dataset value along the\n"
	    "                     connecting segment.\n"
	    "\n"
	    "          max_abs  : Output the dataset value with the maximum\n"
	    "                     absolute value along the segment.\n"
	    "\n"
	    "          median   : Output the median of the dataset values\n"
	    "                     along the connecting segment.\n"
	    "\n"
	    "          midpoint : Output the dataset value with xyz\n"
	    "                     coordinates at the midpoint of the nodes.\n"
	    "\n"
	    "          min      : Output the minimum dataset value along the\n"
	    "                     connecting segment.\n"
	    "\n"
	    "          mode     : Output the mode of the dataset values along\n"
	    "                     the connecting segment.\n"
	    "\n"
	    "          seg_vals : Output all of the dataset values along the\n"
	    "                     connecting segment.  Here, only sub-brick\n"
	    "                     number 0 will be considered.\n"
	    "\n"
	    "  ------------------------------\n"
	    "\n"
	    "  options specific to functions on 2 surfaces:\n"
	    "\n"
	    "          -f_steps NUM_STEPS :\n"
	    "\n"
	    "                     Use this option to specify the number of\n"
	    "                     evenly spaced points along each segment.\n"
	    "                     The default is 2 (i.e. just use the two\n"
	    "                     surface nodes as endpoints).\n"
	    "\n"
	    "                     e.g.     -f_steps 10\n"
	    "                     default: -f_steps 2\n"
	    "\n"
	    "          -f_index TYPE :\n"
	    "\n"
	    "                     This option specifies whether to use all\n"
	    "                     segment point values in the filter (using\n"
	    "                     the 'nodes' TYPE), or to use only those\n"
	    "                     corresponding to unique volume voxels (by\n"
	    "                     using the 'voxel' TYPE).\n"
	    "\n"
	    "                     For instance, when taking the average along\n"
	    "                     one node pair segment using 10 node steps,\n"
	    "                     perhaps 3 of those nodes may occupy one\n"
	    "                     particular voxel.  In this case, does the\n"
	    "                     user want the voxel counted only once, or 3\n"
	    "                     times?  Each way makes sense.\n"
	    "                     \n"
	    "                     Note that this will only make sense when\n"
	    "                     used along with the '-f_steps' option.\n"
	    "                     \n"
	    "                     Possible values are \"nodes\", \"voxels\".\n"
	    "                     The default value is voxels.  So each voxel\n"
	    "                     along a segment will be counted only once.\n"
	    "                     \n"
	    "                     e.g.  -f_index nodes\n"
	    "                     e.g.  -f_index voxels\n"
	    "                     default: -f_index voxels\n"
	    "\n"
	    "          -f_keep_surf_order :\n"
	    "\n"
	    "                     Depreciated.\n"
	    "\n"
	    "                     See required arguments -surf_A and -surf_B,\n"
	    "                     above.\n"
	    "\n"
	    "          Note: The following -f_pX_XX options are used to alter\n"
	    "                the lengths and locations of the computational\n"
	    "                segments.  Recall that by default, segments are\n"
	    "                defined using the node pair coordinates as\n"
	    "                endpoints.  And the direction from p1 to pn is\n"
	    "                from the inner surface to the outer surface.\n"
	    "\n"
	    "          -f_p1_mm DISTANCE :\n"
	    "\n"
	    "                     This option is used to specify a distance\n"
	    "                     in millimeters to add to the first point of\n"
	    "                     each line segment (in the direction of the\n"
	    "                     second point).  DISTANCE can be negative\n"
	    "                     (which would set p1 to be farther from pn\n"
	    "                     than before).\n"
	    "\n"
	    "                     For example, if a computation is over the\n"
	    "                     grey matter (from the white matter surface\n"
	    "                     to the pial), and it is wished to increase\n"
	    "                     the range by 1mm, set this DISTANCE to -1.0\n"
	    "                     and the DISTANCE in -f_pn_mm to 1.0.\n"
	    "\n"
	    "                     e.g.  -f_p1_mm -1.0\n"
	    "                     e.g.  -f_p1_mm -1.0 -f_pn_mm 1.0\n"
	    "\n"
	    "          -f_pn_mm DISTANCE :\n"
	    "\n"
	    "                     Similar to -f_p1_mm, this option is used\n"
	    "                     to specify a distance in millimeters to add\n"
	    "                     to the second point of each line segment.\n"
	    "                     Note that this is in the same direction as\n"
	    "                     above, from point p1 to point pn.\n"
	    "                     \n"
	    "                     So a positive DISTANCE, for this option,\n"
	    "                     would set pn to be farther from p1 than\n"
	    "                     before, and a negative DISTANCE would set\n"
	    "                     it to be closer.\n"
	    "\n"
	    "                     e.g.  -f_pn_mm 1.0\n"
	    "                     e.g.  -f_p1_mm -1.0 -f_pn_mm 1.0\n"
	    "\n"
	    "          -f_p1_fr FRACTION :\n"
	    "\n"
	    "                     Like the -f_pX_mm options above, this\n"
	    "                     is used to specify a change to point p1, in\n"
	    "                     the direction of point pn, but the change\n"
	    "                     is a fraction of the original distance,\n"
	    "                     not a pure change in millimeters.\n"
	    "                     \n"
	    "                     For example, suppose one wishes to do a\n"
	    "                     computation based on the segments spanning\n"
	    "                     the grey matter, but to add 20%% to either\n"
	    "                     side.  Then use -0.2 and 0.2:\n"
	    "\n"
	    "                     e.g.  -f_p1_fr -0.2\n"
	    "                     e.g.  -f_p1_fr -0.2 -f_pn_fr 0.2\n"
	    "\n"
	    "          -f_pn_fr FRACTION :\n"
	    "\n"
	    "                     See -f_p1_fr above.  Note again that the\n"
	    "                     FRACTION is in the direction from p1 to pn.\n"
	    "                     So to extend the segment past pn, this\n"
	    "                     FRACTION will be positive (and to reduce\n"
	    "                     the segment back toward p1, this -f_pn_fr\n"
	    "                     FRACTION will be negative).\n"
	    "\n"
	    "                     e.g.  -f_pn_fr 0.2\n"
	    "                     e.g.  -f_p1_fr -0.2 -f_pn_fr 0.2\n"
	    "\n"
	    "                     Just for entertainment, one could reverse\n"
	    "                     the order that the segment points are\n"
	    "                     considered by adjusting p1 to be pn, and\n"
	    "                     pn to be p1.  This could be done by adding\n"
	    "                     a fraction of 1.0 to p1 and by subtracting\n"
	    "                     a fraction of 1.0 from pn.\n"
	    "\n"
	    "                     e.g.  -f_p1_fr 1.0 -f_pn_fr -1.0\n"
	    "\n"
	    "  ------------------------------\n"
	    "\n"
	    "  options specific to use of normals:\n"
	    "\n"
	    "    Notes:\n"
	    "\n"
	    "      o Using a single surface with its normals for segment\n"
	    "        creation can be done in lieu of using two surfaces.\n"
	    "\n"
	    "      o Normals at surface nodes are defined by the average of\n"
	    "        the normals of the triangles including the given node.\n"
	    "\n"
	    "      o The default normals have a consistent direction, but it\n"
	    "        may be opposite of what is should be.  For this reason,\n"
	    "        the direction is verified by default, and may be negated\n"
	    "        internally.  See the '-keep_norm_dir' option for more\n"
	    "        information.\n"
	    "\n"
	    "    -use_norms             : use normals for second surface\n"
	    "\n"
	    "        Segments are usually defined by connecting corresponding\n"
	    "        node pairs from two surfaces.  With this options the\n"
	    "        user can use one surface, along with its normals, to\n"
	    "        define the segments.\n"
	    "\n"
	    "        By default, each segment will be 1.0 millimeter long, in\n"
	    "        the direction of the normal.  The '-norm_len' option\n"
	    "        can be used to alter this default action.\n"
	    "\n"
	    "    -keep_norm_dir         : keep the direction of the normals\n"
	    "\n"
	    "        Normal directions are verified by checking that the\n"
	    "        normals of the outermost 6 points point away from the\n"
	    "        center of mass.  If they point inward instead, then\n"
	    "        they are negated.\n"
	    "\n"
	    "        This option will override the directional check, and\n"
	    "        use the normals as they come.\n"
	    "\n"
	    "    -norm_len LENGTH       : use LENGTH for node normals\n"
	    "\n"
	    "        e.g.     -norm_len  3.0\n"
	    "        e.g.     -norm_len -3.0\n"
	    "        default: -norm_len  1.0\n"
	    "\n"
	    "        For use with the '-use_norms' option, this allows the\n"
	    "        user to specify a directed distance to use for segments\n"
	    "        based on the normals.  So for each node on a surface,\n"
	    "        the computation segment will be from the node, in the\n"
	    "        direction of the normal, a signed distance of LENGTH.\n"
	    "\n"
	    "        A negative LENGTH means to use the opposite direction\n"
	    "        from the normal.\n"
	    "\n"
	    "        The '-surf_B' option is not allowed with the use of\n"
	    "        normals.\n"
	    "\n"
	    "  ------------------------------\n"
	    "\n"
	    "  general options:\n"
	    "\n"
	    "    -cmask MASK_COMMAND    : (optional) command for dataset mask\n"
	    "\n"
	    "        e.g. -cmask '-a fred_func+orig[2] -expr step(a-0.8)'\n"
	    "\n"
	    "        This option will produce a mask to be applied to the\n"
	    "        input AFNI dataset.  Note that this mask should form a\n"
	    "        single sub-brick.\n"
	    "\n"
	    "        This option follows the style of 3dmaskdump (since the\n"
	    "        code for it was, uh, borrowed from there (thanks Bob!)).\n"
	    "\n"
	    "        See '3dmaskdump -help' for more information.\n"
	    "\n"
	    "    -debug LEVEL           :  (optional) verbose output\n"
	    "\n"
	    "        e.g. -debug 2\n"
	    "\n"
	    "        This option is used to print out status information \n"
	    "        during the execution of the program.  Current levels are\n"
	    "        from 0 to 5.\n"
	    "\n"
	    "    -dnode NODE_NUM        :  (optional) node for debug\n"
	    "\n"
	    "        e.g. -dnode 1874\n"
	    "\n"
	    "        This option is used to print out status information \n"
	    "        for node NODE_NUM.\n"
	    "\n"
	    "    -help                  : show this help\n"
	    "\n"
	    "        If you can't get help here, please get help somewhere.\n"
	    "\n"
	    "    -hist                  : show revision history\n"
	    "\n"
	    "        Display module history over time.\n"
	    "\n"
	    "    -no_headers            : do not output column headers\n"
	    "\n"
	    "        Column header lines all begin with the '#' character.\n"
	    "        With the '-no_headers' option, these lines will not be\n"
	    "        output.\n"
	    "\n"
	    "    -oob_index INDEX_NUM   : specify default index for oob nodes\n"
	    "\n"
	    "        e.g.     -oob_index -1\n"
	    "        default: -oob_index  0\n"
	    "\n"
	    "        By default, nodes which lie outside the box defined by\n"
	    "        the -grid_parent dataset are considered out of bounds,\n"
	    "        and are skipped.  If an out of bounds index is provided,\n"
	    "        or an out of bounds value is provided, such nodes will\n"
	    "        not be skipped, and will have indices and values output,\n"
	    "        according to the -oob_index and -oob_value options.\n"
	    "        \n"
	    "        This INDEX_NUM will be used for the 1dindex field, along\n"
	    "        with the i, j and k indices.\n"
	    "        \n"
	    "\n"
	    "    -oob_value VALUE       : specify default value for oob nodes\n"
	    "\n"
	    "        e.g.     -oob_value -999.0\n"
	    "        default: -oob_value    0.0\n"
	    "\n"
	    "        See -oob_index, above.\n"
	    "        \n"
	    "        VALUE will be output for nodes which are out of bounds.\n"
	    "\n"
	    "    -oom_value VALUE       : specify default value for oom nodes\n"
	    "\n"
	    "        e.g. -oom_value -999.0\n"
	    "        e.g. -oom_value    0.0\n"
	    "\n"
	    "        By default, node pairs defining a segment which gets\n"
	    "        completely obscured by a command-line mask (see -cmask)\n"
	    "        are considered \"out of mask\", and are skipped.\n"
	    "\n"
	    "        If an out of mask value is provided, such nodes will not\n"
	    "        be skipped.  The output indices will come from the first\n"
	    "        segment point, mapped to the AFNI volume.  All output vN\n"
	    "        values will be the VALUE provided with this option.\n"
	    "\n"
	    "        This option is meaningless without a '-cmask' option.\n"
	    "\n"
	    "    -out_1D OUTPUT_FILE    : specify a 1D file for the output\n"
	    "\n"
	    "        e.g. -out_1D mask_values_over_dataset.1D\n"
	    "        e.g. -out_1D stderr\n"
	    "        default: write to stdout\n"
	    "\n"
	    "        This is where the user will specify which file they want\n"
	    "        the output to be written to.  Note that the output file\n"
	    "        should not yet exist.\n"
	    "\n"
	    "        Two special (valid) cases are stdout and stderr, either\n"
	    "        of which may be specified.\n"
	    "\n"
	    "    -version               : show version information\n"
	    "\n"
	    "        Show version and compile date.\n"
	    "\n"
	    "  --------------------------------------------------\n"
	    "\n"
	    "Output from the program defaults to 1D format, in ascii text.\n"
	    "For each node (pair) that results in output, there will be one\n"
	    "line, consisting of:\n"
	    "\n"
	    "    node    : the index of the current node (or node pair)\n"
	    "\n"
	    "    1dindex : the global index of the AFNI voxel used for output\n"
	    "\n"
	    "              Note that for some filters (min, max, midpoint,\n"
	    "              median and mode) there is a specific location (and\n"
	    "              therefore voxel) that the result comes from.  It\n"
	    "              will be accurate (though median may come from one\n"
	    "              of two voxels that are averaged).\n"
	    "\n"
	    "              For filters without a well-defined source (such as\n"
	    "              average or seg_vals), the 1dindex will come from\n"
	    "              the first point on the corresponding segment.\n"
	    "\n"
	    "    i j k   : the i j k indices matching 1dindex\n"
	    "\n"
	    "              These indices are based on the orientation of the\n"
	    "              grid parent dataset.\n"
	    "\n"
	    "    vals    : the number of segment values applied to the filter\n"
	    "\n"
	    "              Note that when -f_index is 'nodes', this will\n"
	    "              always be the same as -f_steps, except when using\n"
	    "              the -cmask option.  In that case, along a single \n"
	    "              segment, some points may be in the mask, and some\n"
	    "              may not.\n"
	    "\n"
	    "              When -f_index is 'voxels' and -f_steps is used,\n"
	    "              vals will often be much smaller than -f_steps.\n"
	    "              This is because many segment points may in a\n"
	    "              single voxel.\n"
	    "\n"
	    "    v0, ... : the requested output values\n"
	    "\n"
	    "              These are the filtered values, usually one per\n"
	    "              AFNI sub-brick.  For example, if the -map_func\n"
	    "              is 'ave', then there will be one segment-based\n"
	    "              average output per sub-brick of the grid parent.\n"
	    "\n"
	    "              In the case of the 'seg_vals' filter, however,\n"
	    "              there will be one output value per segment point\n"
	    "              (possibly further restricted to voxels).  Since\n"
	    "              output is not designed for a matrix of values,\n"
	    "              'seg_vals' is restricted to a single sub-brick.\n"
	    "\n"
	    "\n"
	    "  Author: R. Reynolds  - %s\n"
	    "\n"
	    "                (many thanks to Z. Saad and R.W. Cox)\n"
	    "\n",
	    prog, prog,
	    prog, prog, prog, prog, prog, prog, prog,
	    VERSION );
    }
    else if ( level == V2S_USE_HIST )
	fputs(g_history, stdout);
    else if ( level == V2S_USE_VERSION )
	fprintf(stderr,"%s : %s, compile date: %s\n", prog, VERSION, __DATE__);
    else 
	fprintf( stderr, "usage called with illegal level <%d>\n", level );

    RETURN(-1);
}


/*----------------------------------------------------------------------
 * set_3dmm_bounds	 - note 3dmm bounding values
 *
 * This is an outer bounding box, like FOV, not SLAB.
 *----------------------------------------------------------------------
*/
int set_3dmm_bounds ( THD_3dim_dataset *dset, THD_fvec3 *min, THD_fvec3 *max)
{
    float tmp;
    int   c;

ENTRY("set_3dmm_bounds");

    if ( !dset || !min || !max )
    {
	fprintf(stderr, "** invalid params to set_3dmm_bounds: (%p,%p,%p)\n",
		dset, min, max );
	RETURN(-1);
    }

    /* get undirected bounds */
    min->xyz[0] = DSET_XORG(dset) - 0.5 * DSET_DX(dset);
    max->xyz[0] = min->xyz[0] + DSET_NX(dset) * DSET_DX(dset);

    min->xyz[1] = DSET_YORG(dset) - 0.5 * DSET_DY(dset);
    max->xyz[1] = min->xyz[1] + DSET_NY(dset) * DSET_DY(dset);

    min->xyz[2] = DSET_ZORG(dset) - 0.5 * DSET_DZ(dset);
    max->xyz[2] = min->xyz[2] + DSET_NZ(dset) * DSET_DZ(dset);

    for ( c = 0; c < 3; c++ )
	if ( min->xyz[c] > max->xyz[c] )
	{
	    tmp = min->xyz[c];
	    min->xyz[c] = max->xyz[c];
	    max->xyz[c] = tmp;
	}

    RETURN(0);
}


/*----------------------------------------------------------------------
 * f3mm_out_of_bounds	 - check wether cp is between min and max
 *
 * 			 - v2.3 [rickr]
 *----------------------------------------------------------------------
*/
int f3mm_out_of_bounds( THD_fvec3 * cp, THD_fvec3 * min, THD_fvec3 * max )
{
    int c;

    if ( !cp || !min || !max )
	return(-1);

    for ( c = 0; c < 3; c++ )
    {
	if ( ( cp->xyz[c] < min->xyz[c] ) ||
	     ( cp->xyz[c] > max->xyz[c] ) )
	    return(-1);
    }

    return(0);
}


/*----------------------------------------------------------------------
 * print_header	   - dump standard header for node output         - v2.4
 *----------------------------------------------------------------------
*/
int print_header( FILE * outfp, char * surf, char * map, int nvals )
{
    int val;

ENTRY("print_header");

    fprintf( outfp, "# --------------------------------------------------\n" );
    fprintf( outfp, "# surface '%s', '%s' :\n", surf, map );
    fprintf( outfp, "#\n" );

    /* output column headers */
    fprintf( outfp, "#    node     1dindex    i    j    k     vals" );
    for ( val = 0; val < nvals; val++ )
	fprintf( outfp, "       v%-2d  ", val );
    fputc( '\n', outfp );

    /* underline the column headers */
    fprintf( outfp, "#   ------    -------   ---  ---  ---    ----   " );
    for ( val = 0; val < nvals; val++ )
	fprintf( outfp, " --------   " );
    fputc( '\n', outfp );

    RETURN(0);
}

/*----------------------------------------------------------------------
 * v2s_adjust_endpoints		- adjust endpoints for map and options
 *
 * return   0 on success
 *        < 0 on error
 *----------------------------------------------------------------------
*/
int v2s_adjust_endpts( smap_opts_t * sopt, THD_fvec3 * p1, THD_fvec3 * pn )
{
    THD_fvec3 f3_diff;
    float     dist, factor;

ENTRY("v2s_adjust_endpts");

    if ( !sopt || !p1 || !pn )
    {
	fprintf(stderr,"** v2s_ae: invalid params (%p,%p,%p)\n", sopt, p1, pn);
	RETURN(-1);
    }

    /* first, get the difference, and distance */
    f3_diff.xyz[0] = pn->xyz[0] - p1->xyz[0];
    f3_diff.xyz[1] = pn->xyz[1] - p1->xyz[1];
    f3_diff.xyz[2] = pn->xyz[2] - p1->xyz[2];

    dist = dist_f3mm( p1, pn );

    if ( (sopt->f_p1_fr != 0.0) || (sopt->f_p1_mm != 0.0) )
    {
	if ( sopt->f_p1_fr != 0.0 )	/* what the heck, choose fr if both */
	    factor = sopt->f_p1_fr;
	else
	    factor = (dist == 0.0) ? 0.0 : sopt->f_p1_mm / dist;

	p1->xyz[0] += factor * f3_diff.xyz[0];
	p1->xyz[1] += factor * f3_diff.xyz[1];
	p1->xyz[2] += factor * f3_diff.xyz[2];
    }

    if ( (sopt->f_pn_fr != 0.0) || (sopt->f_pn_mm != 0.0) )
    {
	if ( sopt->f_pn_fr != 0.0 )
	    factor = sopt->f_pn_fr;
	else
	    factor = (dist == 0.0) ? 0.0 : sopt->f_pn_mm / dist;

	pn->xyz[0] += factor * f3_diff.xyz[0];
	pn->xyz[1] += factor * f3_diff.xyz[1];
	pn->xyz[2] += factor * f3_diff.xyz[2];
    }

    switch ( sopt->map )
    {
	default:
	    fprintf(stderr,"** v2s_ae: mapping %d not ready\n", sopt->map );
	    RETURN(-1);

	case E_SMAP_AVE:
	case E_SMAP_MAX:
	case E_SMAP_MAX_ABS:
	case E_SMAP_MIN:
	case E_SMAP_MASK:
	case E_SMAP_SEG_VALS:
	case E_SMAP_MEDIAN:
	case E_SMAP_MODE:
	    break;

	case E_SMAP_MIDPT:

	    /* set the first point to be the average of the two */
	    p1->xyz[0] = (p1->xyz[0] + pn->xyz[0]) / 2.0;
	    p1->xyz[1] = (p1->xyz[1] + pn->xyz[1]) / 2.0;
	    p1->xyz[2] = (p1->xyz[2] + pn->xyz[2]) / 2.0;

	    break;
    }

    RETURN(0);
}


/*---------------------------------------------------------------------------
 * vals_over_steps	- return whether a function is displayed over steps
 *
 * Most function results are output per sub-brick.  These functions will
 * have results displayed over the segment steps.
 *---------------------------------------------------------------------------
*/
int vals_over_steps( int map )
{
    if ( map == E_SMAP_SEG_VALS )
	return 1;

    return 0;
}


/*---------------------------------------------------------------------------
 * v2s_apply_filter  - compute results for the given function and index
 *
 * As a side step, return any filter result index.
 *---------------------------------------------------------------------------
*/
float v2s_apply_filter( range_3dmm_res * rr, smap_opts_t * sopt, int index,
 		        int * findex )
{
    static float_list flist = { 0, 0, NULL };    /* for sorting results */
    static int      * ind_list = NULL;		 /* track index sources */
    double            tmp, comp = 0.0;
    float             fval;
    int               count, source;

ENTRY("v2s_apply_filter");

    if ( !rr || !sopt || index < 0 )
    {
	fprintf(stderr,"** v2s_cm2: invalid params (%p,%p,%d)\n",
		rr, sopt, index);
	RETURN(0.0);
    }
    
    if ( rr->ims.num <= 0 )
	RETURN(0.0);

    /* if sorting is required for resutls, do it now */
    if ( v2s_map_needs_sort( sopt->map ) )
    {
	if ( float_list_alloc( &flist, &ind_list, rr->ims.num, 0 ) != 0 )
	    RETURN(0.0);
	for ( count = 0; count < rr->ims.num; count++ )
	{
	    flist.list[count] = MRI_FLOAT_PTR(rr->ims.imarr[count])[index];
	    ind_list  [count] = count;   /* init index sources */
	}
	flist.nused = rr->ims.num;
	float_list_slow_sort( &flist, ind_list );
    }

    switch ( sopt->map )
    {
	default:
	    if ( findex ) *findex = 0;
	    RETURN(0.0);

	case E_SMAP_AVE:
	    if ( findex ) *findex = 0;
	    for ( count = 0; count < rr->ims.num; count++ )
		comp += MRI_FLOAT_PTR(rr->ims.imarr[count])[index];

	    comp = comp / rr->ims.num;
	    break;

	case E_SMAP_MASK:
	case E_SMAP_MIDPT:
	    if ( findex ) *findex = 0;
	    /* we have only the one point */
	    comp = MRI_FLOAT_PTR(rr->ims.imarr[0])[index];
	    break;

	case E_SMAP_MAX:
	    comp = MRI_FLOAT_PTR(rr->ims.imarr[0])[index];
	    if ( findex ) *findex = 0;

	    for ( count = 1; count < rr->ims.num; count++ )
	    {
		tmp = MRI_FLOAT_PTR(rr->ims.imarr[count])[index];
		if ( tmp > comp )
		{
		    if ( findex ) *findex = count;
		    comp = tmp;
		}
	    }
	    break;

	case E_SMAP_MAX_ABS:
	    comp = MRI_FLOAT_PTR(rr->ims.imarr[0])[index];
	    if ( findex ) *findex = 0;

	    for ( count = 1; count < rr->ims.num; count++ )
	    {
		tmp = MRI_FLOAT_PTR(rr->ims.imarr[count])[index];
		if ( fabs(tmp) > fabs(comp) )
		{
		    if ( findex ) *findex = count;
		    comp = tmp;
		}
	    }
	    break;

	case E_SMAP_MIN:
	    comp = MRI_FLOAT_PTR(rr->ims.imarr[0])[index];
	    if ( findex ) *findex = 0;

	    for ( count = 1; count < rr->ims.num; count++ )
	    {
		tmp = MRI_FLOAT_PTR(rr->ims.imarr[count])[index];
		if ( tmp < comp )
		{
		    if ( findex ) *findex = count;
		    comp = tmp;
		}
	    }
	    break;

	case E_SMAP_SEG_VALS:
	    if ( findex ) *findex = 0;
	    comp = MRI_FLOAT_PTR(rr->ims.imarr[index])[0];
	    break;

	case E_SMAP_MEDIAN:
	    count = flist.nused >> 1;
	    if ( (flist.nused & 1) || (count == 0) )
	    {
		comp = flist.list[count];
		if ( findex ) *findex = ind_list[count];
	    }
	    else
	    {
		comp = (flist.list[count-1] + flist.list[count]) / 2;
		if ( findex ) *findex = ind_list[count-1]; /* take first */
	    }
	    break;

	case E_SMAP_MODE:
	    float_list_comp_mode(&flist, &fval, &count, &source);
	    comp = fval;
	    if ( findex ) *findex = ind_list[source];
	    break;
    }

    RETURN((float)comp);
}


/*----------------------------------------------------------------------
 * v2s_map_needs_sort		- does this map function require sorting?
 *
 * return  1 on true
 *         0 on false
 *----------------------------------------------------------------------
*/
int v2s_map_needs_sort( int map )
{
    if ( (map == E_SMAP_MEDIAN) || (map == E_SMAP_MODE) )
	return 1;

    return 0;
}


/*----------------------------------------------------------------------
 * float_list_comp_mode		- compute the mode of the list
 *
 * return  0 : on success
 *        -1 : on error
 *----------------------------------------------------------------------
*/
int float_list_comp_mode( float_list *f, float *mode, int *nvals, int *index )
{
    float fcur;
    int   ncur, c;

ENTRY("float_list_comp_mode");

    /* init default results */
    *nvals = ncur = 1;
    *mode  = fcur = f->list[0];
    *index = 0;

    for ( c = 1; c < f->nused; c++ )
    {
	if ( f->list[c] == fcur )
	    ncur ++;
	else			    /* found a new entry to count   */
	{
	    if ( ncur > *nvals )     /* keep track of any new winner */
	    {
		*mode  = fcur;
		*nvals = ncur;
		*index = c;
	    }

	    fcur = f->list[c];
	    ncur = 1;
	}
    }

    if ( ncur > *nvals )     /* keep track of any new winner */
    {
	*mode  = fcur;
	*nvals = ncur;
	*index = c;
    }

    RETURN(0);
}


/*----------------------------------------------------------------------
 * float_list_slow_sort		- sort (small) float list
 *
 * If ilist, track index sources.
 *
 * return   0 on success
 *        < 0 on error
 *----------------------------------------------------------------------
*/
int float_list_slow_sort( float_list * f, int * ilist )
{
    float * list, save;
    int     c0, c1, sindex;

ENTRY("float_list_slow_sort");

    list = f->list;  /* for any little speed gain */

    for ( c0 = 0; c0 < f->nused-1; c0++ )
    {
	sindex = c0;
	save   = list[c0];

	/* find smallest remaining */
	for ( c1 = c0+1; c1 < f->nused; c1++ )
	    if ( list[c1] < save )
	    {
		sindex = c1;
		save   = list[sindex];
	    }

	/* swap if smaller was found */
	if ( sindex > c0 )
	{
	    list[sindex] = list[c0];
	    list[c0]     = save;

	    if ( ilist )        /* make same swap of indices */
	    {
		c1            = ilist[sindex];
		ilist[sindex] = ilist[c0];
		ilist[c0]     = c1;
	    }
	}
    }

    RETURN(0);
}


/*----------------------------------------------------------------------
 * float_list_alloc		- verify float list memory
 *
 * If truncate, realloc down to necessary size.
 * If ilist, make space for accompanying int list.
 *
 * return   0 on success
 *        < 0 on error
 *----------------------------------------------------------------------
*/
int float_list_alloc( float_list * f, int ** ilist, int size, int truncate )
{
    THD_fvec3 f3_diff;
    float     dist, factor;

ENTRY("float_list_alloc");

    if ( (f->nalloc < size) ||
	 (truncate && (f->nalloc > size)) )
    {
	f->list = (float *)realloc(f->list, size * sizeof(float));
	if ( ! f->list )
	{
	    fprintf(stderr,"** float_list_alloc: failed for %d floats\n", size);
	    RETURN(-2);
	}
	f->nalloc = size;

	if ( ilist )	 /* then allocate accompanying ilist */
	{
	    *ilist = (int *)realloc(*ilist, size * sizeof(int));
	    if ( ! *ilist )
	    {
		fprintf(stderr,"** float_list_alloc: failed for %d ints\n",
			size);
		RETURN(-2);
	    }
	}

	if ( truncate && (f->nused > f->nalloc) )
	    f->nused = f->nalloc;
    }

    RETURN(0);
}


/*---------------------------------------------------------------------------
 * directed_dist  - travel from pold, along dir, a distance of dist
 *---------------------------------------------------------------------------
*/
float directed_dist( float * pnew, float * pold, float * dir, float dist ) 
{
    double mag, ratio;
    int    c;

ENTRY("directed_dist");

    mag = magnitude_f(dir, 3);

    if ( mag < V2S_EPSILON )	/* can't be negative */
	ratio = 0.0;
    else
	ratio = dist/mag;

    for ( c = 0; c < 3; c++ )
	pnew[c] = pold[c] + dir[c]*ratio;

    RETURN(dist);
}


#if 0								/* v3.8 */

/*---------------------------------------------------------------------------
 * verify_2surf_order  - if surfaces are not inner to outer, swap them
 *---------------------------------------------------------------------------
*/
int verify_2surf_order( float radii[2], node_list_t * N, int debug )
{
    THD_fvec3   node;
    THD_fvec3 * fp0, * fp1;
    char      * tmp_label;
    int         index;

ENTRY("verify_2surf_order");

    if ( !radii || !N )
    {
	fprintf(stderr,"** v2so: invalid params (%p,%p)\n", radii, N);
	RETURN(-1);
    }

    if (radii[0] <= radii[1])		/* cool, we're outta here... */
    {
	if (debug > 1)
	    fprintf(stderr,"-- surfaces are already ordered inner to outer\n"
		           "   (radius %f <= radius %f)\n", radii[0], radii[1]);
	RETURN(0);
    }

    fprintf(stderr, "++ surfaces %s and %s have radii %f and %f\n"
	            "   -> swapping surface order (to go inner to outer)\n",
		    N->labels[0], N->labels[1], radii[0], radii[1]);

    /* first just switch labels */
    tmp_label    = N->labels[0];
    N->labels[0] = N->labels[1];
    N->labels[1] = tmp_label;

    /* now, *sigh*, switch all nodes */
    fp0 = N->nodes;
    fp1 = N->nodes + N->nnodes;
    for ( index = 0; index < N->nnodes; index++, fp0++, fp1++ )
    {
	node = *fp0;
	*fp0 = *fp1;
	*fp1 = node;
    }

    RETURN(0);
}

#endif


/*---------------------------------------------------------------------------
 * surf_ave_radius  -  compute the average distance from the Center
 *---------------------------------------------------------------------------
*/
int surf_ave_radius( float * radius, SUMA_SurfaceObject * so, int disp )
{
    double   ss, sum = 0.0;
    float  * fp;
    float    c0, c1, c2;
    int      node;

ENTRY("surf_ave_radius");

    if ( !so || !radius || so->N_Node <= 0 )
    {
	fprintf(stderr, "** disp_sar, so, radius == %p,%p\n", so, radius );
	RETURN(-1);
    }

    c0 = so->Center[0];				   /* for a little speed */
    c1 = so->Center[1];
    c2 = so->Center[2];

    fp = so->NodeList;
    for ( node = 0; node < so->N_Node; node++ )
    {
	ss  = (*fp - c0) * (*fp - c0);   fp++;
	ss += (*fp - c1) * (*fp - c1);   fp++;
	ss += (*fp - c2) * (*fp - c2);   fp++;

	sum += sqrt(ss);
    }

    *radius = sum/so->N_Node;

    if ( disp )
	fprintf(stderr,"-- surf %s has average dist %f\n"
		       "        to center (%f, %f, %f)\n",
		so->Label, *radius, c0, c1, c2 );

    RETURN(0);
}


/*----------------------------------------------------------------------
 * disp_param_t  -  display the contents of the param_t struct
 *----------------------------------------------------------------------
*/
int disp_param_t ( char * info, param_t * p )
{
ENTRY("disp_param_t");

    if ( info )
	fputs( info, stderr );

    if ( p == NULL )
    {
	fprintf(stderr, "disp_param_t: p == NULL\n" );
	RETURN(-1);
    }

    fprintf(stderr,
	    "param_t struct at %p :\n"
	    "    gpar  : vcheck  = %p : %s\n"
	    "    f3mm_min (xyz)  = (%f, %f, %f)\n"
	    "    f3mm_max (xyz)  = (%f, %f, %f)\n"
	    "    outfp, cmask    = %p : %p\n"
	    "    ncmask, ccount  = %d, %d\n"
	    "    nvox            = %d\n"
	    , p,
	    p->gpar, ISVALID_DSET(p->gpar) ? "valid" : "invalid",
	    p->f3mm_min.xyz[0], p->f3mm_min.xyz[1], p->f3mm_min.xyz[2], 
	    p->f3mm_max.xyz[0], p->f3mm_max.xyz[1], p->f3mm_max.xyz[2], 
	    p->outfp, p->cmask, p->ncmask, p->ccount, p->nvox
	    );

    RETURN(0);
}


/*----------------------------------------------------------------------
 * disp_opts_t  -  display the contents of the opts_t struct
 *----------------------------------------------------------------------
*/
int disp_opts_t ( char * info, opts_t * opts )
{
ENTRY("disp_opts_t");

    if ( info )
	fputs( info, stderr );

    if ( opts == NULL )
    {
	fprintf( stderr, "disp_opts_t: opts == NULL\n" );
	RETURN(-1);
    }

    fprintf(stderr,
	    "options struct at %p :\n"
	    "    gpar_file           = %s\n"
	    "    out_file            = %s\n"
	    "    spec_file           = %s\n"
	    "    sv_file             = %s\n"
	    "    cmask_cmd           = %s\n"
	    "    map_str             = %s\n"
	    "    snames[0,1]         = %s, %s\n"
	    "    no_head             = %d\n"
	    "    use_norms, norm_len = %d, %f\n"
	    "    keep_norm_dir       = %d\n"
	    "    debug, dnode        = %d, %d\n"
	    "    f_index_str         = %s\n"
	    "    f_steps             = %d\n"
	    "    f_p1_fr, f_pn_fr    = %f, %f\n"
	    "    f_p1_mm, f_pn_mm    = %f, %f\n"
	    , opts,
	    CHECK_NULL_STR(opts->gpar_file), CHECK_NULL_STR(opts->out_file),
	    CHECK_NULL_STR(opts->spec_file), CHECK_NULL_STR(opts->sv_file),
	    CHECK_NULL_STR(opts->cmask_cmd), CHECK_NULL_STR(opts->map_str),
	    CHECK_NULL_STR(opts->snames[0]), CHECK_NULL_STR(opts->snames[1]),
	    opts->no_head, opts->use_norms, opts->norm_len, opts->keep_norm_dir,
	    opts->debug, opts->dnode,
	    CHECK_NULL_STR(opts->f_index_str), opts->f_steps,
	    opts->f_p1_fr, opts->f_pn_fr, opts->f_p1_mm, opts->f_pn_mm
	    );

    RETURN(0);
}


/*----------------------------------------------------------------------
 * disp_smap_opts_t  -  display the contents of the smap_opts_t struct
 *----------------------------------------------------------------------
*/
int disp_smap_opts_t ( char * info, smap_opts_t * sopt )
{
ENTRY("disp_smap_opts_t");

    if ( info )
	fputs( info, stderr );

    if ( sopt == NULL )
    {
	fprintf(stderr, "disp_smap_opts_t: sopt == NULL\n");
	RETURN(-1);
    }

    fprintf(stderr,
	    "smap_opts_t struct at %p :\n"
	    "    map, debug, dnode   = %d, %d, %d\n"
	    "    no_head             = %d\n"
	    "    use_norms, norm_len = %d, %f\n"
	    "    keep_norm_dir       = %d\n"
	    "    f_index, f_steps    = %d, %d\n"
	    "    f_p1_fr, f_pn_fr    = %f, %f\n"
	    "    f_p1_mm, f_pn_mm    = %f, %f\n"
	    "    cmask               = %p\n"
	    , sopt,
	    sopt->map, sopt->debug, sopt->dnode, sopt->no_head,
	    sopt->use_norms, sopt->norm_len, sopt->keep_norm_dir,
	    sopt->f_index, sopt->f_steps,
	    sopt->f_p1_fr, sopt->f_pn_fr, sopt->f_p1_mm, sopt->f_pn_mm,
	    sopt->cmask
	    );

    RETURN(0);
}


/*----------------------------------------------------------------------
 * magnitude_f		- return magnitude of float vector
 *----------------------------------------------------------------------
*/
double magnitude_f( float * p, int length )
{
    int     c;
    double  sums = 0.0;

    for ( c = 0; c < length; c++, p++ )
	sums += (*p) * (*p);

    return(sqrt(sums));
}


/*----------------------------------------------------------------------
 * dist_f3mm		- return Euclidean distance between the points
 *----------------------------------------------------------------------
*/
float dist_f3mm( THD_fvec3 * p1, THD_fvec3 * p2 )
{
    double d0, d1, d2;

    if ( p1 == NULL || p2 == NULL )
    {
	fprintf( stderr, "** dist_f3mm: invalid params (%p,%p)\n", p1, p2 );
	return(0.0);
    }

    d0 = p1->xyz[0] - p2->xyz[0];
    d1 = p1->xyz[1] - p2->xyz[1];
    d2 = p1->xyz[2] - p2->xyz[2];

    return(sqrt(d0*d0 + d1*d1 + d2*d2));
}


/*---------------------------------------------------------------------------
 * disp_range_3dmm  -  display the contents of the range_3dmm struct
 *---------------------------------------------------------------------------
*/
int disp_range_3dmm ( char * info, range_3dmm * dp )
{
ENTRY("disp_range_3dmm");

    if ( info )
	fputs( info, stderr );

    if ( dp == NULL )
    {
	fprintf(stderr, "disp_range_3dmm: dp == NULL\n");
	RETURN(-1);
    }

    fprintf(stderr,
	    "range_3dmm struct at %p :\n"
	    "    dset    = %p : %s\n"
	    "    p1      = (%f, %f, %f)\n"
	    "    pn      = (%f, %f, %f)\n",
	    dp, dp->dset, ISVALID_DSET(dp->dset) ? "valid" : "invalid",
	    dp->p1.xyz[0], dp->p1.xyz[1], dp->p1.xyz[2],
	    dp->pn.xyz[0], dp->pn.xyz[1], dp->pn.xyz[2] );

    RETURN(0);
}


/*---------------------------------------------------------------------------
 * disp_range_3dmm_res  -  display the contents of the range_3dmm_res struct
 *---------------------------------------------------------------------------
*/
int disp_range_3dmm_res ( char * info, range_3dmm_res * dp )
{
ENTRY("disp_range_3dmm_res");

    if ( info )
	fputs( info, stderr );

    if ( dp == NULL )
    {
	fprintf(stderr, "disp_range_3dmm: dp == NULL\n");
	RETURN(-1);
    }

    fprintf(stderr,
	    "range_3dmm_res struct at %p :\n"
	    "    ims.num, ims.nall  = %d, %d\n"
	    "    ims.imarr          = %p\n"
	    "    masked, ifirst     = %d, %d\n"
	    "    i3first[0,1,2]     = %d, %d, %d\n"
	    "    i3arr              = %p\n"
	    , dp,
	    dp->ims.num, dp->ims.nall, dp->ims.imarr, dp->masked, dp->ifirst,
	    dp->i3first.ijk[0], dp->i3first.ijk[1], dp->i3first.ijk[2],
	    dp->i3arr );

    if ( dp->i3arr )
	fprintf(stderr,
	    "    i3arr[0].ijk       = %d, %d, %d\n",
	    dp->i3arr[0].ijk[0], dp->i3arr[0].ijk[1], dp->i3arr[0].ijk[2] );

    RETURN(0);
}


/*---------------------------------------------------------------------------
 * disp_mri_imarr  -  display the contents of the MRI_IMARR struct
 *---------------------------------------------------------------------------
*/
int disp_mri_imarr ( char * info, MRI_IMARR * dp )
{
    float * fp;
    int     cr, cc;

ENTRY("disp_mri_imarr");

    if ( info )
	fputs( info, stderr );

    if ( dp == NULL )
    {
	fprintf(stderr, "disp_mri_imarr: dp == NULL\n");
	RETURN(-1);
    }

    fprintf(stderr,
	    "mri_imarr struct at %p :\n"
	    "    num, nall = %d, %d\n",
	    dp, dp->num, dp->nall );

    for ( cr = 0; cr < dp->num; cr++ )
    {
	fp = MRI_FLOAT_PTR(dp->imarr[cr]);
	fprintf(stderr, "    %3d: ", cr);
	for ( cc = 0; cc < dp->imarr[cr]->nx; cc++, fp++ )
	    fprintf(stderr, "%f  ", *fp );
	fputc( '\n', stderr );
    }

    RETURN(0);
}

int disp_fvec ( char * info, float * f, int len )
{
    int c;

ENTRY("disp_fvec");

    if ( info )
	fputs( info, stderr );

    if ( f == NULL || len < 0 )
    {
	fprintf(stderr, "disp_fvec: bad input: f = %p, len = %d\n", f, len);
	RETURN(-1);
    }

    for ( c = 0; c < len; c++ )
	fprintf(stderr, "  %f", f[c]);

    fputc('\n', stderr);

    RETURN(0);
}

