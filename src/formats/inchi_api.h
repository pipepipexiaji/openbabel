#ifndef __INHCH_API_H__
#define __INHCH_API_H__

/* radical definitions */
typedef enum tagINChIRadical {
   INChI_RADICAL_NONE    = 0,
   INChI_RADICAL_SINGLET = 1,
   INChI_RADICAL_DOUBLET = 2,
   INChI_RADICAL_TRIPLET = 3
} inchi_Radical;

/* bond type definitions */
typedef enum tagINChIBondType {
   INChI_BOND_TYPE_NONE    =  0,
   INChI_BOND_TYPE_SINGLE  =  1,
   INChI_BOND_TYPE_DOUBLE  =  2,
   INChI_BOND_TYPE_TRIPLE  =  3,
   INChI_BOND_TYPE_ALTERN  =  4
} inchi_BondType;
/* 2D stereo definitions */
typedef enum tagINChIBondStereo2D {
   /* stereocenter-related; positive: the sharp end points to this atom  */
   INChI_BOND_STEREO_NONE           =  0,
   INChI_BOND_STEREO_SINGLE_1UP     =  1,
   INChI_BOND_STEREO_SINGLE_1EITHER =  4,
   INChI_BOND_STEREO_SINGLE_1DOWN   =  6,
   /* stereocenter-related; negative: the sharp end points to the opposite atom  */
   INChI_BOND_STEREO_SINGLE_2UP     = -1,
   INChI_BOND_STEREO_SINGLE_2EITHER = -4,
   INChI_BOND_STEREO_SINGLE_2DOWN   = -6,
   /* stereobond-related */
   INChI_BOND_STEREO_DOUBLE_EITHER  =  3
} inchi_BondStereo2D;

/* sizes definitions */
#define MAXVAL                   20 /* max number of bonds per atom */
#define ATOM_EL_LEN               6 /* length of ASCIIZ element symbol field */
#define NUM_H_ISOTOPES            3 /* number of hydrogen isotopes: protium, D, T */
#define ISOTOPIC_SHIFT_FLAG   10000  /* add to isotopic shift = */
                                     /* isotopic mass - average atomic mass*/
#define ISOTOPIC_SHIFT_MAX      100  /* max abs(isotopic mass - average atomic mass) */

#ifndef INCHI_US_CHAR_DEF
typedef signed char   S_CHAR;
typedef unsigned char U_CHAR;
#define INCHI_US_CHAR_DEF
#endif

#ifndef INCHI_US_SHORT_DEF
typedef signed short   S_SHORT;
typedef unsigned short U_SHORT;
#define INCHI_US_SHORT_DEF
#endif

typedef  S_SHORT AT_NUM;      /* atom number; starts from 0 */

/*************************************************
 *
 *
 *  A T O M S   a n d   C O N N E C T I V I T Y
 *
 *
 *************************************************/

typedef struct tagINChIAtom {
    /* atom coordinates */
    double x;
    double y;
    double z;
    /* connectivity */
    AT_NUM  neighbor[MAXVAL];     /* adjacency list: ordering numbers of */
                                  /*            the adjacent atoms, >= 0 */
    S_CHAR  bond_type[MAXVAL];    /* inchi_BondType */
    /* 2D stereo */
    S_CHAR  bond_stereo[MAXVAL];  /* inchi_BondStereo2D; negative if the */
                                  /* sharp end points to opposite atom */
    /* other atom properties */
    char    elname[ATOM_EL_LEN];  /* zero-terminated chemical element name:*/
                                  /* "H", "Si", etc. */
    AT_NUM  num_bonds;            /* number of neighbors, bond types and bond*/
                                  /* stereo in the adjacency list */
    S_CHAR  num_iso_H[NUM_H_ISOTOPES+1]; /* implicit hydrogen atoms */
                                  /* [0]: number of implicit non-isotopic H
                                       (exception: num_iso_H[0]=-1 means INCHI
                                       adds implicit H automatically),
                                     [1]: number of implicit isotopic 1H (protium),
                                     [2]: number of implicit 2H (deuterium),
                                     [3]: number of implicit 3H (tritium) */
    AT_NUM  isotopic_mass;        /* 0 => non-isotopic; isotopic mass or  */
                                  /* ISOTOPIC_SHIFT_FLAG + mass - (average atomic mass) */
    S_CHAR  radical;              /* inchi_Radical */
    S_CHAR  charge;               /* positive or negative; 0 => no charge */
}inchi_Atom;

