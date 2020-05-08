#pragma once

#define NO_SIGR_MAX 32
#define NO_SIGR_MIN 1

#define NO_IO_MAX 15
#define NO_IO_MIN 1

#define NO_PREGORELIH_SIJALICA_MAX 15
#define NO_PREGORELIH_SIJALICA_MIN 0

// Spec se odno si na SG strelice, tramvaje, pesake i vozace
#define NO_SPEC_SG_MIN 0
#define NO_SPEC_SG_MAX 32

#define RED_YELLOW_TIME_MIN 2
#define RED_YELLOW_TIME_MAX 10

#define YELLOW_TIME_MIN 3
#define YELLOW_TIME_MAX 10

#define YELLOW_FLASH_TIME_MIN 0
#define YELLOW_FLASH_TIME_MAX 30

#define ALL_YELLOW_TIME_MIN YELLOW_FLASH_TIME_MIN
#define ALL_YELLOW_TIME_MAX YELLOW_FLASH_TIME_MAX

#define ALL_RED_TIME_MIN YELLOW_FLASH_TIME_MIN
#define ALL_RED_TIME_MAX YELLOW_FLASH_TIME_MAX

#define LAST_ERROR_IN_BUFFER_MIN 1
#define LAST_ERROR_IN_BUFFER_MAX 100

#define FPLAN_NUMBER_MIN 1
#define FPLAN_NUMBER_MAX 19

namespace kss2_config
{

typedef unsigned char uchar;
typedef unsigned int uint;

struct SEE_Adrese
{
    uchar kss_485;
    uchar grad;
    uchar uredj;
    uchar group_adr;
};

struct EEfolder
{
    uchar noSigGr;     /* broj signalnih grupa */
    uchar noIOadr;     /* broj IO ploca */
    uchar noCflPr;     /* broj konfliktnih parova */
    uchar noCrkSij;    /* broj crknutih sijalica koji izazivaju ispad */
    SEE_Adrese adrese; /*adresa na 485, grad, uredj, grupna_adr */
    uchar e, f;        /* rezervni parametri */
    uint mGreenTblCRC; /* CRC tabele minimalnog zelenog       */
    uint ConflPTblCRC; /* CRC tabele konflikata               */
    uint SiGrIOTblCRC; /* CRC tabele inicijalizacije IO ploca */
};
// -------------------------------------------

struct SStartTimes
{
    uchar zuti_treptac;
    uchar sve_zuto;
    uchar sve_crveno;
} /* startna vremena kontrolera	*/;

struct S_konflmode
{
    uchar pg1 : 2; /*  modovi podgrupa:  B_MOD_TEST 0   B_MOD_ZT  1	B_MOD_SC  2	 B_MOD_A  3 */
    uchar pg2 : 2;
    uchar pg3 : 2;
    uchar pg4 : 2;

    uchar vpg1 : 1;
    uchar vpg2 : 1;
    uchar vpg3 : 1;
    uchar vpg4 : 1;

