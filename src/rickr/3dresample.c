#include "mrilib.h"
#include "r_new_resam_dset.h"
#include "r_idisp.h"

#define MAIN

/*----------------------------------------------------------------------
 * 3dresample - create a new dataset by reorienting and resampling
 *              an existing one 
 *
 * usage:  3dresample  [options]  -prefix OUTPUT_DSET  -inset INPUT_DSET
 *
 *    options:
 *		-help             : detailed program info
 *		-debug            : spit out info
 *              -dxyz DX DY DZ    : resample to a new grid
 *					(DX, DY, DZ are real numbers in mm)
 *		-orient OR_CODE	  : reorient to new orientation code
 *					(a three character string, each
 *					 from the set {A,P, I,S, L,R})
 *
 *		-master MAST_DSET : apply orient/dxyz from MAST_DSET
 *              -zeropad          : zeropad a dataset to match master
 *
 *		-rmode RESAM      : one of {"NN", "Li", "Cu", "Bk"}
 *
 *    examples:
 *	3dresample -orient "asl" -rmode NN -prefix asl.dset -inset inset+orig
 *	3dresample -dxyz 1.0 1.0 0.9 -prefix 119.dset -inset some.input+tlrc
 *      3dresample -master master+orig -prefix new.copy -inset old.copy+orig
 *      3dresample -master master+orig -zeropad -prefix new -inset old+orig
 *----------------------------------------------------------------------
*/

/*--- local stuff ------------------------------------------------------*/

#define USE_LONG	1
#define USE_SHORT	2

#define DELTA_MIN	 0.1
#define DELTA_MAX	99.9

typedef struct
{
    THD_3dim_dataset * dset;
    THD_3dim_dataset * mset;
    double             dx, dy, dz;
    char               orient[4];
    char             * prefix;
    int                resam;
    int                zeropad;
    int                debug;
} options_t;

int disp_opts_data   ( char * info, options_t * opts );
int init_options     ( options_t * opts, int argc, char * argv [] );
int new_zeropad_dset ( options_t * opts, THD_3dim_dataset ** dout );
int resam_str2mode   ( char * mode );
int sync_master_opts ( options_t * opts );
int usage            ( char * prog, int level );

/*----------------------------------------------------------------------*/

int main( int argc , char * argv[] )
{
    THD_3dim_dataset * dout;
    options_t          opts;
    int                ret_val;

    mainENTRY("3dresample"); machdep(); AFNI_logger("3dresample",argc,argv);

    /* validate inputs and init options structure */
    if ( (ret_val = init_options(&opts, argc, argv)) != 0 )
	return ret_val;

    /* actually resample and/or reorient the dataset */
    if ( (dout = r_new_resam_dset( opts.dset, opts.dx, opts.dy, opts.dz,
				   opts.orient, opts.resam ) ) == NULL )
    {
	fprintf( stderr, "failure to resample dataset, exiting...\n" );
	return FAIL;
    }

    /* possibly zeropad the output dataset */
    if ( opts.zeropad && new_zeropad_dset(&opts, &dout) )
	return FAIL;

    return write_results( dout, &opts, argc, argv );

    return 0;
}