/*******************************************************************
 * Notes: 1. Atom ordering numbers (i, k, and atom[i].neighbor[j] below)
 *           start from zero; max. ordering number is (num_atoms-1).
 *        2. inchi_Atom atom[i] is connected to the atom[atom[i].neighbor[j]]
 *           by a bond that has type atom[i].bond_type[j] and 2D stereo type
 *           atom[i].bond_stereo[j] (in case of no stereo
 *           atom[i].bond_stereo[j] = INChI_BOND_STEREO_NONE)
 *           Index j is in the range 0 <= j <= (atom[i].num_bonds-1)
 *        3. Any connection (represented by atom[i].neighbor[j],
 *           atom[i].bond_type[j], and atom[i].bond_stereo[j])
 *           should be present in one or both adjacency list:
 *             if k = atom[i].neighbor[j] then i may or may not be present in
 *           atom[k].neighbor[] list. You may populate adjacency lists
 *           with only such neighbors that atom[i].neighbor[j] < i
 *        4. in Molfiles usually
 *           (number of implicit H) = Valence - SUM(bond_type[])
 *        5. Seemingly illogical order of the inchi_Atom members was
 *           chosen in an attempt to avoid alignment problems when
 *           accessing inchi_Atom from unrelated to C programming
 *           languages such as Visual Basic.
 *******************************************************************/