    uchar glm : 2; /*   glavni mod:  B_MOD_TEST 0   B_MOD_ZT  1	B_MOD_SC  2	 B_MOD_A  3	*/
    uchar glt : 1;
    uchar fl : 1;
};

union Ukonflmod {
    S_konflmode km;
    uchar bytes[sizeof(S_konflmode)];
};

struct SPGmod
{
    uchar gr1;
    uchar gr2;
    uchar gr3;
    uchar gr4;
};

struct Sbd
{
    uchar mod : 2; /*  B_MOD_TEST 0   B_MOD_ZT  1	B_MOD_SC  2	 B_MOD_A  3	*/
    uchar koord_lok : 1;
    uchar fors_tab : 1;
    uchar v_trep : 1;
    uchar p_trep : 1;
    uchar detektor : 1;
    uchar najava : 1;
};

struct SBios_Dinamic
{
    Sbd proc;
    Ukonflmod knfl;
};

struct SRestart
{
    uchar no : 2;
    uchar time : 6;
    uchar temp_no;
    uchar refresh;
};

struct SDrerr_byte
{
    uchar drerr_pg1 : 1;
    uchar drerr_pg2 : 1;
    uchar drerr_pg3 : 1;
    uchar drerr_pg4 : 1;
    uchar drop : 1;
    uchar nondrop : 1;
    uchar prekidac : 2;
};

struct SErrorCtrl
{
    uchar brojac_buff;
    uchar idle;
    uchar idle1;
};

struct SError
{
    uchar type, address, data;
};

struct SRealTime_
{
    uchar hour;
    uchar min;
    uchar sec;
    uchar date;
    uchar month;
    uchar year;
};

struct SErrorTime
{
    SError serror;
    uchar broj_plana;
    uchar vreme_u_ciklusu;
    SRealTime_ realtime;
};

struct SErrorTimeComm
{
    SErrorTime sdrop;
    SErrorTime snondrop;
    SDrerr_byte sflags;
};

#define PNP_VEL 3

struct SSPIfolder
{
    uchar brvg;    /* broj vozackih grupa */
    uchar brtg;    /* broj tramvajskih grupa */
    uchar brpg;    /* broj pesackih */
    uchar brstrel; /* broj strelica */
    uchar czuto_v; /* czuto vreme - 2 sek po defaultu */
    uchar zuto_v;  /* zuto vreme - 3 sek po defaultu */
    SStartTimes start_times;
    uchar PnP[PNP_VEL]; /* plug and play indetifikacija */

    /*uchar none;*/
    uchar zapis_na_minuta;

    uchar fors_plan;
    uchar broj_planova; /* ukupan broj upisanih signalnih planova */
    uchar ispravnost;   /* treba biti 1 aki ima gresaka u kontroleru */
    uint CRC_dana;
    uint CRC_praznika;
    uint CRC_datuma;

    SBios_Dinamic Bios;
    //struct SBios_Dinamic  Dinamic;
    SRestart Restart;

    SErrorTimeComm ErrorTimeComm;
    SErrorCtrl ErrorCtrl;

    uchar leto_zima;
    uchar none;

}; /* zaglavlje podataka u baterijskom ram-u */

/****************   end of strukture podataka za CONTROLLER_CONFIG      *********************/

/****************   strukture podataka za CNFLBoard      *********************/

#define NO_IO_MIN 1
#define NO_IO_MAX 15

struct SSijlelem
{
    uint sub : 2;    /* zutotreptacka podgrupa */
    uint sigr : 5;   /* signalna grupa  */
    uint valid : 1;  /* validnost */
    uint collor : 2; /* boja leda: 0-mrak, 1-crvena, 2-zelena, 3-zuta */
    uint led : 3;    /* redni broj leda: 0-7  */
    uint osig : 1;   /* osiguranje */
    uint yell : 1;   /* zuti treptac */
    uint sign : 1;   /* osvetljenje saogracajnog znaka */
};

struct SIOSijl
{
    SSijlelem sijlelem[6];
};

#define NO_CONFL_COUPLES_MIN 0
#define NO_CONFL_COUPLES_MAX 496
#define NO_MIN_GREEN_MAX 32
struct SConflP
{
    uchar sigr1;
    uchar sigr2;
    uchar ftime : 4; // ????????????????  Za novi uredjaj mora da se definise ceo signed byte da bi mogao da se definise negativan broj MT BL
    uchar ltime : 4;
};

auto load(const char *fileName,
                      EEfolder *ef,
                      SSPIfolder *sspi,
                      SIOSijl *ios,
                      SConflP *cc,
                      uchar *mg) -> int;

auto save(const char *fileName,
                      const EEfolder *ef,
                      const SSPIfolder *sspi,
                      const SIOSijl *ios,
                      const SConflP *cc,
                      const uchar *mg) -> void;
}