/*----------------------------------------------------------------------
 * init_options - validate inputs, give help, init options struct
 *----------------------------------------------------------------------
*/
int init_options ( options_t * opts, int argc, char * argv [] )
{
    int ac;

    /* clear out the options structure */
    memset( opts, 0, sizeof(options_t) );

    for ( ac = 1; ac < argc; ac++ )
    {
	if ( ! strncmp(argv[ac], "-help", 2) )
	{
	    usage( argv[0], USE_LONG );
	    return FAIL;
	}
	else if ( ! strncmp(argv[ac], "-debug", 6) )
	{
	    opts->debug = 1;
	}
	else if ( ! strncmp(argv[ac], "-dxyz", 3) )	/* dxyz */
	{
	    if ( (ac+3) >= argc )
	    {
		fputs( "option usage: -dxyz DX DY DZ\n", stderr );
		usage( argv[0], USE_SHORT );
		return FAIL;
	    }

	    opts->dx = atof(argv[++ac]);
	    opts->dy = atof(argv[++ac]);
	    opts->dz = atof(argv[++ac]);

	    if ( (opts->dx < DELTA_MIN || opts->dx > DELTA_MAX) ||
	         (opts->dy < DELTA_MIN || opts->dy > DELTA_MAX) ||
	         (opts->dz < DELTA_MIN || opts->dz > DELTA_MAX) )
	    {
		fprintf( stderr, "dxyz must be in [%.1f,%.1f]\n",
			 DELTA_MIN, DELTA_MAX );
		return FAIL;
	    }
	}
	else if ( ! strncmp(argv[ac], "-or", 3) )	/* orientation */
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -orient OR_STRING\n", stderr );
		usage( argv[0], USE_SHORT );
		return FAIL;
	    }

	    strncpy( opts->orient, argv[++ac], 3 );
	}
	else if ( ! strncmp(argv[ac], "-master", 5) )	/* master */
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -master MAST_DSET\n", stderr );
		usage( argv[0], USE_SHORT );
		return FAIL;
	    }

	    opts->mset = THD_open_dataset( argv[++ac] );
	    if ( ! ISVALID_DSET(opts->mset) )
	    {
		fprintf( stderr, "invalid master dataset <%s>\n", argv[ac] );
		return FAIL;
	    }
	}
	else if ( ! strncmp(argv[ac], "-zeropad", 5) )	/* zeropad */
	{
	    opts->zeropad = 1;
	}
	else if ( ! strncmp(argv[ac], "-rmode", 6) )	/* resample mode */
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -rmode RESAMPLE_MODE\n", stderr );
		usage( argv[0], USE_SHORT );
		return FAIL;
	    }

	    if ( ( (opts->resam = resam_str2mode(argv[++ac]) ) < 0 ) ||
		 (  opts->resam > LAST_RESAM_TYPE ) )
	    {
		fprintf( stderr, "invalid resample mode <%s>\n", argv[ac] );
		return FAIL;
	    }
	}
	else if ( ! strncmp(argv[ac], "-prefix", 4) )	/* new dset prefix */
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -prefix OUTPUT_PREFIX\n", stderr );
		usage( argv[0], USE_SHORT );
		return FAIL;
	    }

	    opts->prefix = argv[++ac];
	    if ( !THD_filename_ok(opts->prefix) )
	    {
		fprintf( stderr, "invalid output prefix <%s>\n", opts->prefix );
		return usage( argv[0], USE_SHORT );
	    }
	}
	else if ( ! strncmp(argv[ac], "-inset", 3 ) )     /* input dset */
	{
	    if ( (ac+1) >= argc )
	    {
		fputs( "option usage: -inset INPUT_DSET\n", stderr );
		usage( argv[0], USE_SHORT );
		return FAIL;
	    }

	    opts->dset = THD_open_dataset( argv[++ac] );
	    if ( ! ISVALID_DSET(opts->dset) )
	    {
		fprintf( stderr, "invalid input dataset <%s>\n", argv[ac] );
		return FAIL;
	    }
	}
	else	 /* invalid option */
	{
	    fprintf( stderr, "invalid option <%s>\n", argv[ac] );
	    usage( argv[0], USE_SHORT );
	    return FAIL;
	}
    }

    if ( opts->debug )
	disp_opts_data( "post options: ", opts );

    if ( !ISVALID_DSET(opts->dset) || (opts->prefix == NULL) )
    {
	fprintf( stderr, "missing prefix or input dset, exiting...\n" );
	usage( argv[0], USE_SHORT );
	return FAIL;
    }

    if ( sync_master_opts( opts ) )
	return FAIL;

    return 0;
}