/*******************************************************************
    0D Stereo Parity and Type definitions
 *******************************************************************
            Note:
            =====
            o Below #A is the ordering number of atom A, starting from 0
            o See parity values corresponding to 'o', 'e', and 'u' in
              inchi_StereoParity0D definition below)

           =============================================
            stereogenic bond >A=B< or cumulene >A=C=C=B<
           =============================================

                                 neighbor[4]  : {#X,#A,#B,#Y} in this order
     X                           central_atom : NO_ATOM
      \            X      Y      type         : INChI_StereoType_DoubleBond
       A==B         \    /
           \         A==B
            Y

    parity= 'e'    parity= 'o'   unknown parity = 'u'

    Limitations:
    ============
    o Atoms A and B in cumulenes MUST be connected by a chain of double bonds;
      atoms A and B in a stereogenic 'double bond' may be connected by a double,
      single, or alternating bond.
    o One atom may belong to up to 3 stereogenic bonds (i.g. in a fused
      aromatic structure).
    o Multiple stereogenic bonds incident to any given atom should
      either all except possibly one have (possibly different) defined
      parities ('o' or 'e') or should all have an unknown parity 'u'.

      Note on parities of alternating stereobonds
      ===========================================
                                                     D--E
      In large rings  (see Fig. 1, all              //   \\
      atoms are C) all alternating bonds         B--C      F--G
      are treated as stereogenic.              //              \\
      To avoid "undefined" bond parities      A                  H
      for bonds BC, DE, FG, HI, JK, LM, AN     \               /
      it is recommended to mark them with       N==M       J==I
      parities.                                     \     /
                                                      L==K    Fig. 1
      Such a marking will make
      the stereochemical layer unambiguous
      and it will be different from the          B--C      F--G
      stereochemical layer of the second       //   \\   //    \\
      structure (Fig. 2).                     A      D--E        H
                                               \               /
                                                N==M       J==I
      By default, double and alternating            \     /
      bonds in 8-member and greater rings             L==K    Fig. 2
      are treated by INChI as stereogenic.


           =============================================
            tetrahedral atom
           =============================================

   4 neighbors

            X                    neighbor[4] : {#W, #X, #Y, #Z}
            |                    central_atom: #A
         W--A--Y                 type        : INChI_StereoType_Tetrahedral
            |
            Z
   parity: if (X,Y,Z) are clockwize when seen from W then parity is 'e' otherwise 'o'
   Example (see AXYZW above): if W is above the plane XYZ then parity = 'e'

   3 neighbors

              Y          Y       neighbor[4] : {#A, #X, #Y, #Z}
             /          /        central_atom: #A
         X--A  (e.g. O=S   )     type        : INChI_StereoType_Tetrahedral
             \          \
              Z          Z

   parity: if (X,Y,Z) are clockwize when seen from A then parity is 'e',
                                                          otherwise 'o'
   unknown parity = 'u'
   Example (see AXYZ above): if A is above the plane XYZ then parity = 'e'
   This approach may be used also in case of an implicit H attached to A.

           =============================================
            allene
           =============================================

       X       Y                 neighbor[4]  : {#X,#A,#B,#Y}
        \     /                  central_atom : #C
         A=C=B                   type         : INChI_StereoType_Allene

                                      Y      X
                                      |      |
     when seen from A along A=C=B:  X-A    Y-A

                          parity:   'e'    'o'

   parity: if A, B, Y are clockwise when seen from X then parity is 'e',
                                                          otherwise 'o'
   unknown parity = 'u'
   Example (see XACBY above): if X on the diagram is above the plane ABY
                                                      then parity is 'o'

   Limitations
   ===========
   o Atoms A and B in allenes MUST be connected by a chain of double bonds;

   ==============================================
   Note. Correspondence to CML 0D stereo parities
   ==============================================
   a list of 4 atoms corresponds to CML atomRefs4

   tetrahedral atom
   ================
       CML atomParity > 0 <=> INChI_PARITY_EVEN
       CML atomParity < 0 <=> INChI_PARITY_ODD

                                    | 1   1   1   1  |  where xW is x-coordinate of
                                    | xW  xX  xY  xZ |  atom W, etc. (xyz is a
       CML atomParity = determinant | yW  yX  yY  yZ |  'right-handed' Cartesian
                                    | zW  zX  xY  zZ |  coordinate system)

   allene (not yet defined in CML)
   ===============================
   the parity corresponds to the sign of the following determinant
   in exactly same way as for tetrahedral atoms:

       | 1   1   1   1  |  where bonds and neighbor[4] array are
       | xX  xA  xB  xY |  same as defined above for allenes
       | yX  yA  yB  yY |  Obviously, the parity is same for
       | zX  zA  xB  zY |  {#X,#A,#B,#Y} and {#Y,#B,#A,#X}
                           because of the even number of column permutations.

   stereogenic double bond and (not yet defined in CML) cumulenes
   ==============================================================
       CML 'C' (cis)      <=> INChI_PARITY_ODD
       CML 'T' (trans)    <=> INChI_PARITY_EVEN


   How INChI uses 0D parities
   ==========================

   1. 0D parities are used if all atom coordinates are zeroes.

   In addition to that:

   2. 0D parities are used for Stereobonds, Allenes, or Cumulenes if:

   2a. A bond to the end-atom is shorter than MIN_BOND_LEN=0.000001
   2b. A ratio of two bond lengths to the end-atom is smaller than MIN_SINE=0.03
   2c. In case of a linear fragment X-A=B end-atom A is treated as satisfying 2a-b

       0D parities are used if 2a or 2b or 2c applies to one or both end-atoms.

   3. 0D parities are used for Tetrahedral Atoms if at least one of 3a-c is true:

   3a. One of bonds to the central atom is shorter than MIN_BOND_LEN=0.000001
   3b. A ratio of two bond lengths to the central atom is smaller than MIN_SINE=0.03
   3c. The four neighbors are almost in one plane or the central atom and
       its only 3 explicit neighbors are almost in one plane

   Notes on 0D parities and 'undefined' stereogenic elements
   =========================================================

   If 0D parity is to be used according to 1-3 but    CH3     CH3
   has not been provided then the corresponding         \    /
   stereogenic element is considered 'undefined'.        C=CH
                                                        /
   For example, if in the structure (Fig. 3)           H
   the explicit H has been moved so that it                Fig. 3
   has same coordinates as atom >C= (that is,
   the length of the bond H-C became zero)
   then the double bond is assigned 'undefined'       CH3      CH3
   parity which by default is omitted from the          \     /
   Identifier.                                           CH=CH

   However, the structure on Fig. 4 will have double        Fig. 4
   bond parity 'o' and its parity in the Identifier is (-).

   Notes on 0D parities in structures containing metals
   ====================================================
   Since INChI disconnects bonds to metals the 0D parities upon the
   disconnection may change in several different ways:

   1) previously non-stereogenic bond may become stereogenic:

         \     /                            \     /  
          CH==CH          disconnection      CH==CH  
           \ /               ======>                 
            M                                  M     

     before the disconnection:    after the disconnection:
     atoms C has valence=5 and    the double bond may become
     the double bond is not       stereogenic
     recognized as stereogenic

   2) previously stereogenic bond may become non-stereogenic:

       M                           M(+)       
        \    /                           / 
         N==C      disconnection  (-)N==C  
             \        ======>            \ 

   3) Oddball structures, usually resulting from projecting 3D
      structures on the plane, may contain fragment like that
      depicted on Fig. 5:

              M   A                      M   A   
              |\ /   B                      /   B 
              | X   /     disconnection    /   /  
              |/ \ /         ======>      /   /   
              C===C                      C===C    
             Fig. 5
     (X stands for bond intersection)
    
     A-C=C-B parity is              A-C=C-B parity is
     trans (e)                      cis (o) or undefined
     because the bond               because C valence = 3,
     orientation is same            not 4.
     as on Fig, 6 below:

          A       M
           \     /     Removal of M from the structure
            C===C      on Fig. 5 changes the geometry from trans
           /     \     to cis. 
          M'      B    Removal of M and M' from the structure
          Fig. 6       on Fig. 6 does not change the A-C=C-B
                       geometry: it is trans.

   To resolve the problem INChI API accepts the second parity
   corresponding to the metal-disconnected structure.
   To store both bond parities use left shift by 3 bits:

   inchi_Stereo0D::parity = ParityOfConnected | (ParityOfDisconnected<<3)

   In case when only disconnected structure parity exists set
   ParityOfConnected = INChI_PARITY_UNDEFINED.
   This is the only case when INChI_PARITY_UNDEFINED parity
   may be fed to the INChI.

   In cases when the bond parity in a disconnected structure exists and
   differs from the parity in the connected structure the atoms A and B
   should be non-metals.

****************************************************************************/