/*----------------------------------------------------------------------*/
int usage ( char * prog, int level )
{
    if ( level == USE_SHORT )
    {
	fprintf( stderr, "usage : %s [options] -prefix OUT_DSET "
		                              "-inset IN_DSET\n", prog );
	return 0;
    }
    else if ( level == USE_LONG )
    {
	printf( "\n"
		"%s - reorient and/or resample a dataset\n"
		"\n"
		"  usage: %s [options] -prefix OUT_DSET -inset IN_DSET\n"
		"\n"
		"  examples:\n"
		"\n"
		"    %s -orient asl -rmode NN -prefix asl.dset -inset in+orig\n"
		"    %s -dxyz 1.0 1.0 0.9 -prefix 119.dset -inset in+tlrc\n"
		"    %s -master master+orig -prefix new.dset -inset old+orig\n"
		"    %s -master M+orig -zeropad -prefix new -inset old+orig\n"
		"\n"
		"  options: \n"
		"    -help            : show this help information\n"
		"    -debug           : print debug info along the way\n"
		"\n"
		"    -dxyz DX DY DZ   : resample to new dx, dy and dz\n"
		"          e.g.  -dxyz 1.0 1.0 0.9\n"
		"          default is to leave unchanged\n"
		"\n"
		"          Each of DX,DY,DZ must be a positive real number,\n"
		"          and will be used for a voxel delta in the new\n"
		"          dataset (according to any new orientation).\n"
		"\n"
		"    -orient OR_CODE  : reorient to new axis order.\n"
		"          e.g.  -orient asl\n"
		"          default is to leave unchanged\n"
		"\n"
		"          The orientation code is a 3 character string,\n"
		"          where the characters come from the respective\n"
		"          sets {A,P}, {I,S}, {L,R}.\n"
		"\n"
		"    -master MAST_DSET: apply orient/dxyz from MAST_DSET\n"
		"          e.g.  -master master.dset+orig\n"
		"\n"
		"          Get dxyz and orient from a master dataset.  This\n"
		"          option cannot be used with -dxyz or -orient.\n"
		"\n"
		"    -zeropad         : zeropad to match box of master\n"
		"          e.g.  -zeropad\n"
		"\n"
		"          Pad and/or chop a resulting dataset to match the\n"
		"          volume of the dataset supplied as master MAST_DSET\n"
		"          with the -master option.  The -zeropad option can\n"
		"          only be used if the -master option is also used.\n"
		"\n"
		"    -rmode RESAM     : use this resampling method\n"
		"          e.g.  -rmode Linear\n"
		"          default is NN (nearest neighbor)\n"
		"\n"
		"          The resampling method string RESAM should come\n"
		"          from the set {'NN', 'Li', 'Cu', 'Bk'}.  These\n"
                "          are for 'Nearest Neighbor', 'Linear', 'Cubic'\n"
		"          and 'Blocky' interpolation, respectively.\n"
		"          See 'Anat resam mode' under the 'Define Markers'\n"
		"          window in afni.\n"
		"\n"
		"    -prefix OUT_DSET : required prefix for output dataset\n"
		"          e.g.  -prefix reori.asl.pickle\n"
		"\n"
		"    -inset IN_DSET   : required input dataset to reorient\n"
		"          e.g.  -inset old.dset+orig\n"
		"\n",
		prog, prog, prog, prog, prog, prog );

	return 0;
    }

    fprintf( stderr, "usage called with illegal level <%d>\n", level );

    return FAIL;
}

/*----------------------------------------------------------------------*/
int resam_str2mode ( char * modestr )
{
    int mode;

    for (mode = FIRST_RESAM_TYPE; mode <= LAST_RESAM_TYPE; mode++ )
    {
	if ( ! strncmp( modestr, RESAM_typestr[mode], 2 ) )
	    return mode;
    }

    return FAIL;
}


/*----------------------------------------------------------------------*/
int write_results ( THD_3dim_dataset * dout, options_t * opts,
		    int argc, char * argv [] )
{
    /* set filename */
    EDIT_dset_items( dout, ADN_prefix, opts->prefix, ADN_none );

    if ( THD_is_file(DSET_HEADNAME(dout)) )
    {
	fprintf( stderr, "error: cannot overwrite existing dataset <%s>\n",
		 DSET_HEADNAME(dout) );
	return FAIL;
    }

    /* set number of time-axis slices to 0 */
    if( DSET_NUM_TTOFF(dout) > 0 )
	EDIT_dset_items( dout, ADN_nsl, 0, ADN_none );

    /* add to old history */
    tross_Copy_History( opts->dset , dout );
    tross_Make_History( "3dresample", argc, argv, dout );


    /* write the output files */
    if ( DSET_write( dout ) != True )
    {
	fprintf( stderr, "failure to write dataset, exiting...\n" );
	return FAIL;
    }

    return 0;
}