#define NO_ATOM          (-1) /* non-existent (central) atom */

/* 0D parity types */
typedef enum tagINChIStereoType0D {
   INChI_StereoType_None        = 0,
   INChI_StereoType_DoubleBond  = 1,
   INChI_StereoType_Tetrahedral = 2,
   INChI_StereoType_Allene      = 3
} inchi_StereoType0D;

/* 0D parities */
typedef enum tagINChIStereoParity0D {
   INChI_PARITY_NONE      = 0,
   INChI_PARITY_ODD       = 1,  /* 'o' */
   INChI_PARITY_EVEN      = 2,  /* 'e' */
   INChI_PARITY_UNKNOWN   = 3,  /* 'u' */
   INChI_PARITY_UNDEFINED = 4   /* '?' -- should not be used; however, see Note above */
} inchi_StereoParity0D;


/*************************************************
 *
 *
 *  0D - S T E R E O  (if no coordinates given)
 *
 *
 *************************************************/


typedef struct tagINChIStereo0D {
    AT_NUM  neighbor[4];    /* 4 atoms always */
    AT_NUM  central_atom;   /* central tetrahedral atom or a central */
                            /* atom of allene; otherwise NO_ATOM */
    S_CHAR  type;           /* inchi_StereoType0D */
    S_CHAR  parity;         /* inchi_StereoParity0D: may be a combination of two parities: */
                            /* ParityOfConnected | (ParityOfDisconnected << 3), see Note above */
}inchi_Stereo0D;