/*----------------------------------------------------------------------*/
int sync_master_opts ( options_t * opts )
{
    THD_dataxes * dax;

    if ( opts->zeropad && !opts->mset )
    {
	fputs( "error: cannot use -zeropad without -master\n", stderr );
	return FAIL;
    }

    if ( opts->debug )		/* dset is valid by now */
    {
	r_idisp_thd_3dim_dataset( "sync dset : ", opts->dset );
	r_idisp_thd_dataxes     ( "sync dset : ", opts->dset->daxes );
    }

    if ( !opts->mset )
	return 0;			/* OK */

    if ( ! ISVALID_DSET(opts->mset) ||
	 ! ISVALID_DATAXES(opts->mset->daxes ) )
    {
	fputs( "error: problem with master dset, it is not valid!\n", stderr );
	return FAIL;			/* non-NULL but invalid is bad */
    }

    if ( ( opts->dx != 0.0 ) || ( opts->orient[0] != '\0' ) )
    {
	fputs( "error: -dxyz and -orient are not valid with -master option, "
	       "exiting...\n", stderr );
	return FAIL;
    }

    /* all is okay, so fill dxyz and orientation code from master */
    dax = opts->mset->daxes;

    opts->dx = dax->xxdel;
    opts->dy = dax->yydel;
    opts->dz = dax->zzdel;

    opts->orient[0] = ORIENT_typestr[dax->xxorient][0];
    opts->orient[1] = ORIENT_typestr[dax->yyorient][0];
    opts->orient[2] = ORIENT_typestr[dax->zzorient][0];
    opts->orient[3] = '\0';

    if ( opts->debug )
    {
	disp_opts_data( "sync opts : ", opts );
	r_idisp_thd_3dim_dataset( "sync mset : ", opts->mset );
	r_idisp_thd_dataxes     ( "sync mset : ", opts->mset->daxes );
    }

    return 0;
}

/*----------------------------------------------------------------------*/
int disp_opts_data ( char * info, options_t * opts )
{
    if ( info )
	fputs( info, stdout );

    if ( opts == NULL )
    {
	printf( "disp_opts_data: opts == NULL\n" );
	return FAIL;
    }

    printf( "options struct at %p :\n"
	    "    dset        = %p (%s)\n"
	    "    mset        = %p (%s)\n"
	    "    (dx,dy,dz)  = (%6.3f, %6.3f, %6.3f)\n"
	    "    orient      = %.6s\n"
	    "    prefix      = %.60s\n"
	    "    resam       = %d\n"
	    "    zeropad     = %d\n"
	    "    debug       = %d\n",
	    opts,
	    opts->dset, ISVALID_DSET(opts->dset) ? "valid" : "invalid",
	    opts->mset, ISVALID_DSET(opts->mset) ? "valid" : "invalid",
	    opts->dx, opts->dy, opts->dz,
	    opts->orient, opts->prefix, opts->resam,
	    opts->zeropad, opts->debug );

    return 0;
}


/*----------------------------------------------------------------------
 * new_zeropad_dset - create a new zeropadded dataset
 *
 * Replace dout with a new padded one.
 * This function copies the master part of 3dZeropad.c.
 *----------------------------------------------------------------------
*/
int new_zeropad_dset ( options_t * opts, THD_3dim_dataset ** dout )
{
    THD_3dim_dataset * tmp_dset;
    THD_dataxes      * max = opts->mset->daxes, * iax = (*dout)->daxes;
    int                nerr = 0;
    float              mxbot,mybot,mzbot, mxtop,mytop,mztop, mdx,mdy,mdz;
    float              ixbot,iybot,izbot, ixtop,iytop,iztop, idx,idy,idz;
    int                mnx,mny,mnz, inx,iny,inz;
    int                add_xb,add_xt, add_yb,add_yt, add_zb,add_zt;
    int                add_I=0, add_S=0, add_A=0, add_P=0, add_L=0, add_R=0;

    /* check if datasets are oriented the same */
    if( max->xxorient != iax->xxorient ||
        max->yyorient != iax->yyorient ||
        max->zzorient != iax->zzorient )
    {
	fputs("error: orientation mismatch!\n", stderr );
        nerr++;
    }

    /* check if datasets have same voxel dimensions */
    mdx = max->xxdel;  mdy = max->yydel; mdz = max->zzdel;
    idx = iax->xxdel;  idy = iax->yydel; idz = iax->zzdel;
    mnx = max->nxx;    mny = max->nyy;   mnz = max->nzz;
    inx = iax->nxx;    iny = iax->nyy;   inz = iax->nzz;

    if( fabs(mdx-idx) > 0.01*fabs(mdx) ||
        fabs(mdy-idy) > 0.01*fabs(mdy) ||
        fabs(mdz-idz) > 0.01*fabs(mdz) )
    {
       fputs("error: voxel size mismatch!\n", stderr);
       nerr++;
    }

    if ( nerr > 0 )
    {
	if ( opts->debug )
	{
	    r_idisp_thd_dataxes( "dset : ", (*dout)->daxes );
	    r_idisp_thd_dataxes( "mset : ", opts->mset->daxes );
	}
	return FAIL;
    }

    /* data is good! */

    /* calculate coords at top and bottom of each dataset */
    mxbot = max->xxorg; mxtop = mxbot + mnx*mdx;
    mybot = max->yyorg; mytop = mybot + mny*mdy;
    mzbot = max->zzorg; mztop = mzbot + mnz*mdz;

    ixbot = iax->xxorg; ixtop = ixbot + inx*idx;
    iybot = iax->yyorg; iytop = iybot + iny*idy;
    izbot = iax->zzorg; iztop = izbot + inz*idz;

    /* calculate amount to add/trim at each face */
    add_xb = (int) rint((ixbot-mxbot)/idx);
    add_xt = (int) rint((mxtop-ixtop)/idx);
    add_yb = (int) rint((iybot-mybot)/idy);
    add_yt = (int) rint((mytop-iytop)/idy);
    add_zb = (int) rint((izbot-mzbot)/idz);
    add_zt = (int) rint((mztop-iztop)/idz);

    /* map trims from x,y,z to RL,AP,IS coords */

    switch( iax->xxorient ){
	case ORI_R2L_TYPE: add_R = add_xb; add_L = add_xt; break;
	case ORI_L2R_TYPE: add_L = add_xb; add_R = add_xt; break;
	case ORI_I2S_TYPE: add_I = add_xb; add_S = add_xt; break;
	case ORI_S2I_TYPE: add_S = add_xb; add_I = add_xt; break;
	case ORI_A2P_TYPE: add_A = add_xb; add_P = add_xt; break;
	case ORI_P2A_TYPE: add_P = add_xb; add_A = add_xt; break;
	default          : fputs("bad xxorient!\n", stderr); return FAIL;
    }

    switch( iax->yyorient ){
	case ORI_R2L_TYPE: add_R = add_yb; add_L = add_yt; break;
	case ORI_L2R_TYPE: add_L = add_yb; add_R = add_yt; break;
	case ORI_I2S_TYPE: add_I = add_yb; add_S = add_yt; break;
	case ORI_S2I_TYPE: add_S = add_yb; add_I = add_yt; break;
	case ORI_A2P_TYPE: add_A = add_yb; add_P = add_yt; break;
	case ORI_P2A_TYPE: add_P = add_yb; add_A = add_yt; break;
	default          : fputs("bad xxorient!\n", stderr); return FAIL;
    }

    switch( iax->zzorient ){
	case ORI_R2L_TYPE: add_R = add_zb; add_L = add_zt; break;
	case ORI_L2R_TYPE: add_L = add_zb; add_R = add_zt; break;
	case ORI_I2S_TYPE: add_I = add_zb; add_S = add_zt; break;
	case ORI_S2I_TYPE: add_S = add_zb; add_I = add_zt; break;
	case ORI_A2P_TYPE: add_A = add_zb; add_P = add_zt; break;
	case ORI_P2A_TYPE: add_P = add_zb; add_A = add_zt; break;
	default          : fputs("bad xxorient!\n", stderr); return FAIL;
    }

    if ( opts->debug )
    {
	printf( "++ zeropad: (I,S,A,P,R,L) = (%d,%d,%d,%d,%d,%d)\n",
		add_I, add_S, add_A, add_P, add_R, add_L );
    }

    /* pad if we need to */
    if ( add_I || add_S || add_A || add_P || add_R || add_L )
    {
	tmp_dset = THD_zeropad( *dout,
				add_I, add_S, add_A, add_P, add_R, add_L,
				opts->prefix, ZPAD_PURGE );

	if ( !ISVALID_DSET( tmp_dset ) )
	{
	    fputs( "THD_zeropad failed!\n", stderr );
	    return FAIL;
	}

	DSET_delete( *dout );
	*dout = tmp_dset;
    }

    return 0;
}