/*************************************************
 *
 *
 *  I N C h I    D L L     I n p u t
 *
 *
 *************************************************/

typedef struct tagINChI_Input {
    /* the caller is responsible for the data allocation and deallocation */
    inchi_Atom     *atom;         /* array of num_atoms elements */
    inchi_Stereo0D *stereo0D;     /* array of num_stereo0D 0D stereo elements or NULL */
    char           *szOptions;    /* INChI options: space-delimited; each is preceded by */
                                  /* '/' or '-' depending on OS and compiler */
    AT_NUM          num_atoms;    /* number of atoms in the structure < 1024 */
    AT_NUM          num_stereo0D; /* number of 0D stereo elements */
}inchi_Input;

/*************************************************
 *
 *
 *  I N C h I     D L L     O u t p u t
 *
 *
 *************************************************/

typedef struct tagINChI_Output {
    /* zero-terminated C-strings allocated by GetINChI() */
    /* to deallocate all of them call FreeINChI() (see below) */
    char *szINChI;     /* INChI ASCIIZ string */
    char *szAuxInfo;   /* Aux info ASCIIZ string */
    char *szMessage;   /* Error/warning ASCIIZ message */
    char *szLog;       /* log-file ASCIIZ string, contains a human-readable list */
                       /* of recognized options and possibly an Error/warning message */
} inchi_Output;

/*************************************************
 *
 *
 *  I N C h I      D L L     I n t e r f a c e
 *
 *
 *************************************************/

#if (defined( WIN32 ) && defined( _MSC_VER ) && defined(INCHI_LINK_AS_DLL) )
    /* Win32 & MS VC ++, compile and link as a DLL */
    #ifdef _USRDLL
        /* INChI library dll */
        #define INCHI_API __declspec(dllexport)
        #define EXPIMP_TEMPLATE
        #define INCHI_DECL __stdcall
     #else
        /* calling the INChI dll program */
        #define INCHI_API __declspec(dllimport)
        #define EXPIMP_TEMPLATE extern
        #define INCHI_DECL __stdcall
     #endif
#else
    /* create a statically linked INChI library or link to an executable */
    #define INCHI_API
    #define EXPIMP_TEMPLATE
    #define INCHI_DECL
#endif

/* GetINChI() return values */

typedef enum tagRetValGetINChI {
 
    inchi_Ret_SKIP    = -2, /* not used in INChI dll */
    inchi_Ret_EOF     = -1, /* no structural data has been provided */
    inchi_Ret_OKAY    =  0, /* Success; no errors or warnings */
    inchi_Ret_WARNING =  1, /* Success; warning(s) issued */
    inchi_Ret_ERROR   =  2, /* Error: no INChI has been created */
    inchi_Ret_FATAL   =  3, /* Severe error: no INChI has been created (typically, memory allocation failure) */
    inchi_Ret_UNKNOWN =  4  /* Unknown program error */

} RetValGetINChI;

/* to compile all INChI code as a C++ code #define INCHI_ALL_CPP */
#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

/* inchi_Input is created by the user; strings in inchi_Output are allocated and deallocated by INChI */
EXPIMP_TEMPLATE INCHI_API int INCHI_DECL GetINChI( inchi_Input *inp, inchi_Output *out );

/* FreeINChI() should be called to deallocate char* pointers obtained from each GetINChI() call */
EXPIMP_TEMPLATE INCHI_API void INCHI_DECL FreeINChI ( inchi_Output *out );

/* helper: get string length */
EXPIMP_TEMPLATE INCHI_API int INCHI_DECL GetStringLength( char *p );

#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

/* List of -Exports for INChI_DLL.dll produced by dumpbin:
    ordinal hint RVA      name
          1    0 00048540 _FreeINChI@4
          2    1 00048590 _GetINChI@8
          3    2 0004A6B0 _GetStringLength@4
*/

/* Currently there is no callback function for aborting, progress, etc. */

#endif /* __INHCH_API_H__ */
